///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Missing.hpp"
#include "Fork.hpp"
#include "../Temporal.hpp"
#include "../verbs/Do.inl"
#include "../verbs/Interpret.inl"

#define VERBOSE_MISSING_POINT(...) \
   Logger::Verbose(__VA_ARGS__)
#define VERBOSE_MISSING_POINT_TAB(...) \
   const auto tabs = Logger::Verbose(__VA_ARGS__, Logger::Tabs{})
#define VERBOSE_FUTURE(...) 

using namespace Langulus::Anyness;
using namespace Langulus::Flow::Inner;


/// Initialize a missing point by a precompiled filter                        
///   @param filter - the filter to set                                       
Missing::Missing(const TMany<DMeta>& filter, Real priority)
   : mFilter {filter}
   , mPriority {priority} { }

/// Initialize a missing point by a filter, will be precompiled               
/// i.e. all meta data definitions will be gathered                           
///   @param filter - the filter to set                                       
Missing::Missing(const Block<>& filter, Real priority)
   : mPriority {priority} {
   mFilter.GatherFrom(filter, DataState::Missing);
   mFilter.SetState(filter.GetState());
}

/// Check if immediate contents are accepted by the filter of this point      
/// Verbs are always accepted                                                 
///   @param content - the content to check                                   
///   @return true if contents are accepted                                   
bool Missing::Accepts(const Block<>& content) const {
   if (not mFilter or content.CastsTo<Verb, true>())
      return true;

   for (auto type : mFilter) {
      if (content.template CastsToMeta<true>(type))
         return true;
   }

   return false;
}

/// Check if the missing point has been satisfied by pushed contents          
///   @return true if point was satisfied                                     
bool Missing::IsSatisfied() const {
   if (not mContent or not mFilter)
      return false;

   bool satisfied = false;
   mContent.ForEachDeep([&](const Block<>& b) {
      if (Accepts(b))
         satisfied = true;
      return not satisfied;
   });

   return satisfied;
}

/// Push content to the missing point, if filters allow it                    
///   @attention assumes content is a valid container                         
///   @attention assumes content has been Temporal::Compiled previously       
///   @param content - the content to push                                    
///   @return true if mContent changed                                        
bool Missing::Push(const Many& content) {
   bool atLeastOneSuccess = false;

   if (content.IsDeep()) {
      // Always nest deep contents, we must filter each part and        
      // make sure branches are correctly inserted in forks             
      if (content.IsOr()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         Missing fork {mFilter, mPriority};
         fork.mContent.MakeOr();
            
         content.ForEach([&](const Many& subcontent) {
            atLeastOneSuccess |= fork.Push(subcontent);
         });

         mContent.SmartPush(IndexBack, Abandon(fork.mContent));
      }
      else {
         // Just nest                                                   
         content.ForEach([&](const Many& subcontent) {
            atLeastOneSuccess |= Push(subcontent);
         });
      }

      return atLeastOneSuccess;
   }

   //                                                                   
   // If reached, we're pushing flat data                               
   // Let's check if there's a filter                                   
   if (not mFilter) {
      // No filter, just push                                           
      if (not content.template CastsTo<Verb, true>()) {
         // Always try interpreting scope as verbs                      
         Verbs::InterpretAs<Verb> interpreter;
         interpreter.ShortCircuit(false);

         if (DispatchDeep(content, interpreter)) {
            // Something was interpreted to verbs                       
            // Compile and push it                                      
            const auto compiled = Temporal::Compile(
               interpreter.GetOutput(), NoPriority);
            return Push(compiled);
         }
      }

      // Scope is either verbs or something else, just push             
      bool pastHasBeenConsumed = false;
      Many linked;
      try {
         linked = Link(content, {}, pastHasBeenConsumed);
      }
      catch (const Except::Link&) {
         return false;
      }

      if (linked) {
         if (pastHasBeenConsumed) {
            // There were missing points in content, and this           
            // mContent   has been consumed to fill them, so we         
            // directly overwrite                                       
            mContent = Abandon(linked);
         }
         else {
            // There were no missing points in content, so just         
            // push it to this future point as it was provided          
            mContent << Abandon(linked);
         }

         VERBOSE_MISSING_POINT("Resulting contents: ", mContent);
         return true;
      }
      else return false;
   }

   //                                                                   
   // Filters are available, interpret source as requested              
   //TODO Always add an interpretation to verbs? deprecated?
   VERBOSE_MISSING_POINT_TAB("Satisfying filter ", mFilter,
      " by interpreting ", content);

   Verbs::Interpret interpreter {mFilter};
   interpreter.ShortCircuit(false);

   if (DispatchDeep(content, interpreter)) {
      // If results in verb skip insertion                              
      // Instead delay push to an unfiltered location later on          
      //TODO is this really required? removed for now
      const auto compiled = Temporal::Compile(
         interpreter.GetOutput(), NoPriority);
      VERBOSE_MISSING_POINT(Logger::Green, "Interpreted as: ", compiled);
      return Push(compiled);
   }

   // Nothing pushed to this point                                      
   VERBOSE_MISSING_POINT(Logger::DarkYellow,
      "Nothing viable remained after interpretation");
   return false;
}

/// Links the missing past points of the provided scope, using mContent as    
/// the past, and returns a viable overwrite for mContent                     
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param environment - a fallback past provided by Temporal               
///   @param consumedPast - [out] set to true if anythingin this point has    
///      been used in any scope missing past                                  
///   @return the linked equivalent to the provided scope                     
Many Missing::Link(const Block<>& scope, const Block<>& environment, bool& consumedPast) const {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsDeep()) {
      // Nest scopes, linking any past points in subscopes              
      scope.ForEach([&](const Block<>& subscope) {
         try {
            result << Link(subscope, environment, consumedPast);
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", subscope);
         }
      });

      if (result.GetCount() < 2)
         result.MakeAnd();
      return Abandon(result);
   }

   // Iterate backwards - the last future points are always most        
   // relevant for linking                                              
   // Lets start, by scanning all future points in the available        
   // stack. Scope will be shallow-copied for each encountered          
   // branch, and then cloned if changes occur.                         
   const auto found = scope.ForEach(
      [&](const Trait& trait) {
         try {
            result << Trait::From(
               trait.GetTrait(), 
               Link(trait, environment, consumedPast)
            );
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", trait);
         }
      },
      [&](const Construct& construct) {
         try {
            result << Construct {
               construct.GetType(), 
               Link(construct.GetDescriptor(), environment, consumedPast)
            };
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", construct);
         }
      },
      [&](const Verb& verb) {
         try {
            result << Verb::FromMeta(
               verb.GetVerb(), 
               Link(verb.GetArgument(), environment, consumedPast),
               verb.GetCharge(), 
               verb.GetVerbState()
            ).SetSource(
               Link(verb.GetSource(), environment, consumedPast)
            );
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", verb);
         }
      },
      [&](const Inner::MissingPast& past) {
         VERBOSE_MISSING_POINT_TAB(
            "Linking future point ", *this, " to past point ", past);

         if (mPriority != NoPriority and mPriority <= past.mPriority) {
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped past point with higher priority: ", past);
            LANGULUS_THROW(Link, "Past point of higher priority");
         }

         Inner::MissingPast pastShallowCopy;
         pastShallowCopy.mFilter = past.mFilter;
         if (not mContent) {
            if (not environment)
               LANGULUS_THROW(Link, "No environment provided for temporal flow");

            VERBOSE_MISSING_POINT(
               "(empty future point, so falling back to environment: ", environment, ')');

            if (not pastShallowCopy.Push(environment)) {
               if (not scope.IsOr())
                  LANGULUS_THROW(Link, "Scope not likable");
            }
         }
         else {
            if (not pastShallowCopy.Push(mContent)) {
               if (not scope.IsOr())
                  LANGULUS_THROW(Link, "Scope not linkable");
            }
            else consumedPast = true;
         }

         result << Abandon(pastShallowCopy.mContent);
      }
   );

   if (not found) {
      // Anything else just gets propagated                             
      result = Many {scope};
   }

   if (result.GetCount() < 2)
      result.MakeAnd();
   return Abandon(result);
}

/// Links the missing past points of the provided Neat, using mContent as     
/// the past, and returns a viable overwrite for mContent                     
///   @attention assumes argument is a valid scope                            
///   @param neat - the Neat to link                                          
///   @param environment - a fallback past provided by Temporal               
///   @param consumedPast - [out] set to true if anythingin this point has    
///      been used in any scope missing past                                  
///   @return the linked equivalent to the provided Neat                      
Many Missing::Link(const Neat& neat, const Block<>& environment, bool& consumedPast) const {
   Neat result;
   neat.ForEachTrait([&](const Trait& trait) {
      try {
         result << Trait::From(
            trait.GetTrait(),
            Link(trait, environment, consumedPast)
         );
      }
      catch (const Except::Link&) {
         VERBOSE_MISSING_POINT(Logger::DarkYellow,
            "Skipped branch: ", trait);
      }
   });

   neat.ForEachConstruct([&](const Construct& construct) {
      try {
         result << Construct {
            construct.GetType(),
            Link(construct.GetDescriptor(), environment, consumedPast)
         };
      }
      catch (const Except::Link&) {
         VERBOSE_MISSING_POINT(Logger::DarkYellow,
            "Skipped branch: ", construct);
      }
   });

   neat.ForEachTail([&](const Block<>& group) {
      // Compile anything else                                          
      result << Link(group, environment, consumedPast);
   });

   return Abandon(result);
}

/// Log the missing point                                                     
Missing::operator Text() const {
   Text result;
   result += '(';
   mFilter.Serialize(result);

   if (mPriority != NoPriority)
      result += Text {" !", mPriority};

   if (mContent) {
      result += ", ";
      mContent.Serialize(result);
   }

   result += ')';
   return result;
}

/// Default past point                                                        
MissingPast::MissingPast() {
   mFilter.MakePast();
}

/// Default future point                                                      
MissingFuture::MissingFuture() {
   mFilter.MakeFuture();
}

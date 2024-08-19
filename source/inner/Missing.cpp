///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Missing.hpp"
#include "Fork.hpp"
#include "../Temporal.hpp"
#include "../verbs/Do.inl"
#include "../verbs/Interpret.inl"

#if 1
   #define VERBOSE_MISSING_POINT(...)     Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_MISSING_POINT_TAB(...) const auto tabs = Logger::VerboseTab(__VA_ARGS__)
   #define VERBOSE_FUTURE(...)            Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_MISSING_POINT(...)     LANGULUS(NOOP)
   #define VERBOSE_MISSING_POINT_TAB(...) LANGULUS(NOOP)
   #define VERBOSE_FUTURE(...)            LANGULUS(NOOP)
#endif

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
Missing::Missing(const Many& filter, Real priority)
   : mPriority {priority} {
   mFilter.GatherFrom(filter, DataState::Missing);
   mFilter.SetState(filter.GetState());
}

/// Check if immediate contents are accepted by the filter of this point      
/// Verbs are always accepted                                                 
///   @param content - the content to check                                   
///   @return true if contents are accepted                                   
bool Missing::Accepts(const Many& content) const {
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
   mContent.ForEachDeep([&](const Many& b) {
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
      if (content.IsOr() and content.IsDense()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         Missing fork {mFilter, mPriority};
         fork.mContent.MakeOr();
            
         content.ForEach([&](const Many& subcontent) {
            atLeastOneSuccess |= fork.Push(subcontent);
         });

         mContent.SmartPush(IndexBack, Abandon(fork.mContent));
      }
      else if (content.IsDense()) {
         // Just nest-push                                              
         content.ForEach([&](const Many& subcontent) {
            atLeastOneSuccess |= Push(subcontent);
         });
      }
      else {
         // Sparse blocks are always inserted as-is                     
         // They are never linked, so not to affect contents outside    
         // this flow. This makes the flow impure, because it can be    
         // affected from the outside.                                  
         content.ForEach([&](const Many& subcontent) {
            mContent << &subcontent;
            atLeastOneSuccess = true;
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
      Many linked;
      try { linked = Link(content, mContent); }
      catch (const Except::Link&) { return false; }

      if (linked) {
         mContent << Abandon(linked);
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
///   @param context - a fallback past provided by Temporal                   
///   @return the linked equivalent to the provided scope                     
Many Missing::Link(const Many& scope, const Many& context) const {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsDeep()) {
      // Nest scopes, linking any past points in subscopes              
      scope.ForEach([&](const Many& subscope) {
         try {
            VERBOSE_MISSING_POINT_TAB("Linking subscope: ", subscope);
            result << Link(subscope, context);
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

   // Link all missing points in the provided scope, using context      
   const auto found = scope.ForEach(
      [&](const Trait& trait) {
         // Link a trait                                                
         try {
            VERBOSE_MISSING_POINT_TAB("Linking trait: ", trait);
            result << Trait::From(
               trait.GetTrait(), Link(trait, context)
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
         // Link a construct                                            
         try {
            VERBOSE_MISSING_POINT_TAB("Linking construct: ", construct);
            result << Construct {
               construct.GetType(), Link(construct.GetDescriptor(), context)
            };
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", construct);
         }
      },
      [&](const A::Verb& verb) {
         // Link a verb                                                 
         try {
            VERBOSE_MISSING_POINT_TAB("Linking verb: ", verb);
            result << Verb::FromMeta(
               verb.GetVerb(), 
               Link(verb.GetArgument(), context),
               verb.GetCharge(), 
               verb.GetVerbState()
            ).SetSource(Link(verb.GetSource(), context));
         }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", verb);
         }
      },
      [&](const Inner::MissingPast& past) {
         // Replace a missing past point with provided context          
         VERBOSE_MISSING_POINT_TAB("Linking past point: ", past);
         if (mPriority != NoPriority and mPriority <= past.mPriority) {
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped past point with higher priority: ", past);
            LANGULUS_THROW(Link, "Past point of higher priority");
         }

         Inner::MissingPast pastShallowCopy;
         pastShallowCopy.mFilter = past.mFilter;
         if (not pastShallowCopy.Push(context)) {
            if (not scope.IsOr())
               LANGULUS_THROW(Link, "Scope not likable");
         }
         result << Abandon(pastShallowCopy.mContent);
      }
   );

   if (not found) {
      // Anything else just gets propagated                             
      result = scope;
   }

   if (result.GetCount() < 2)
      result.MakeAnd();

   VERBOSE_MISSING_POINT("Link result: ", result);
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
Many Missing::Link(const Neat& neat, const Many& context) const {
   Neat result;
   neat.ForEachTrait([&](const Trait& trait) {
      // Link a trait inside the neat scope                             
      try {
         result << Trait::From(
            trait.GetTrait(),
            Link(trait, context)
         );
      }
      catch (const Except::Link&) {
         VERBOSE_MISSING_POINT(Logger::DarkYellow,
            "Skipped branch: ", trait);
      }
   });

   neat.ForEachConstruct([&](const Construct& construct) {
      // Link a construct inside the neat scope                         
      try {
         result << Construct {
            construct.GetType(),
            Link(construct.GetDescriptor(), context)
         };
      }
      catch (const Except::Link&) {
         VERBOSE_MISSING_POINT(Logger::DarkYellow,
            "Skipped branch: ", construct);
      }
   });

   neat.ForEachTail([&](const Many& group) {
      // Link anything else                                             
      result << Link(group, context);
   });

   return Abandon(result);
}

/// Log the missing point                                                     
Missing::operator Text() const {
   Text result;

   if ((mPriority and mPriority != NoPriority) or mContent) {
      result += '(';
      mFilter.Serialize(result);

      if (mPriority and mPriority != NoPriority)
         result += Text {" !", mPriority};

      if (mContent) {
         result += ", ";
         mContent.Serialize(result);
      }

      result += ')';
   }
   else mFilter.Serialize(result);
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

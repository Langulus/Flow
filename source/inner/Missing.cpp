///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Missing.hpp"
#include "Redundant.hpp"
#include <Langulus/Temporal.hpp>
#include <Langulus/Verbs/Do.hpp>
#include <Langulus/Verbs/Interpret.hpp>

#if 1
   #define VERBOSE_MISSING_ENABLED() 1
   #define VERBOSE_MISSING_POINT(...)     Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_MISSING_POINT_TAB(...) const auto tabs = Logger::VerboseTab(__VA_ARGS__)
   #define VERBOSE_FUTURE(...)            Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_MISSING_ENABLED() 0
   #define VERBOSE_MISSING_POINT(...)     LANGULUS(NOOP)
   #define VERBOSE_MISSING_POINT_TAB(...) LANGULUS(NOOP)
   #define VERBOSE_FUTURE(...)            LANGULUS(NOOP)
#endif

using namespace Langulus::Flow;


/// Initialize a missing point by a precompiled filter                        
///   @param above the missing point above this one                           
///   @param filter the filter to set                                         
///   @param priority the precedence of the point                             
Missing::Missing(Missing* above, const TMany<DMeta>& filter, Real priority)
   : mFilter   {filter}
   , mPriority {priority}
   , mAbove    {above} {}

/// Initialize a missing point by a filter, which will be precompiled.        
/// In other words: all meta data definitions will be extracted.              
///   @param above the missing point above this one                           
///   @param filter the filter to set                                         
///   @param priority the precedence of the point                             
Missing::Missing(Missing* above, const Many& filter, Real priority)
   : mPriority {priority}
   , mAbove    {above} {
   mFilter.GatherFrom(filter, [](Many const& i) { return i.IsMissing(); });
   mFilter.SetState(filter.GetState());
}

/// Check if immediate contents are accepted by the filter of this point.     
/// Verbs are always accepted.                                                
///   @param content the content to check                                     
///   @return true if contents are acceptable                                 
bool Missing::Accepts(const Many& content) const {
   if (not mFilter or content.template Is<Verb>())
      return true;

   for (auto type : mFilter) {
      if (content.CastsTo(type, 1, true))
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

/// Insert data into a past point                                             
///   @param content - the content to push                                    
///   @return true if mContent changed                                        
void MissingPast::FillPast(const Many& content) {
   if (not content) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("Can't push empty content");
      #endif
      throw Exception("Can't push empty content");
   }

   if (content.IsDeep()) {
      // Always nest deep contents, we must filter each part and        
      // make sure branches are correctly inserted in forks             
      if (content.IsOr() and not content.IsSparse()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         MissingPast fork {this, mFilter, mPriority};
         fork.mContent.EnableOr();
            
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            try {
               fork.FillPast(subcontent);
               atLeastOneSuccess = true;
            }
            catch (...) {}
         });

         if (not atLeastOneSuccess)
            throw Exception("All branches failed the push");

         mContent.Compose(Abandon(fork.mContent));
      }
      else if (not content.IsSparse()) {
         // Just nest-push                                              
         content.ForEach([&](const Many& subcontent) {
            FillPast(subcontent);
         });
      }
      else {
         // Sparse blocks are always inserted as-is, and never repeated 
         // They are never linked, so not to affect contents outside    
         // this flow. This makes the flow impure, because it can be    
         // affected from the outside.                                  
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            if (not mFilter) {
               mContent <<= &subcontent;
               atLeastOneSuccess = true;
            }
            else if (subcontent.GetType()) {
               for (const auto& type : mFilter) {
                  if (not subcontent.CastsTo(type))
                     continue;

                  mContent <<= &subcontent;
                  atLeastOneSuccess = true;
                  break;
               }
            }
         });

         if (not atLeastOneSuccess)
            throw Exception("Nothing was pushed");
      }

      return;
   }
   else if (content.template Is<Redundant>()) {
      // Redundant data serves only the purpose of filling past         
      // and acts as a deep container                                   
      content.ForEach([&](const Redundant& redundant) {
         FillPast(redundant.mContent);
      });
   }

   //                                                                   
   // If reached, we're pushing flat data                               
   // A handy lambda to commit any changes to the current mContent      
   const auto commit = [&](const Many& a) {
      if (a.IsSparse())
         mContent <<= a;               // Avoid duplications if sparse  
      else
         mContent << a;
   };

   if (mFilter) {
      // Filters are available, interpret source as requested           
      Verbs::Interpret interpret {mFilter};
      auto& output = interpret.GetOutput();
      if (Verbs::Do(interpret).In(content).Run() and output) {
         VERBOSE_MISSING_POINT(Logger::Green, 
            "Satisfying filter by interpreting ", content, " as ", output);
         commit(output);
      }
      else if (not mContent) {
         #if VERBOSE_MISSING_ENABLED()
            Logger::Error("Unsatisfied filter: ", mFilter);
         #endif
            throw Exception("Unsatisfied filter");
      }
   }
   //else commit(content);
}

/// Insert data into a future point. Any missing past points inside 'content' 
/// will be filled from either what's currently in this future point, or from 
/// contents of the future points above this one                              
///   @attention assumes 'content' has been Temporal::Compiled previously     
///   @param content - the content to push                                    
///   @return true if mContent changed                                        
void MissingFuture::FillFuture(const Many& content, Temporal& flow) {
   if (not content) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("Can't push empty content");
      #endif
      throw Exception("Can't push empty content");
   }

   if (content.IsDeep()) {
      // Always nest deep contents, we must filter each part and        
      // make sure branches are correctly inserted in forks             
      if (content.IsOr() and not content.IsSparse()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         MissingFuture fork {this, mFilter, mPriority};
         fork.mContent.EnableOr();
            
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            try {
               fork.FillFuture(subcontent, flow);
               atLeastOneSuccess = true;
            }
            catch (...) {}
         });

         if (not atLeastOneSuccess)
            throw Exception("All branches failed the push");

         mContent.Compose(Abandon(fork.mContent));
      }
      else if (not content.IsSparse()) {
         // Just nest-push                                              
         content.ForEach([&](const Many& subcontent) {
            FillFuture(subcontent, flow);
         });
      }
      else {
         // Sparse blocks are always inserted as-is, and never repeated 
         // They are never linked, so not to affect contents outside    
         // this flow. This makes the flow impure, because it can be    
         // affected from the outside.                                  
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            if (not mFilter) {
               mContent <<= &subcontent;
               atLeastOneSuccess = true;
            }
            else if (subcontent.GetType()) {
               for (const auto& type : mFilter) {
                  if (not subcontent.CastsTo(type))
                     continue;

                  mContent <<= &subcontent;
                  atLeastOneSuccess = true;
                  break;
               }
            }
         });

         if (not atLeastOneSuccess)
            throw Exception("Nothing was pushed");
      }

      // Contents were modified, remap futures below                    
      mBelow = {};
      Missing::RemapFutures(*this, mContent);
      return;
   }

   //                                                                   
   // If reached, we're pushing flat data                               
   // Fill any missing past points in the contents we're filling with   
   // If past fails to be satisfied within the current context, this    
   // function will throw and you're responsible of trying futures      
   // above this one - repeat until satisfied or nothing left above     
   Many linked;
   MissingFuture* context = this;
   while (context) {
      try {
         linked = Link(content, *context);
         break;
      }
      catch (...) {
         context = static_cast<MissingFuture*>(context->mAbove);
         continue;
      }
   }

   if (not context) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("None of the hierarchical past was satisfactory");
      #endif
      throw Exception("None of the hierarchical past was satisfactory");
   }

   if (mFilter) {
      // Filters are available, interpret contents as requested         
      Verbs::Interpret interpreter {mFilter};
      auto& output = interpreter.GetOutput();
      if (DispatchDeep(linked, interpreter) and output) {
         VERBOSE_MISSING_POINT(Logger::Green, 
            "Satisfying filter by interpreting ", linked, " as ", output);
         Commit(output, flow);
      }
      else if (not mContent) {
         #if VERBOSE_MISSING_ENABLED()
            Logger::Error("Unsatisfied filter: ", mFilter);
         #endif
            throw Exception("Unsatisfied filter");
      }
   }
   else Commit(linked, flow);

   // Contents were modified in a way that can introduce new            
   // futures below, so remap those                                     
   mBelow = {};
   Missing::RemapFutures(*this, mContent);
}

/// After a scope has been compiled, linked, and all that jazz, it comes      
/// the time to push in the stacks. If there are verbs that have non-default  
/// frequency or time point, they will be directed towards the frequency and  
/// time stacks. All else gets inserted in contents of this missing future    
/// point. It is important this dispatch is done at this final step, because  
/// rated/timed verbs still have to be linked with the relevant future point  
///   @param linked - the compiled & linked scope to insert                   
///   @param flow - the flow to use when inserting rated/timed verbs          
void MissingFuture::Commit(const Many& linked, Temporal& flow) {
   LglsAssumeDev(not linked.IsDeep(),
      "Can't commit a deep scope here");
   LglsAssumeDev(not linked.IsOr(),
      "Can't commit branches here");

   if (linked.IsSparse()) {
      // Avoid duplications if new content is sparse                    
      // No need to remap futures below, because sparse contents        
      // never link with anything                                       
      mContent <<= linked;
   }
   else if (linked.template Is<Verb>()) {
      //TODO what about rated/timed recipes?
      linked.ForEach([&](const Verb& v) {
         const auto time = v.GetTime();
         const auto rate = v.GetRate();
         TMany<Verb> local = v;

         if (time) {
            // Verb is timed, forward it to the time stack              
            local[0].SetTime(0);
            auto found = flow.mTimeStack.FindIt(time);
            if (not found) {
               flow.mTimeStack.Insert(time, flow);
               found = flow.mTimeStack.FindIt(time);
            }

            LglsAssumeDev(found->mFuture,
               "Invalid future");
            found->mFuture->Commit(local, found.GetValue());

            //found.GetValue().LinkRelative(local, v, entanglement);
         }
         else if (rate) {
            // Verb is rated, forward it to the frequency stack         
            local[0].SetRate(0);
            auto found = flow.mFrequencyStack.FindIt(rate);
            if (not found) {
               flow.mFrequencyStack.Insert(rate, flow);
               found = flow.mFrequencyStack.FindIt(rate);
            }

            LglsAssumeDev(found->mFuture,
               "Invalid future");
            found->mFuture->Commit(local, found.GetValue());

            /*LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );*/
         }
         else mContent << local;
      });
   }
   else mContent << linked;
}


/// Just a helper function for logging                                        
template<class T>
decltype(auto) Missing::VerboseLinking(const T& what, const MissingFuture& context) {
   #if VERBOSE_MISSING_ENABLED()
      if constexpr (CT::Same<T, Tag>) {
         Logger::Verbose("Linking tag ");
         Temporal::DumpTag(what);
      }
      else if constexpr (CT::Same<T, Recipe>) {
         Logger::Verbose("Linking recipe ");
         Temporal::DumpRecipe(what);
      }
      else if constexpr (CT::Same<T, Verb>) {
         Logger::Verbose("Linking verb ");
         Temporal::DumpVerb(what);
      }
      else if constexpr (CT::DerivedFrom<T, Missing>) {
         Logger::Verbose("Linking point ");
         Temporal::DumpMissing(what);
      }
      else if constexpr (CT::Same<T, Many>) {
         Logger::Verbose("Linking subscope ");
         bool unused = true;
         Temporal::DumpInner(what, false, unused);
      }
      else static_assert(false, "Unsupported linking");

      Logger::Append(" to point ");
      Temporal::DumpMissing(context);
      return Logger::Append(Logger::Tabs {1});
   #else
      return 0;
   #endif
}

/// Links the missing past points with the provided context                   
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param context - the future point we're using as past context           
///   @return the linked equivalent to the provided scope                     
Many Missing::Link(const Many& scope, const MissingFuture& context) const {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsDeep()) {
      // Nest scopes, linking any past points in subscopes              
      scope.ForEach([&](const Many& subscope) {
         const auto tab = VerboseLinking(subscope, context);
         try { result << Link(subscope, context); }
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

   // Link all missing past points in the provided scope using context  
   const auto found = scope.ForEach(
      [&](const Tag& tag) {
         // Link a tag                                                  
         const auto tab = VerboseLinking(tag, context);
         result << Tag::From(tag, Link(tag, context));
      },
      [&](const Recipe& recipe) {
         // Link a construct                                            
         const auto tab = VerboseLinking(recipe, context);
         result << Recipe::From(recipe, Link(recipe.GetDescriptor(), context));
      },
      [&](const Verb& verb) {
         // Link a verb                                                 
         const auto tab = VerboseLinking(verb, context);
         auto source = Link(verb.GetSource(), context);
         result << Verb::From(verb, Link(verb.GetArgument(), context)).SetSource(Abandon(source));
      },
      [&](const MissingPast& past) {
         // Replace a missing past point with provided context          
         const auto tab = VerboseLinking(past, context);
         if (mPriority > past.mPriority) {
            #if VERBOSE_MISSING_ENABLED()
               Logger::Error("Skipped because of precedence");
            #endif
            throw Exception("Skipped because of precedence");
         }

         if (past.mFilter) {
            MissingPast pastShallowCopy;
            pastShallowCopy.mFilter = past.mFilter;
            pastShallowCopy.FillPast(context.mContent);
            result << Abandon(pastShallowCopy.mContent);
         }
         else if (context.mContent) {
            // We're not allowed to modify the context! But we should   
            // modify the precedence, if it happens that the new        
            // contents should completely wrap around the old           
            auto& mutableContext = const_cast<MissingFuture&>(context);
            mutableContext.mPriority = past.mPriority;
            // Insert as redundant so that it doesn't clog the log      
            result << Redundant {mutableContext.mContent};
         }
         else {
            // Nothing to link with                                     
            #if VERBOSE_MISSING_ENABLED()
               Logger::Error("Nothing to link with - context is empty");
            #endif
            throw Exception("Nothing to link with");
         }
      }
   );

   if (not found)
      result = scope;   // Anything else just gets propagated           
   return Abandon(result);
}

/// Scan for future points below a given context                              
/// If a future point contains other future points below it that are of the   
/// same priority, then it's considered suspended                             
///   @param context - the future point to search below                       
///   @param stack - used for nesting deep contents                           
///   @return the hierarchy of future points below the context                
void Missing::RemapFutures(MissingFuture& context, const Many& stack) {
   if (not stack or stack.IsSparse())
      return;           // No point in scanning sparse stacks - they're 
                        // never linked with                            

   if (stack.IsOr())
      context.mBelow.MakeOr();

   if (stack.IsDeep()) {
      // Nest deep stack if dense                                       
      stack.ForEachRev([&](const Many& substack) {
         RemapFutures(context, substack);
      });
      return;
   }

   // Flat if reached                                                   
   stack.ForEachRev(
      [&](const Tag& tag) {
         // Nest inside traits                                          
         RemapFutures(context, tag.GetData());
      },
      [&](const Recipe& recipe) {
         // Nest inside constructs                                      
         RemapFutures(context, recipe.GetDescriptor());
      },
      [&](const Verb& verb) {
         // Nest inside verbs                                           
         RemapFutures(context, verb.GetArgument());
         RemapFutures(context, verb.GetSource());
      },
      [&](const MissingFuture& below_const) {
         // Nest/register missing future points                         
         auto& below = const_cast<MissingFuture&>(below_const);
         below.mSuspended = false;
         below.mAbove = &context;
         below.mBelow = {};
         RemapFutures(below, below.mContent);

         if (below.mPriority == context.mPriority and below.mBelow) {
            // More missing futures below, suspend this one             
            below.mSuspended = true;
            below.mBelow.ForEachDeep([&](MissingFuture& next) {
               next.mAbove = &context;
               context.mBelow << &next;
            });
         }
         else context.mBelow << &below;
      }
   );
}

/// Log the missing point                                                     
Missing::operator Text() const {
   Text result;

   if (mSuspended) {
      mContent.Serialize(result);
      return result;
   }

   if (mPriority or mContent) {
      result += '(';
      mFilter.Serialize(result);

      if (mPriority)
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

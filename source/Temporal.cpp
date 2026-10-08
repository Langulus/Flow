///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/Time.inl>
#include <Langulus/Code.inl>
#include <Langulus/Neat.hpp>
#include <Langulus/Temporal.hpp>
#include <Langulus/Verbs/Do.hpp>
#include "inner/Missing.hpp"
#include "inner/Entangled.hpp"
#include "inner/Redundant.hpp"

#define LglsVerboseEnabled 0
#include <Langulus/Logger/ToggleVerbose.hpp>

#define LglsVerboseTempo(...) \
   LglsVerbose(Verbose, *this, ": ", __VA_ARGS__)

#define LglsVerboseTempoScoped(...) \
   LglsVerboseScoped(Verbose, *this, ": ", __VA_ARGS__)

using namespace Langulus;
using namespace Langulus::Flow;


/// Default constructor, add the initial missing future point                 
Temporal::Temporal() {
   mPriorityStack << MissingFuture {};
   mFuture = mPriorityStack.As<MissingFuture*>();
}

/// Construct as a sub-flow                                                   
///   @attention assumes parent is a valid pointer                            
///   @param parent the parent flow                                           
Temporal::Temporal(Temporal* parent)
   : mParent {parent} {
   mPriorityStack << MissingFuture {};
   mFuture = mPriorityStack.As<MissingFuture*>();
}

/// For logging temporal instances                                            
Temporal::operator Text() const {
   return Text::Mention(this);
}

/// Reset progress for the priority stack                                     
void Temporal::Reset() {
   // Reset timers                                                      
   mStart = mNow = mPrevTime = {};

   // Reset the execution state of all verbs in the priority stack      
   ResetInner(mPriorityStack);

   // Reset all entanglements                                           
   for (auto b : mEntanglements)
      (*b)->mDone = false;
}

/// Reset progress for all verbs inside a scope                               
///   @param scope scope to reset                                             
void Temporal::ResetInner(Many& scope) {
   scope.ForEach(
      [&](Many& m) {
         if (not m.IsSparse())
            ResetInner(m);
      },
      [&](Missing& missing) {
         if (not missing.mContent.IsSparse())
            ResetInner(missing.mContent);
      },
      [&](Entangled& entangled) {
         if (not entangled.mTrueContent.IsSparse())
            ResetInner(entangled.mTrueContent);
         if (not entangled.mFalseContent.IsSparse())
            ResetInner(entangled.mFalseContent);
      },
      [&](Tag& tag) {
         if (not tag.IsSparse()) {
            auto data = tag.GetData();
            ResetInner(data);
         }
      },
      [&](Recipe& recipe) {
         auto descriptor = recipe.GetDescriptor();
         ResetInner(descriptor);
      },
      [&](Neat& neat) {
         neat.ForEachTag([this](Tag& tag) {
            auto data = tag.GetData();
            ResetInner(data);
         });
         neat.ForEachRecipe([this](Recipe& recipe) {
            auto descriptor = recipe.GetDescriptor();
            ResetInner(descriptor);
         });
         neat.ForEachTail([this](Many& stuff) {
            ResetInner(stuff);
         });
      },
      [&](Verb& verb) {
         ResetInner(verb.GetSource());
         auto argument = verb.GetArgument();
         ResetInner(argument);
         verb.Clear();
      }
   );
}

/// Compare two flows                                                         
///   @param other the flow to compare with                                   
///   @return true if both flows are the same                                 
bool Temporal::operator == (Temporal const& other) const {
   return mFrequencyStack == other.mFrequencyStack
      and mTimeStack      == other.mTimeStack
      and mPriorityStack  == other.mPriorityStack;
}

/// Check if flow contains anything                                           
///   @return true if flow contains something                                 
bool Temporal::IsValid() const {
   return mPriorityStack or mTimeStack or mFrequencyStack;
}

/// Get the accumulated running time across all Updates                       
///   @return the time                                                        
auto Temporal::GetUptime() const -> Time {
   return mNow - mStart;
}

/// Get the difference in time between the last two updates                   
///   @return the time differences                                            
auto Temporal::GetDeltaTime() const -> Time {
   return mNow - mPrevTime;
}

/// Advance the flow - moves time forward, executes stacks                    
///   @param dt delta time                                                    
///   @param sideffects any side effects produced by executing                
///   @return true if no exit was requested                                   
bool Temporal::Update(Time dt, Many& sideffects) {
   if (mStart == mNow) {
      // We're at the beginning of time - execute the priority stack    
      Many unusedContext;
      bool unusedSkip = false;
      Flow::Execute(mPriorityStack, unusedContext, sideffects, false, unusedSkip);
   }

   // Avoid updating anything else, if no time had passed               
   if (not dt)
      return true;

   // Advance the global cycler for the flow                            
   mPrevTime = mNow;
   mNow += dt;

   // Execute flows that occur periodically                             
   for (auto pair : mFrequencyStack) {
      auto* innerFlow = *pair.GetVal();
      innerFlow->mNow += dt;
      auto ticks = innerFlow->GetUptime().Seconds() / mRatePeriod.Seconds();
      const auto frequency = pair.GetKey();
      while (ticks >= frequency) {
         // Time to execute the periodic flow                           
         innerFlow->Reset();
         innerFlow->Update({}, sideffects);
         ticks -= frequency;
      }

      // Make sure any leftover time is returned to the periodic flow   
      innerFlow->mNow = innerFlow->mStart + mRatePeriod * ticks;
   }

   // Execute flows that occur after a given point in time              
   const auto ticks = GetUptime().Seconds() / mTimePeriod.Seconds();
   for (auto pair : mTimeStack) {
      const auto timepoint = pair.GetKey();
      if (timepoint > ticks) {
         // The time stack is sorted, so no point in continuing         
         break;
      }

      // Always update all time points before the tick count            
      // They might have periodic flows inside                          
      auto* innerFlow = *pair.GetVal();
      innerFlow->Update(dt, sideffects);
   }

   return true;
}

/// Merge a flow                                                              
///   @param other the flow to merge with this one                            
void Temporal::Merge(Temporal const& other) {
   // Merge priority stacks                                             
   mPriorityStack.MergeRange(other.mPriorityStack);

   // Merge time stacks                                                 
   for (auto pair : other.mTimeStack) {
      const auto timepoint = pair.GetKey();
      auto found = mTimeStack.Merge(timepoint);
      auto* innerFlow = pair.GetVal();
      auto v = found.GetVal();
      if (v)
         v->Merge(*innerFlow);
      else
         v.Assign(innerFlow);
   };

   // Merge frequency stacks                                            
   for (auto pair : other.mFrequencyStack) {
      const auto frequency = pair.GetKey();
      auto found = mFrequencyStack.Merge(frequency);
      auto* innerFlow = pair.GetVal();
      auto v = found.GetVal();
      if (v)
         v->Merge(*innerFlow);
      else
         v.Assign(innerFlow);
   };
}

/// Push a scope of verbs and data to the flow                                
/// The following rules are used to place the data:                           
///   1. Data is always inserted at future missing points (??) - there is     
///      always at least one such point in any flow (at the back of the       
///      main scope)                                                          
///   2. If inserted data has a past missing point (?), that point will be    
///      filled with whatever data is already available at the place of       
///      insertion                                                            
///   3. Future and past points might have a filter, which decides what       
///      kind of data can be inserted at that point                           
///   4. Future and past points might have priority, which decides what       
///      kind of verbs are allowed inside. Priorities are set when a          
///      verb is inserted. A verb of higher-or-equal priority can never be    
///      inserted in a point of lower priority. A verb of higher-or-equal     
///      priority can only wrap lower-or-equal priority scopes in itself.     
///   5. Future and past points might have branches, which forces shallow     
///      duplication of missing future/past content when linking              
///   6. Verbs with different frequency and time charge go to the             
///      corresponding stacks, and are stripped from such properties;         
///      from there on, they're handled conventionally, by the                
///      aforementioned rules in the context of that stack                    
///   7. When this temporal flow is not at the beginning of time, any new     
///      verbs that would've been executed until now are executed, and their  
///      side effects are returned.                                           //TODO what if they affect other verbs in the flow, like inside of them, or producing contexts that can be used by other verbs? this is a huge deal
///   @attention assumes argument is a valid scope                            
///   @param scope the scope to analyze and push                              
///   @return any side effects of the insertion                               
Many Temporal::PushInner(Many scope) {
   Many compiled;

   {
      #if LglsVerboseEnabled
         LglsVerboseTempoScoped("Pushing: ");
         bool unused = true;
         DumpInner(scope, true, unused);
      #endif

      // Compile pushed scope to an intermediate format                 
      compiled = Compile(scope);
   }
   {
      #if LglsVerboseEnabled
         LglsVerboseTempoScoped("Compiled to: ");
         bool unused = true;
         DumpInner(compiled, true, unused);
      #endif
   }

   // Link new scope with the available stacks                          
   try { Link(compiled, {}); }
   catch (...) { return {}; }

   Dump();

   // Execute the new scope and return any side effects                 
   Many sideffects;
   Update({}, sideffects);
   return Abandon(sideffects);
}

/// Compiles a scope into an intermediate form, used by the flow              
///   @attention assumes argument is a valid scope                            
///   @param scope the scope to compile                                       
///   @param priority the priority to set for any missing point created       
///      for the provided scope.                                              
///   @return the compiled scope                                              
Many Temporal::Compile(Many const& scope, Real priority) {
   Many result;
   if (scope.IsOr())
      result.EnableOr();

   if (scope.IsPast()) {
      // Convert the scope to a MissingPast intermediate format         
      result = MissingPast {nullptr, scope, priority};
      return Abandon(result);
   }
   else if (scope.IsFuture()) {
      // Convert the scope to a MissingFuture intermediate format       
      result = MissingFuture {nullptr, scope, 0};
      return Abandon(result);
   }
   else if (scope.IsDeep()) {
      if (scope.IsSparse()) {
         // Sparse scopes are always inserted, even if empty. They act  
         // as handles, that can change context externally. They are    
         // never compiled, because that would require fiddling with    
         // the contents of the handle.                                 
         // @attention any sparse element inside a flow will cause that 
         //    flow to become impure, as in, it can be affected by      
         //    external factors, and is no longer purely functional.    
         scope.ForEach([&](Many const& subscope) {
            result << &subscope;
         });
      }
      else {
         // Nest dense deep scopes                                      
         scope.ForEach([&](Many const& subscope) {
            result << Compile(subscope, priority);
         });
      }
      return Abandon(result);
   }

   const auto done = scope.ForEach(
      [&](Tag const& subscope) {
         // Compile traits                                              
         result << Tag::From(subscope, Compile(subscope.GetData(), priority));
      },
      [&](Recipe const& subscope) {
         // Compile constructs                                          
         result << Recipe::From(subscope, Compile(subscope.GetDescriptor(), priority));
      },
      [&](Verb const& subscope) {
         // Compile verbs                                               
         result << Verb::From(subscope, Compile(subscope.GetArgument(), subscope.GetPrecedence()))
                                    .In(Compile(subscope.GetSource(),   subscope.GetPrecedence()));
      }
   );

   if (not done) {
      // Just propagate content                                         
      result = scope;
   }

   return Abandon(result);
}

/// Link a scope's past points to future points that are on the stack         
///   @param scope the scope to link and insert                               
///   @param entanglementAbove an optional entanglement from above scope      
void Temporal::Link(Many const& scope, Ref<Entanglement> const& entanglementAbove) {
   LglsAssumeDev(mFuture, "Invalid future");

   // Every time we push an OR scope we create an entanglement          
   Ref<Entanglement> entanglement;
   if (scope.IsOr()) {
      entanglement = mEntanglements.Emplace(
         entanglementAbove ? entanglementAbove->mParent : nullptr
      );
   }
   else entanglement = entanglementAbove;

   if (scope.IsDeep()) {
      if (scope.IsSparse()) {
         // Sparse blocks are always pushed directly without linking    
         // anything, because that would require changing data behind   
         // the handle. This allows for specifying contexts externally, 
         // but also makes the flow impure, because it allows it to be  
         // affected by external influence.                             
         scope.ForEach([&](Many const& sub) {
            LglsAssert(PushFutures(&sub, *mFuture, entanglement),
                       "Couldn't push to future");
         });
      }
      else {
         // Nest-link dense deep scope                                  
         scope.ForEach([&](Many const& sub) {
            Link(sub, entanglement);
         });
      }

      return;
   }

   // Handle shallow scope                                              
   const auto linked = scope.ForEach(
      [&](Tag const& t) {
         // Forward to all future points in the priority stack          
         TMany<Tag> local = t;
         LglsAssert(PushFutures(local, *mFuture, entanglement),
                    "Couldn't push to future");
      },
      [&](Recipe const& c) {
         // Forward to all future points in the priority stack          
         TMany<Recipe> local = c;
         LglsAssert(PushFutures(local, *mFuture, entanglement),
                    "Couldn't push to future");
      },
      [&](Verb const& v) {
         if (v.IsVerb<Verbs::Do>()) {
            // "Do" verbs act as context/mass/rate/time setters         
            // Don't push them, but use them to set environment for     
            // any sub-verbs                                            
            if (v.GetSource()) {
               LglsAssert(PushFutures(v.GetSource(), *mFuture, entanglement),
                          "Couldn't push to future");
            }

            LinkRelative(v.GetArgument(), v, entanglement);
         }
         /*else if (v.GetTime()) {
            // Verb is timed, forward it to the time stack              
            TMany<Verb> local = v;
            local[0].SetTime(0);

            auto found = mTimeStack.FindIt(v.GetTime());
            if (not found) {
               mTimeStack.Insert(v.GetTime(), this);
               found = mTimeStack.FindIt(v.GetTime());
            }

            found.GetValue().LinkRelative(local, v, entanglement);
         }
         else if (v.GetRate()) {
            // Verb is rated, forward it to the frequency stack         
            TMany<Verb> local = v;
            local[0].SetRate(0);

            auto found = mFrequencyStack.FindIt(v.GetRate());
            if (not found) {
               mFrequencyStack.Insert(v.GetRate(), this);
               found = mFrequencyStack.FindIt(v.GetRate());
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }*/
         else {
            // Forward it to the priority stack                         
            TMany<Verb> local = v;
            LglsAssert(PushFutures(local, *mFuture, entanglement),
                       "Couldn't push to future");
         }
      }
   );

   if (not linked) {
      // Still not linked? Probably an idea. Push it to the futures     
      // This may fail, but it doesn't really matter                    
      PushFutures(scope, *mFuture, entanglement);
   }
}

/// Push a scope into future points already available in the flow, but do it  
/// in a manner similar in energy to a given verb                             
///   @param scope the scope to push                                          
///   @param override the reference verb                                      
///   @param entanglementAbove an optional entanglement from above scope      
void Temporal::LinkRelative(
   Many const& scope,
   Verb const& override,
   Ref<Entanglement> const& entanglementAbove
) {
   LglsAssumeDev(mFuture, "Invalid future");

   // Every time we push an OR scope we create an entanglement          
   Ref<Entanglement> entanglement;
   if (scope.IsOr()) {
      entanglement = mEntanglements.Emplace(
         entanglementAbove ? entanglementAbove->mParent : nullptr
      );
   }
   else entanglement = entanglementAbove;

   if (scope.IsDeep()) {
      // Nest deep scope                                                
      scope.ForEach([&](Many const& sub) {
         LinkRelative(sub, override, entanglement);
      });
      return;
   }

   // Handle shallow scope                                              
   scope.ForEach(
      [&](Tag const& t) {
         TMany<Tag> local = t;

         // Forward to future point in appropriate stack, according to  
         // the override verb                                           
         /*if (override.GetTime()) {
            // Tag is timed, forward it to the time stack             
            auto found = mTimeStack.FindIt(override.GetTime());
            if (not found) {
               mTimeStack.Insert(override.GetTime(), this);
               found = mTimeStack.FindIt(override.GetTime());
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }
         else if (override.GetRate()) {
            // Verb is rated, forward it to the frequency stack         
            auto found = mFrequencyStack.FindIt(override.GetRate());
            if (not found) {
               mFrequencyStack.Insert(override.GetRate(), this);
               found = mFrequencyStack.FindIt(override.GetRate());
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }
         else {*/
            // Forward it to the priority stack                         
            LglsAssert(PushFutures(local, *mFuture, entanglement),
                       "Couldn't push to future");
         //}
      },
      [&](Recipe const& c) {
         TMany<Recipe> local = c;

         // Forward to future point in appropriate stack,               
         // according to the override verb                              
         /*if (override.GetTime()) {
            // Tag is timed, forward it to the time stack             
            auto found = mTimeStack.FindIt(override.GetTime());
            if (not found) {
               mTimeStack.Insert(override.GetTime(), this);
               found = mTimeStack.FindIt(override.GetTime());
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }
         else if (override.GetRate()) {
            // Verb is rated, forward it to the frequency stack         
            auto found = mFrequencyStack.FindIt(override.GetRate());
            if (not found) {
               mFrequencyStack.Insert(override.GetRate(), this);
               found = mFrequencyStack.FindIt(override.GetRate());
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }
         else {*/
            // Forward it to the priority stack                         
            LglsAssert(PushFutures(local, *mFuture, entanglement),
                       "Couldn't push to future");
         //}
      },
      [&](Verb const& v) {
         // Multiply verb energy and merge contexts                     
         const Verb localOverride = v * override;

         if (v.IsVerb<Verbs::Do>()) {
            // "Do" verbs act as context/mass/rate/time setters         
            // Don't push them, but use them to set environment for     
            // any sub-verbs                                            
            if (v.GetSource()) {
               LglsAssert(PushFutures(v.GetSource(), *mFuture, entanglement),
                          "Couldn't push to future");
            }

            LinkRelative(v.GetArgument(), localOverride, entanglement);
         }
         /*else if (localOverride.GetTime()) {
            // Verb is timed, forward it to the time stack              
            const auto time = localOverride.GetTime();
            TMany<Verb> local = v;
            local[0].SetTime(0);

            auto found = mTimeStack.FindIt(time);
            if (not found) {
               mTimeStack.Insert(time, this);
               found = mTimeStack.FindIt(time);
            }

            found.GetValue().LinkRelative(local, localOverride, entanglement);
         }
         else if (localOverride.GetRate()) {
            // Verb is rated, forward it to the frequency stack         
            const auto rate = localOverride.GetRate();
            TMany<Verb> local = v;
            local[0].SetRate(0);
            //if (not local[0].GetSource()) //TODO check if this is allowed - no other branch seems to override source
            //   local[0].SetSource(localOverride.GetSource());

            auto found = mFrequencyStack.FindIt(rate);
            if (not found) {
               mFrequencyStack.Insert(rate, this);
               found = mFrequencyStack.FindIt(rate);
            }

            LglsAssumeDev(found.GetValue().mFuture,
               "Invalid future");
            LANGULUS_ASSERT(
               found.GetValue().PushFutures(local, *found.GetValue().mFuture, entanglement),
               Flow, "Couldn't push to future"
            );
         }*/
         else {
            // Forward it to the priority stack                         
            // Collapse all verb charges at this point                  
            /*TMany<Verb> local = v;
            local[0].SetMass(localOverride.GetMass());
            local[0].SetPriority(localOverride.GetPriority());

            // Always push any valid source to the future, so that      
            // missing past can get satisfied by it                     
            if (override.GetSource()) {
               LANGULUS_ASSERT(
                  PushFutures(override.GetSource(), *mFuture, entanglement),
                  Flow, "Couldn't push to future"
               );
            }*/
            TMany<Verb> local = localOverride;
            LglsAssert(PushFutures(local, *mFuture, entanglement),
                       "Couldn't push to future");
         }
      }
   );
}

/// Links the missing past points of the provided scope, with the missing     
/// future points of the provided stack. But anything new could go into       
/// old future points, as long as state and filters allows it!                
///   @attention assumes argument is a valid scope                            
///   @param scope the scope to link                                          
///   @param future [in/out] the future to link with                          
///   @param entanglementAbove an optional entanglement from above scope      
///   @return true if scope was linked successfully, either in the provided   
///      'future', or in any of the futures below it                          
bool Temporal::PushFutures(
   Many const& scope, MissingFuture& future,
   Ref<Entanglement> const& entanglementAbove
) noexcept {
   bool atLeastOneSuccess = false;
   try {
      // Try to link here                                               
      future.FillFuture(scope, *this);
      atLeastOneSuccess = true;
   }
   catch (...) {}

   // If reached, then scope wasn't linked in the immediate future      
   // Dig deeper for any future points below the provided one           
   if (not atLeastOneSuccess or not scope.IsExecutableDeep()) {
      future.mBelow.ForEach([&](MissingFuture& below) {
         atLeastOneSuccess |= PushFutures(scope, below, entanglementAbove);
         // Continue linking only if the future is uncertain            
         //return not (future.mBelow.IsOr() and atLeastOneSuccess);
      });
   }

   /*if (atLeastOneSuccess) {
      bool first = true;
      DumpInner(future.mContent, true, first);
   }*/

   return atLeastOneSuccess;
}

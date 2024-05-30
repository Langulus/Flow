///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Time.inl"
#include "Code.inl"
#include "Resolvable.inl"
#include "inner/Missing.hpp"
#include "inner/Fork.hpp"
#include "Temporal.hpp"

#if 0
   #define VERBOSE_TEMPORAL(...)       Logger::Verbose(*this, ": ", __VA_ARGS__)
   #define VERBOSE_TEMPORAL_TAB(...)   const auto tab = Logger::Verbose(*this, ": ", __VA_ARGS__, Logger::Tabs{})
#else
   #define VERBOSE_TEMPORAL(...)       LANGULUS(NOOP)
   #define VERBOSE_TEMPORAL_TAB(...)   LANGULUS(NOOP)
#endif

using namespace Langulus::Flow;


/// Default constructor, add the initial missing future point                 
///   @param environment - the initial flow environment                       
Temporal::Temporal() {
   mPriorityStack << Inner::MissingFuture {};
}

/// Construct as a sub-flow                                                   
///   @attention assumes parent is a valid pointer                            
///   @param parent - the parent flow                                         
Temporal::Temporal(Temporal* parent)
   : mParent {parent} {
   mPriorityStack << Inner::MissingFuture {};
}

/// Serialize temporal as Code                                                
Temporal::operator Code() const {
   return IdentityOf(this);
}

/// Serialize temporal as debug string                                        
Temporal::operator Text() const {
   return IdentityOf(this);
}

/// Reset progress for the priority stack                                     
void Temporal::Reset() {
   mStart = mNow = {};
   //TODO reset stack verb done statuses
   VERBOSE_TEMPORAL("Reset");
}
   
/// Compare two flows                                                         
///   @param other - the flow to compare with                                 
///   @return true if both flows are the same                                 
bool Temporal::operator == (const Temporal& other) const {
   return mFrequencyStack == other.mFrequencyStack
      and mTimeStack == other.mTimeStack
      and mPriorityStack == other.mPriorityStack;
}

/// Check if flow contains anything executable                                
///   @return true if flow contains at least one verb                         
bool Temporal::IsValid() const {
   return mPriorityStack or mTimeStack or mFrequencyStack;
}

/// Dump the contents of the flow to the log                                  
void Temporal::Dump() const {
   Logger::Verbose(mPriorityStack);
   Logger::Verbose(mTimeStack);
   Logger::Verbose(mFrequencyStack);
}

/// Get the accumulated running time across all Updates                       
///   @return the time                                                        
Langulus::Time Temporal::GetUptime() const {
   return mNow - mStart;
}

/// Advance the flow - moves time forward, executes stacks                    
///   @param dt - delta time                                                  
///   @return true if no exit was requested                                   
bool Temporal::Update(Time dt) {
   if (mStart == mNow) {
      // We're at the beginning of time - execute the priority stack    
      VERBOSE_TEMPORAL(Logger::Purple,
         "Flow before execution: ", mPriorityStack);

      Many unused;
      Execute(mPriorityStack, unused);

      VERBOSE_TEMPORAL(Logger::Purple,
         "Flow after execution: ", mPriorityStack);
   }

   // Avoid updating anything else, if no time had passed               
   if (not dt)
      return true;

   // Advance the global cycler for the flow                            
   mNow += dt;

   // Execute flows that occur periodically                             
   for (auto pair : mFrequencyStack) {
      pair.mValue.mNow += dt;
      auto ticks = pair.mValue.GetUptime().Seconds() / mPeriod.Seconds();

      while (ticks >= pair.mKey) {
         // Time to execute the periodic flow                           
         pair.mValue.Reset();
         pair.mValue.Update();
         ticks -= pair.mKey;
      }

      // Make sure any leftover time is returned to the periodic flow   
      pair.mValue.mNow = pair.mValue.mStart + mPeriod * ticks;
   }

   // Execute flows that occur after a given point in time              
   const auto ticks = GetUptime().Seconds() / mPeriod.Seconds();
   for (auto pair : mTimeStack) {
      if (pair.mKey > ticks) {
         // The time stack is sorted, so no point in continuing         
         break;
      }

      // Always update all time points before the tick count            
      // They might have periodic flows inside                          
      pair.mValue.Update(dt);
   }

   return true;
}

/// Merge a flow                                                              
///   @param other - the flow to merge with this one                          
void Temporal::Merge(const Temporal& other) {
   // Concatenate priority stacks                                       
   mPriorityStack += other.mPriorityStack;

   // Merge time stacks                                                 
   for (auto pair : other.mTimeStack) {
      auto found = mTimeStack.FindIt(pair.mKey);
      if (not found) {
         // New time point required                                     
         mTimeStack.Insert(pair.mKey, this);
         found = mTimeStack.FindIt(pair.mKey);
      }

      found.mValue->Merge(pair.mValue);
   };

   // Merge frequency stacks                                            
   for (auto pair : other.mFrequencyStack) {
      auto found = mFrequencyStack.FindIt(pair.mKey);
      if (not found) {
         // New time point required                                     
         mFrequencyStack.Insert(pair.mKey, this);
         found = mFrequencyStack.FindIt(pair.mKey);
      }

      found.mValue->Merge(pair.mValue);
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
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to analyze and push                            
///   @return true if the flow changed                                        
bool Temporal::Push(Many scope) {
   VERBOSE_TEMPORAL_TAB("Pushing: ", scope);

   // Compile pushed scope to an intermediate format                    
   auto compiled = Compile(scope, Inner::NoPriority);
   VERBOSE_TEMPORAL("Compiled to: ", compiled);

   // Link new scope with the available stacks                          
   try { Link(compiled); }
   catch (...) {
      return false;
   }

   if (mPriorityStack) {
      VERBOSE_TEMPORAL(Logger::Purple, "Priority flow: ", mPriorityStack);
   }
   if (mTimeStack) {
      VERBOSE_TEMPORAL(Logger::Purple, "Time flow: ", mTimeStack);
   }
   if (mFrequencyStack) {
      VERBOSE_TEMPORAL(Logger::Purple, "Frequency flow: ", mFrequencyStack);
   }
   return true;
}

/// This will omit any compile-time junk that remains in the provided         
/// scope, so we can execute it conventionally                                
///   @param scope - the scope to collapse                                    
///   @return the collapsed scope                                             
/*Many Temporal::Collapse(const Block<>& scope) {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsDeep()) {
      // Nest deep scopes                                               
      scope.ForEach([&](const Block<>& subscope) {
         auto collapsed = Collapse(subscope);
         if (collapsed)
            result << Abandon(collapsed);
      });

      if (result.GetCount() < 2)
         result.MakeAnd();
      return Abandon(result);
   }

   const auto done = scope.ForEach(
      [&](const Trait& subscope) {
         // Collapse traits                                             
         auto collapsed = Collapse(subscope);
         if (collapsed) {
            result << Trait::From(
               subscope.GetTrait(), 
               Abandon(collapsed)
            );
         }
      },
      [&](const Construct& subscope) {
         // Collapse constructs                                         
         auto collapsed = Collapse(subscope.GetDescriptor());
         if (collapsed) {
            result << Construct {
               subscope.GetType(),
               Abandon(collapsed),
               subscope.GetCharge()
            };
         }
      },
      [&](const Verb& subscope) {
         // Collapse verbs                                              
         auto collapsedArgument = Collapse(subscope.GetArgument());
         if (collapsedArgument) {
            auto v = Verb::FromMeta(
               subscope.GetVerb(),
               Abandon(collapsedArgument),
               subscope.GetCharge(),
               subscope.GetVerbState()
            ).SetSource(Collapse(subscope.GetSource()));
            result << Abandon(v);
         }
      },
      [&](const Inner::MissingPast& subscope) {
         // Collapse missing pasts                                      
         if (subscope.IsSatisfied())
            result << subscope.mContent;
      },
      [&](const Inner::MissingFuture& subscope) {
         // Collapse missing futures                                    
         if (subscope.IsSatisfied())
            result << subscope.mContent;
      }
   );

   if (not done and scope)
      result = scope;
   if (result.GetCount() < 2)
      result.MakeAnd();
   return Abandon(result);
}

/// This will omit any compile-time junk that remains in the provided         
/// scope, so we can execute it conventionally                                
///   @param scope - the scope to collapse                                    
///   @return the collapsed scope                                             
Many Temporal::Collapse(const Neat&) {
   TODO();
   return {};
}*/

/// Compiles a scope into an intermediate form, used by the flow              
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to compile                                     
///   @param priority - the priority to set for any missing point created     
///      for the provided scope.                                              
///   @return the compiled scope                                              
Many Temporal::Compile(const Block<>& scope, Real priority) {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsPast()) {
      // Convert the scope to a MissingPast intermediate format         
      result = Inner::MissingPast {scope, priority};
      return Abandon(result);
   }
   else if (scope.IsFuture()) {
      // Convert the scope to a MissingFuture intermediate format       
      result = Inner::MissingFuture {scope, priority};
      return Abandon(result);
   }
   else if (scope.IsDeep()) {
      // Nest deep scopes                                               
      scope.ForEach([&](const Block<>& subscope) {
         result << Compile(subscope, priority);
      });
      return Abandon(result);
   }

   const auto done = scope.ForEach(
      [&](const Trait& subscope) {
         // Compile traits                                              
         result << Trait::From(
            subscope.GetTrait(), 
            Compile(subscope, priority)
         );
      },
      [&](const Construct& subscope) {
         // Compile constructs                                          
         result << Construct {
            subscope.GetType(),
            Compile(subscope.GetDescriptor(), priority),
            subscope.GetCharge()
         };
      },
      [&](const Verb& subscope) {
         // Compile verbs                                               
         auto v = Verb::FromMeta(
            subscope.GetVerb(),
            Compile(subscope.GetArgument(), subscope.GetPriority()),
            subscope.GetCharge(),
            subscope.GetVerbState()
         ).SetSource(Compile(subscope.GetSource(), subscope.GetPriority()));
         result << Abandon(v);
      }
   );

   if (not done) {
      // Just propagate content                                         
      result = scope;
   }

   return Abandon(result);
}

/// Compiles a neat descriptor into an intermediate form, used by the flow    
///   @attention assumes argument is a valid scope                            
///   @param neat - the Neat to compile                                       
///   @param priority - the priority to set for any missing point created     
///      for the provided scope.                                              
///   @return the compiled scope                                              
Many Temporal::Compile(const Neat& neat, Real priority) {
   Neat result;
   neat.ForEachTrait([&](const Trait& subscope) {
      // Compile traits                                                 
      result << Trait::From(
         subscope.GetTrait(),
         Compile(subscope, priority)
      );
   });

   neat.ForEachConstruct([&](const Construct& subscope) {
      // Compile constructs                                             
      result << Construct {
         subscope.GetType(),
         Compile(subscope.GetDescriptor(), priority),
         subscope.GetCharge()
      };
   });

   neat.ForEachTail([&](const Block<>& group) {
      // Compile anything else                                          
      result << Compile(group, priority);
   });

   return Abandon(result);
}

/// Links the missing past points of the provided scope, with the missing     
/// future point (or any nested future points inside). Anything could be      
/// pushed to provided future point as a fallback, as long as state and       
/// filters allows it!                                                        
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param future - [in/out] the future point to place inside               
///   @return true if scope was linked successfully                           
/*bool Temporal::PushFutures(const Many& scope, Inner::MissingFuture& future) {
   // Attempt linking to the contents first                             
   if (PushFutures(scope, future.mContent))
      return true;

   //                                                                   
   // If reached, then future point is flat and boring, fallback by     
   // directly linking against it                                       
   VERBOSE_TEMPORAL_TAB("Linking to: ", future);
   return future.Push(scope);
}*/

/// Links the missing past points of the provided scope, with the missing     
/// future points of the provided stack. But anything new could go into       
/// old future points, as long as state and filters allows it!                
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param stack - [in/out] the stack to link with                          
///   @return true if scope was linked successfully                           
bool Temporal::PushFutures(const Many& scope, Block<>& stack) {
   bool atLeastOneSuccess = false;

   if (stack.IsDeep()) {
      // Nest deep stack                                                
      stack.ForEachRev([&](Block<>& substack) {
         atLeastOneSuccess |= PushFutures(scope, substack);
         // Continue linking only if the stack is branched              
         return not (stack.IsOr() and atLeastOneSuccess);
      });

      return atLeastOneSuccess;
   }

   // Iterate backwards - the last future points are always most        
   // relevant for linking. Lets start, by scanning all future          
   // points in the available stack. Scope will be duplicated for       
   // each encountered branch                                           
   stack.ForEachRev(
      [&](Trait& substack) {
         atLeastOneSuccess |= PushFutures(scope, substack);
         // Continue linking only if the stack is branched              
         return not (stack.IsOr() and atLeastOneSuccess);
      },
      [&](Construct& substack) {
         atLeastOneSuccess |= PushFutures(scope, substack.GetDescriptor());
         // Continue linking only if the stack is branched              
         return not (stack.IsOr() and atLeastOneSuccess);
      },
      [&](Verb& substack) -> LoopControl {
         if (PushFutures(scope, substack.GetArgument())) {
            atLeastOneSuccess = true;
            // Continue linking only if the stack is branched           
            return not stack.IsOr();
         }

         if (PushFutures(scope, substack.GetSource())) {
            atLeastOneSuccess = true;
            // Continue linking only if the stack is branched           
            return not stack.IsOr();
         }

         return Loop::Continue;
      },
      [&](Inner::MissingFuture& future) {
         atLeastOneSuccess |= future.Push(scope);
         // Continue linking only if the stack is branched              
         return not (stack.IsOr() and atLeastOneSuccess);
      }
   );

   return atLeastOneSuccess;
}
 
/// Links the missing past points of the provided scope, with the missing     
/// future points of the provided Neat. But anything new could go into        
/// old future points, as long as state and filters allows it!                
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param stack - [in/out] the neat stack to link with                     
///   @return true if scope was linked successfully                           
bool Temporal::PushFutures(const Many& scope, Neat& stack) {
   bool atLeastOneSuccess = false;
   stack.ForEach([&](Block<>& substack) {
      atLeastOneSuccess |= PushFutures(scope, substack);
   });
   return atLeastOneSuccess;
}

/// Experimental                                                              
/// Experimental                                                              
/// Experimental                                                              
void Temporal::Link(const Many& scope) {
   if (scope.IsOr())
      TODO();

   if (scope.IsDeep()) {
      // Nest deep scope                                                
      scope.ForEach([&](const Block<>& sub) {
         Link(sub);
      });
   }
   else {
      // Handle shallow scope                                           
      scope.ForEach(
         [&](const Trait& t) {
            // Forward to all future points in the priority stack       
            TMany<Trait> local = t;
            LANGULUS_ASSERT(
               PushFutures(local, mPriorityStack),
               Flow, "Couldn't push to future"
            );
         },
         [&](const Construct& c) {
            // Forward to all future points in the priority stack       
            TMany<Construct> local = c;
            LANGULUS_ASSERT(
               PushFutures(local, mPriorityStack),
               Flow, "Couldn't push to future"
            );
         },
         [&](const Verb& v) {
            if (v.IsVerb<Verbs::Do>()) {
               // "Do" verbs act as context/mass/rate/time setters      
               // Don't push them, but use them to set environment for  
               // any sub-verbs                                         
               LinkRelative(v.GetArgument(), v);
            }
            else if (v.GetTime()) {
               // Verb is timed, forward it to the time stack           
               TMany<Verb> local = v;
               local[0].SetTime(0);

               auto found = mTimeStack.FindIt(v.GetTime());
               if (not found) {
                  mTimeStack.Insert(v.GetTime(), this);
                  found = mTimeStack.FindIt(v.GetTime());
               }

               found.mValue->LinkRelative(local, v);
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

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
            else {
               // Forward it to the priority stack                      
               TMany<Verb> local = v;
               LANGULUS_ASSERT(
                  PushFutures(local, mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
         }
      );
   }
}

/// Experimental                                                              
/// Experimental                                                              
/// Experimental                                                              
void Temporal::LinkRelative(const Many& scope, const Verb& override) {
   if (scope.IsOr())
      TODO();

   if (scope.IsDeep()) {
      // Nest deep scope                                                
      scope.ForEach([&](const Block<>& sub) {
         LinkRelative(sub, override);
      });
   }
   else {
      // Handle shallow scope                                           
      scope.ForEach(
         [&](const Trait& t) {
            TMany<Trait> local = t;

            // Forward to future point in appropriate stack,            
            // according to the override verb                           
            if (override.GetTime()) {
               // Trait is timed, forward it to the time stack          
               auto found = mTimeStack.FindIt(override.GetTime());
               if (not found) {
                  mTimeStack.Insert(override.GetTime(), this);
                  found = mTimeStack.FindIt(override.GetTime());
               }

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
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

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
            else {
               // Forward it to the priority stack                      
               LANGULUS_ASSERT(
                  PushFutures(local, mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
         },
         [&](const Construct& c) {
            TMany<Construct> local = c;

            // Forward to future point in appropriate stack,            
            // according to the override verb                           
            if (override.GetTime()) {
               // Trait is timed, forward it to the time stack          
               auto found = mTimeStack.FindIt(override.GetTime());
               if (not found) {
                  mTimeStack.Insert(override.GetTime(), this);
                  found = mTimeStack.FindIt(override.GetTime());
               }

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
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

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
            else {
               // Forward it to the priority stack                      
               LANGULUS_ASSERT(
                  PushFutures(local, mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
         },
         [&](const Verb& v) {
            // Multiply verb energy and merge contexts                  
            const Verb localOverride = v * override;

            if (v.IsVerb<Verbs::Do>()) {
               // "Do" verbs act as context/mass/rate/time setters      
               // Don't push them, but use them to set environment for  
               // any sub-verbs                                         
               LinkRelative(v.GetArgument(), localOverride);
            }
            else if (localOverride.GetTime()) {
               // Verb is timed, forward it to the time stack           
               const auto time = localOverride.GetTime();
               TMany<Verb> local = v;
               local[0].SetTime(0);

               auto found = mTimeStack.FindIt(time);
               if (not found) {
                  mTimeStack.Insert(time, this);
                  found = mTimeStack.FindIt(time);
               }

               found.mValue->LinkRelative(local, localOverride);
            }
            else if (localOverride.GetRate()) {
               // Verb is rated, forward it to the frequency stack      
               const auto rate = localOverride.GetRate();
               TMany<Verb> local = v;
               local[0].SetRate(0);
               if (not local[0].GetSource())
                  local[0].SetSource(localOverride.GetSource());

               auto found = mFrequencyStack.FindIt(rate);
               if (not found) {
                  mFrequencyStack.Insert(rate, this);
                  found = mFrequencyStack.FindIt(rate);
               }

               LANGULUS_ASSERT(
                  found.mValue->PushFutures(local, found.mValue->mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
            else {
               // Forward it to the priority stack                      
               // Collapse all verb charges at this point               
               TMany<Verb> local = v;
               local[0].SetMass(localOverride.GetMass());
               local[0].SetPriority(localOverride.GetPriority());
               if (not local[0].GetSource())
                  local[0].SetSource(localOverride.GetSource());

               LANGULUS_ASSERT(
                  PushFutures(local, mPriorityStack),
                  Flow, "Couldn't push to future"
               );
            }
         }
      );
   }
}
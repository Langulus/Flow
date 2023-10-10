///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Time.inl"
#include "Serial.inl"
#include "Code.inl"
#include "Resolvable.inl"
#include "inner/Missing.inl"
#include "inner/Fork.hpp"
#include "Temporal.hpp"

#define VERBOSE_TEMPORAL(...) \
   Logger::Verbose(*this, ": ", __VA_ARGS__)
#define VERBOSE_TEMPORAL_TAB(...) \
   const auto tab = Logger::Verbose(*this, ": ", __VA_ARGS__, Logger::Tabs{})


namespace Langulus::Flow
{

   /// Default constructor, add the initial missing future point              
   ///   @param environment - the initial flow environment                    
   Temporal::Temporal(const Any& environment) {
      mEnvironment = environment;
      mPriorityStack << Inner::MissingFuture {};
   }

   /// Construct as a sub-flow                                                
   ///   @attention assumes parent is a valid pointer                         
   ///   @param parent - the parent flow                                      
   ///   @param state - the flow state                                        
   Temporal::Temporal(Temporal* parent, const State& state)
      : mParent {parent}
      , mState {state} { }

   /// Serialize temporal as Code                                             
   Temporal::operator Code() const {
      return IdentityOf(this);
   }

   /// Serialize temporal as debug string                                     
   Temporal::operator Debug() const {
      return IdentityOf(this);
   }

   /// Reset progress for the priority stack                                  
   void Temporal::Reset() {
      mPreviousTime = mCurrentTime = {};
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
      return mFrequencyStack or mTimeStack or mPriorityStack;
   }

   /// Dump the contents of the flow to the log                               
   void Temporal::Dump() const {
      Logger::Verbose(mPriorityStack);
      Logger::Verbose(mTimeStack);
      Logger::Verbose(mFrequencyStack);
   }

   /// Advance the flow - moves time forward, executes stacks                 
   ///   @param dt - delta time                                               
   void Temporal::Update(Time dt) {
      if (not mCurrentTime) {
         // If we're at the beginning of time - prepare for execution   
         auto collapsed = Collapse(mPriorityStack);

         // Now execute the collapsed priority stack                    
         Any output;
         if (not Execute(collapsed, mEnvironment, output))
            LANGULUS_THROW(Flow, "Update failed");

         // Then, set the priority stack to the output, by wrapping it  
         // in a hight-priority Do verb with future attachment          
         // This guarantees, that a Push is possible after the Update   
         Any future; future.MakeFuture();
         mPriorityStack = Verbs::Do {future}
            .SetSource(Abandon(output))
            .SetPriority(8);

         VERBOSE_TEMPORAL(Logger::Purple,
            "Flow after execution ", mPriorityStack);
      }

      if (not dt) {
         // Avoid updating anything else, if no time had passed         
         return;
      }

      // Advance the global cycler for the flow                         
      mPreviousTime = mCurrentTime;
      mCurrentTime += dt;

      // Execute flows that occur periodically                          
      for (auto pair : mFrequencyStack) {
         pair.mValue->mDuration += dt;
         if (pair.mValue->mDuration >= pair.mKey) {
            // Time to execute the periodic flow                        
            pair.mValue->mPreviousTime = mPreviousTime;
            pair.mValue->mCurrentTime = mCurrentTime;
            pair.mValue->mDuration -= pair.mKey;

            // Update the flow                                          
            // It might have periodic flows inside                      
            pair.mValue->Update(pair.mKey);
         }
      }

      // Execute flows that occur after a given point in time           
      for (auto pair : mTimeStack) {
         if (mCurrentTime < mState.mStart + pair.mKey) {
            // The time stack is sorted, so no point in continuing      
            break;
         }

         // Update the time flow                                        
         // It might have periodic flows inside                         
         pair.mValue->Update(dt);
      }
   }

   /// Merge a flow                                                           
   ///   @param other - the flow to merge with this one                       
   void Temporal::Merge(const Temporal& other) {
      // Concatenate priority stacks                                    
      mPriorityStack += other.mPriorityStack;

      // Merge time stacks                                              
      for (auto pair : other.mTimeStack) {
         const auto found = mTimeStack.Find(pair.mKey);
         if (not found) {
            const State state {
               TimePoint {mState.mStart + pair.mKey},
               Time {mState.mTime + pair.mKey},
               mState.mPeriod
            };

            //TODO make this more elegant
            Ptr<Temporal> newt;
            newt.New(this, state);
            mTimeStack.Insert(pair.mKey, newt.Get());
         }

         mTimeStack[pair.mKey]->Merge(*pair.mValue);
      };

      // Merge periodic stacks                                          
      for (auto pair : other.mFrequencyStack) {
         const auto found = mFrequencyStack.Find(pair.mKey);
         if (not found) {
            const State state {
               mState.mStart,
               mState.mTime,
               pair.mKey
            };

            Ptr<Temporal> newt;
            newt.New(this, state);
            mFrequencyStack.Insert(pair.mKey, newt.Get());
         }

         mFrequencyStack[pair.mKey]->Merge(*pair.mValue);
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
   ///      inserted   in a point of lower priority. A verb of higher-or-equal
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
   bool Temporal::Push(Any scope) {
      VERBOSE_TEMPORAL_TAB("Pushing: ", scope);

      // Compile pushed scope to an intermediate format                 
      auto compiled = Compile(scope, Inner::NoPriority);
      VERBOSE_TEMPORAL("Compiled to: ", compiled);

      // Link new scope with the available stacks                       
      const bool done = Link(compiled, mPriorityStack);
      VERBOSE_TEMPORAL(Logger::Purple, "Flow state: ", mPriorityStack);
      return done;
   }

   /// This will omit any compile-time junk that remains in the provided      
   /// scope, so we can execute it conventionally                             
   ///   @param scope - the scope to collapse                                 
   ///   @return the collapsed scope                                          
   Any Temporal::Collapse(const Block& scope) {
      Any result;
      if (scope.IsOr())
         result.MakeOr();

      if (scope.IsDeep()) {
         // Nest deep scopes                                            
         scope.ForEach([&](const Block& subscope) {
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
            auto collapsed = Collapse(subscope.MakeMessy()); //TODO MakeMessy is slow, probably a specialized Collapse would be better
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

   /// Compiles a scope into an intermediate form, used by the flow           
   ///   @attention assumes argument is a valid scope                         
   ///   @param scope - the scope to compile                                  
   ///   @param priority - the priority to set for any missing point created  
   ///                     for the provided scope                             
   ///   @return the compiled scope                                           
   Any Temporal::Compile(const Block& scope, Real priority) {
      Any result;
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
         scope.ForEach([&](const Block& subscope) {
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
               Compile(subscope.MakeMessy(), priority), //TODO MakeMessy is slow, probably a specialized Compile would be better
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

      if (!done) {
         // Just propagate content                                      
         result = scope;
      }

      return Abandon(result);
   }

   /// Links the missing past points of the provided scope, with the missing  
   /// future point (or any nested future points inside).                     
   /// Anything could be pushed to provided future point as a fallback,       
   /// as long as state and filters allows it!                                
   ///   @attention assumes argument is a valid scope                         
   ///   @param scope - the scope to link                                     
   ///   @param future - [in/out] the future point to place inside            
   ///   @return true if scope was linked successfully                        
   bool Temporal::Link(const Any& scope, Inner::MissingFuture& future) const {
      // Attempt linking to the contents first                          
      if (Link(scope, future.mContent))
         return true;

      //                                                                
      // If reached, then future point is flat and boring, fallback by  
      // directly linking against it                                    
      VERBOSE_TEMPORAL_TAB("Linking to: ", future);
      return future.Push(scope, mEnvironment);
   }

   /// Links the missing past points of the provided scope, with the missing  
   /// future points of the provided stack. But anything new could go into    
   /// old future points, as long as state and filters allows it!             
   ///   @attention assumes argument is a valid scope                         
   ///   @param scope - the scope to link                                     
   ///   @param stack - [in/out] the stack to link with                       
   ///   @return true if scope was linked successfully                        
   bool Temporal::Link(const Any& scope, Block& stack) const {
      bool atLeastOneSuccess = false;

      if (stack.IsDeep()) {
         // Nest deep stack                                             
         stack.ForEachRev([&](Block& substack) {
            atLeastOneSuccess |= Link(scope, substack);
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
            atLeastOneSuccess |= Link(scope, substack);
            // Continue linking only if the stack is branched           
            return not (stack.IsOr() and atLeastOneSuccess);
         },
         [&](Construct& substack) {
            atLeastOneSuccess |= Link(scope, substack);
            // Continue linking only if the stack is branched           
            return not (stack.IsOr() and atLeastOneSuccess);
         },
         [&](Verb& substack) {
            if (Link(scope, substack.GetArgument())) {
               atLeastOneSuccess = true;
               // Continue linking only if the stack is branched        
               return not stack.IsOr();
            }
            if (Link(scope, substack.GetSource())) {
               atLeastOneSuccess = true;
               // Continue linking only if the stack is branched        
               return not stack.IsOr();
            }

            return Flow::Continue;
         },
         [&](Inner::MissingFuture& future) {
            atLeastOneSuccess |= Link(scope, future);
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
   bool Temporal::Link(const Any& scope, Neat& stack) const {
      bool atLeastOneSuccess = false;
      stack.ForEach([&](Block& substack) {
         atLeastOneSuccess |= Link(scope, substack);
      });

      return atLeastOneSuccess;
   }

} // namespace Langulus::Flow

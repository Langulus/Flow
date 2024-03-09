///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Code.hpp"
#include "Time.hpp"
#include <Anyness/TMap.hpp>


namespace Langulus::Flow
{
   namespace Inner
   {
      struct Missing;
      struct MissingFuture;
      struct MissingPast;
   }


   ///                                                                        
   ///   Temporal flow                                                        
   ///                                                                        
   ///   Gives temporality to anything, by providing a time gradient.         
   ///   Can be used to select time points and temporal context.              
   ///   It registers all executed verbs and automatically executes them on   
   /// update, if periodic or delayed. It has buckets for verbs that occur at 
   /// specific time and/or period. An analogy for a flow is a git repository 
   /// where you record all changes (executed actions). You can, therefore,   
   /// fork, branch, etc...                                                   
   ///   The flow can also act as a session serializer - you can use it to    
   /// record the sequence you used to make your game, play your game, or     
   /// truly anything you can imagine, that can be described by a sequence    
   /// of actions. Which is practically everything there is.                  
   ///   You can execute scripts with missing future/past elements in         
   /// them, which means that the temporal flow acts as a time-based linker,  
   /// that actively seeks the past and future inputs for suitable data to    
   /// complete your scripts at runtime.                                      
   ///                                                                        
   class Temporal final {
      LANGULUS_CONVERTS_TO(Code, Text);
      friend struct Inner::Missing;

   private:
      struct State {
         TimePoint mStart;
         Time mTime;
         Time mPeriod;
      };
      
      // A default execution context                                    
      Any mEnvironment;
      // A parent flow                                                  
      Temporal* mParent {};
      // Background charge                                              
      State mState;
      // Increments on each call to Update()                            
      TimePoint mPreviousTime;
      TimePoint mCurrentTime;
      // Accumulated flow duration                                      
      Time mDuration;
      // Priority stack, i.e. the order of things that happen NOW       
      Any mPriorityStack;
      // Verb temporal stack, i.e. things that happen at specific time  
      TOrderedMap<Time, Temporal*> mTimeStack;
      // Verb frequency stack, i.e. things that happen periodically     
      TUnorderedMap<Time, Temporal*> mFrequencyStack;

   protected:
      LANGULUS_API(FLOW) static Any Collapse(const Block&);
      LANGULUS_API(FLOW) static Any Collapse(const Neat&);

      LANGULUS_API(FLOW) static Any Compile(const Block&, Real priority);
      LANGULUS_API(FLOW) static Any Compile(const Neat&, Real priority);

      LANGULUS_API(FLOW) bool Link(const Any&, Block&) const;
      LANGULUS_API(FLOW) bool Link(const Any&, Neat&) const;
      LANGULUS_API(FLOW) bool Link(const Any&, Inner::MissingFuture&) const;

   public:
      Temporal(const Temporal&) = delete;
      Temporal& operator = (const Temporal&) = delete;

      LANGULUS_API(FLOW) Temporal();
      LANGULUS_API(FLOW) Temporal(Temporal*, const State&);
      //LANGULUS_API(FLOW) Temporal(const Any& environment = {});
      LANGULUS_API(FLOW) Temporal(Temporal&&) noexcept = default;

      LANGULUS_API(FLOW) Temporal& operator = (Temporal&&) noexcept = default;

      NOD() LANGULUS_API(FLOW) operator Code() const;
      NOD() LANGULUS_API(FLOW) operator Text() const;

      NOD() LANGULUS_API(FLOW)
      bool operator == (const Temporal&) const;

      NOD() LANGULUS_API(FLOW)
      bool IsValid() const;

      LANGULUS_API(FLOW) void Merge(const Temporal&);
      LANGULUS_API(FLOW) bool Push(Any);

      LANGULUS_API(FLOW) void Reset();
      LANGULUS_API(FLOW) bool Update(Time);

      LANGULUS_API(FLOW) void Dump() const;
   };

} // namespace Langulus::Flow


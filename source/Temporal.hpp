///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      // Parent flow                                                    
      Temporal* mParent {};
      // The time at which this flow started                            
      Time mStart;
      // The time at which current flow execution happens               
      Time mNow;

      // Period that corresponds to a unit of Charge::mTime             
      Time mTimePeriod = 1s;
      // Period that corresponds to a unit of Charge::mRate             
      Time mRatePeriod = 16ms;

      // Priority stack, i.e. hierarchy of events that happen once      
      Many mPriorityStack;
      // Verb temporal stack, i.e. events that happen at specific time  
      // Each unit of time is equal to one mTimePeriod                  
      TOrderedMap<Real, Temporal> mTimeStack;
      // Verb frequency stack, i.e. events that happen periodically     
      // Each unit of time is equal to one mRatePeriod                  
      TUnorderedMap<Real, Temporal> mFrequencyStack;

   protected:
      /*LANGULUS_API(FLOW) static Many Collapse(const Block<>&);
      LANGULUS_API(FLOW) static Many Collapse(const Neat&);*/

      LANGULUS_API(FLOW) static Many Compile(const Block<>&, Real priority);
      LANGULUS_API(FLOW) static Many Compile(const Neat&, Real priority);

      LANGULUS_API(FLOW) bool PushFutures(const Many&, Block<>&);
      LANGULUS_API(FLOW) bool PushFutures(const Many&, Neat&);
      //LANGULUS_API(FLOW) bool PushFutures(const Many&, Inner::MissingFuture&);

      LANGULUS_API(FLOW) void Link(const Many&);
      LANGULUS_API(FLOW) void LinkRelative(const Many&, const Verb&);

   public:
      //Temporal(const Temporal&) = delete;
      //Temporal& operator = (const Temporal&) = delete;

      LANGULUS_API(FLOW) Temporal();
      LANGULUS_API(FLOW) Temporal(Temporal*);
      LANGULUS_API(FLOW) Temporal(Temporal&&) noexcept = default;
      LANGULUS_API(FLOW) Temporal(const Temporal&) noexcept = default;

      LANGULUS_API(FLOW) Temporal& operator = (Temporal&&) noexcept = default;
      LANGULUS_API(FLOW) Temporal& operator = (const Temporal&) noexcept = default;

      NOD() LANGULUS_API(FLOW) operator Code() const;
      NOD() LANGULUS_API(FLOW) operator Text() const;

      NOD() LANGULUS_API(FLOW)
      bool operator == (const Temporal&) const;

      NOD() LANGULUS_API(FLOW)
      bool IsValid() const;

      NOD() LANGULUS_API(FLOW)
      Time GetUptime() const;

      LANGULUS_API(FLOW) void Merge(const Temporal&);
      LANGULUS_API(FLOW) bool Push(Many);

      template<CT::Data T1, CT::Data...TN> requires (sizeof...(TN) >= 1)
      bool Push(T1&& t1, TN&&...tn) {
         return Push(Forward<T1>(t1)) and (Push(Forward<TN>(tn)) and ...);
      }

      LANGULUS_API(FLOW) void Reset();
      LANGULUS_API(FLOW) bool Update(Time = {});
      LANGULUS_API(FLOW) void Dump() const;

   protected:
      void ResetInner(Many&);
   };

} // namespace Langulus::Flow

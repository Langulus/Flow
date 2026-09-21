///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Time.hpp"
#include <thread>


namespace Langulus
{
   ///                                                                        
   /// Manages the framerate by measuring delta-time and sleeping             
   ///                                                                        
   template<int FRAMES_PER_SECOND = 60>
   struct Framerate {
      static constexpr int FramesPerSecond = FRAMES_PER_SECOND;

   protected:
      using dsec    = ::std::chrono::duration<double>;
      using seconds = ::std::chrono::seconds;

      const Time mInvFpsLimit;
      TimePoint mBegin;
      TimePoint mEnd;
      TimePoint mPrevTime;
      Time      mDeltaTime;

   public:
      Framerate()
         : mInvFpsLimit {::std::chrono::round<StdClock::duration>(dsec {1. / FramesPerSecond})}
         , mBegin       {SteadyClock::Now()}
         , mEnd         {mBegin + mInvFpsLimit}
         , mPrevTime    {mBegin} {}

      /// Get the time between ticks                                          
      ///   @return the time period between ticks                             
      Time GetDeltaTime() {
         return mDeltaTime;
      }

      /// Call this from your main loop                                       
      ///   @attention this may make the current thread sleep!                
      void Tick() {
         const auto now = SteadyClock::Now();
         if (now <= mPrevTime)
            return;

         mDeltaTime = now - mPrevTime;
         mPrevTime = now;

         if (now < mEnd) {
            // We've finished early - sleep for the rest of the time    
            ::std::this_thread::sleep_until(mEnd);
         }

         mBegin = mEnd;
         mEnd = mBegin + mInvFpsLimit;
      }
   };
}
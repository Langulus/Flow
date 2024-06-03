///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"
#include <Anyness/Neat.hpp>
#include <chrono>
#include <thread>
#include <fmt/chrono.h>


namespace Langulus
{
   using namespace ::std::literals::chrono_literals;

   namespace A
   {

      /// An abstract clock                                                   
      struct Clock {
         LANGULUS(ABSTRACT) true;
      };

      /// An abstract time                                                    
      struct Time {
         LANGULUS(ABSTRACT) true;
      };

   } // namespace Langulus::A

   using StdClock = ::std::chrono::steady_clock;

   ///                                                                        
   ///   A time point                                                         
   ///                                                                        
   struct TimePoint : A::Time, StdClock::time_point {
      LANGULUS(ABSTRACT) false;
      LANGULUS(POD) true;
      LANGULUS_BASES(A::Time);

      using Base = time_point;
      using Base::time_point;

      constexpr TimePoint() noexcept;
      constexpr TimePoint(const time_point&) noexcept;

      constexpr explicit operator bool() const noexcept;
   };


   ///                                                                        
   ///   A time duration (difference between two time points)                 
   ///                                                                        
   struct Time : A::Time, StdClock::duration {
      LANGULUS(ABSTRACT) false;
      LANGULUS(POD) true;
      LANGULUS_BASES(A::Time);

      using Base = duration;
      using Base::duration;

      constexpr Time() noexcept
         : duration {zero()} {
         using Representation = typename Base::rep;
         static_assert(sizeof(Representation) == sizeof(Time),
            "Size mismatch");
      }

      constexpr Time(const duration& a) noexcept
         : duration {a} {}

      constexpr explicit operator bool() const noexcept;

      template<CT::BuiltinNumber T = Real>
      NOD() T Seconds() const noexcept;

      Time operator + (auto&& rhs) const {
         return ::std::chrono::duration_cast<Base>(
            static_cast<const duration&>(*this) + rhs);
      }

      Time operator * (auto&& rhs) const {
         return ::std::chrono::duration_cast<Base>(
            static_cast<const duration&>(*this) * rhs);
      }
   };


   ///                                                                        
   ///   A steady clock used to aquire TimePoint(s)                           
   ///                                                                        
   class SteadyClock : public A::Clock, private StdClock {
      LANGULUS_BASES(A::Clock);
      NOD() static TimePoint Now() noexcept;
   };

   namespace CT
   {

      template<class T>
      concept Time = SameAsOneOf<T, ::Langulus::TimePoint, ::Langulus::Time>;

   } // namespace Langulus::CT


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

} // namespace Langulus

namespace fmt
{

   ///                                                                        
   /// Extend FMT to be capable of logging Flow::Time                         
   ///                                                                        
   template<>
   struct formatter<Langulus::Time> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT> LANGULUS(INLINED)
      auto format(Langulus::Time const& element, CONTEXT& ctx) const {
         return fmt::format_to(ctx.out(), "{}",
            static_cast<const Langulus::Time::Base&>(element));
      }
   };

} // namespace fmt

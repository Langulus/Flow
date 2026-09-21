///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/CT/Number.hpp>
#include <chrono>
#include <fmt/chrono.h>


namespace Langulus
{
   using namespace ::std::literals::chrono_literals;
   using StdClock = ::std::chrono::steady_clock;


   ///                                                                        
   ///   A time point                                                         
   ///                                                                        
   struct TimePoint : StdClock::time_point {
      using CTTI_POD  = Yup;
      using CTTI_Time = Yup;

      using Base = time_point;
      using Base::time_point;

      constexpr TimePoint() noexcept;
      constexpr TimePoint(const time_point&) noexcept;

      constexpr explicit operator bool() const noexcept;
   };


   ///                                                                        
   ///   A time duration (difference between two time points)                 
   ///                                                                        
   struct Time : StdClock::duration {
      using CTTI_POD  = Yup;
      using CTTI_Time = Yup;

      using Base = duration;
      using Base::duration;

      /// Default constructor means zero duration                             
      constexpr Time() noexcept
         : Base {zero()} {
         using Representation = typename Base::rep;
         static_assert(sizeof(Representation) == sizeof(Time),
            "Size mismatch");
      }

      /// Constrcut by duration_cast to the contained type                    
      template<class T, class PERIOD>
      constexpr Time(const std::chrono::duration<T, PERIOD>& a) noexcept
         : Base {std::chrono::duration_cast<Base>(a)} {}

      /// Assign by duration_cast to the contained type                       
      template<class T, class PERIOD>
      Time& operator = (const std::chrono::duration<T, PERIOD>& rhs) {
         Base::operator = (std::chrono::duration_cast<Base>(rhs));
         return *this;
      }

      constexpr explicit operator bool() const noexcept;

      template<CT::Number T = Real>
      T Seconds() const noexcept;

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
   ///   A steady clock used to acquire TimePoint(s)                          
   ///                                                                        
   class SteadyClock : private StdClock {
      static TimePoint Now() noexcept;
   };
}

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
}

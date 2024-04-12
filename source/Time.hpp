///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"
#include <Anyness/Neat.hpp>
#include <chrono>
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


   ///                                                                        
   ///   A time point                                                         
   ///                                                                        
   struct TimePoint : A::Time, ::std::chrono::steady_clock::time_point {
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
   struct Time : A::Time, ::std::chrono::steady_clock::duration {
      LANGULUS(ABSTRACT) false;
      LANGULUS(POD) true;
      LANGULUS_BASES(A::Time);

      using Base = duration;
      using Base::duration;
      using Base::operator +;

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
   };


   ///                                                                        
   ///   A steady clock used to aquire TimePoint(s)                           
   ///                                                                        
   class SteadyClock : public A::Clock, private ::std::chrono::steady_clock {
      LANGULUS_BASES(A::Clock);
      NOD() static TimePoint Now() noexcept;
   };

   namespace CT
   {

      template<class T>
      concept Time = SameAsOneOf<T, ::Langulus::TimePoint, ::Langulus::Time>;

   } // namespace Langulus::CT

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

///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"
#include <chrono>

namespace Langulus::A
{
   struct Time {
      LANGULUS(ABSTRACT) true;
   };
}

namespace Langulus::Flow
{

   ///                                                                        
   ///   A steady clock used to aquire TimePoint(s)                           
   ///                                                                        
   using SteadyClock = ::std::chrono::steady_clock;


   ///                                                                        
   ///   A time point                                                         
   ///                                                                        
   struct TimePoint : SteadyClock::time_point {
      LANGULUS(POD) true;
      LANGULUS_BASES(A::Time);

      using Base = SteadyClock::time_point;
      using Base::time_point;

      constexpr TimePoint() noexcept
         : time_point {min()} {}

      constexpr TimePoint(const time_point& a) noexcept
         : time_point {a} {}

      constexpr explicit operator bool() const noexcept {
         return *this != min();
      }

      NOD() Hash GetHash() const noexcept {
         using Representation = typename Base::rep;
         static_assert(sizeof(Representation) == sizeof(TimePoint),
            "Size mismatch");
         return HashNumber(reinterpret_cast<const Representation&>(*this));
      }
   };


   ///                                                                        
   ///   A time duration (difference between two time points)                 
   ///                                                                        
   struct Time : SteadyClock::duration {
      LANGULUS(POD) true;
      LANGULUS_BASES(A::Time);

      using Base = SteadyClock::duration;
      using Base::duration;
      using Base::operator +;

      constexpr Time() noexcept
         : duration {zero()} {}

      constexpr Time(const duration& a) noexcept
         : duration {a} {}

      constexpr explicit operator bool() const noexcept {
         return *this != zero();
      }

      NOD() Hash GetHash() const noexcept {
         using Representation = typename Base::rep;
         static_assert(sizeof(Representation) == sizeof(Time),
            "Size mismatch");
         return HashNumber(reinterpret_cast<const Representation&>(*this));
      }

      template<CT::DenseBuiltinNumber T = Real>
      NOD() Real Seconds() const noexcept {
         auto& asBase = static_cast<const Base&>(*this);
         return std::chrono::duration<T, std::chrono::seconds> {asBase}.count();
      }
   };

} // namespace Langulus::Flow

namespace Langulus::CT
{

   template<class T>
   concept Time = SameAsOneOf<T, Flow::TimePoint, Flow::Time>;

} // namespace Langulus::CT
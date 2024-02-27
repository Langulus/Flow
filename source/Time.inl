///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Time.hpp"


namespace Langulus
{

   /// Default time point construction, defaults to the minimal possible value
   LANGULUS(INLINED)
   constexpr TimePoint::TimePoint() noexcept
      : time_point {min()} {
      using Representation = typename Base::rep;
      static_assert(sizeof(Representation) == sizeof(TimePoint),
         "Size mismatch");
   }

   /// Copy-construction                                                      
   ///   @param a - time point to copy                                        
   LANGULUS(INLINED)
   constexpr TimePoint::TimePoint(const time_point& a) noexcept
      : time_point {a} {}

   /// Check if time point is something different from the default value      
   ///   @return true if not default                                          
   LANGULUS(INLINED)
   constexpr TimePoint::operator bool() const noexcept {
      return *this != min();
   }

   /// Default time duration construction, defaults to zero                   
   /*LANGULUS(INLINED)
   constexpr Time::Time() noexcept
      : duration {zero()} {
      using Representation = typename Base::rep;
      static_assert(sizeof(Representation) == sizeof(Time),
         "Size mismatch");
   }*/

   /// Copy-construction                                                      
   ///   @param a - duration to copy                                          
   LANGULUS(INLINED)
   constexpr Time::Time(const duration& a) noexcept
      : duration {a} {}

   /// Descriptor-construction                                                
   ///   @param a - descriptor to scan for relevant data                      
   LANGULUS(INLINED)
   Time::Time(Describe&&)
      : duration {zero()} {
      TODO();
   }

   /// Check if time duration is anything but zero                            
   ///   @return true if not zero                                             
   LANGULUS(INLINED)
   constexpr Time::operator bool() const noexcept {
      return *this != zero();
   }

   /// Get time duration in seconds, represented by type T                    
   template<CT::DenseBuiltinNumber T>
   LANGULUS(INLINED)
   T Time::Seconds() const noexcept {
      auto& asBase = static_cast<const Base&>(*this);
      return std::chrono::duration<T>(asBase).count();
   }

   /// Get current time point                                                 
   ///   @return the time point                                               
   LANGULUS(INLINED)
   TimePoint SteadyClock::Now() noexcept {
      return steady_clock::now();
   }

} // namespace Langulus
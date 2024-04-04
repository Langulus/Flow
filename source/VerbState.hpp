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


namespace Langulus::Flow
{
   
   ///                                                                        
   ///   Verb state flags                                                     
   ///                                                                        
   struct VerbState {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;

      enum Enum : ::std::uint8_t {
         // Default verb state                                          
         // Default state is short-circuited multicast                  
         Default = 0,

         // When verb is long-circuited (as oposed to short-circuited), 
         // it will not cease executing on success, and be executed for 
         // each element in the context if multicasted. Used usually in 
         // interpretation, when you want to guarantee all elements are 
         // converted                                                   
         LongCircuited = 1,

         // When verb is monocast (as opposite to multicast), it will   
         // not iterate deep items, but be executed on the context once 
         // as a whole. Used extensively when executing at compile-time 
         Monocast = 2
      };

      using Type = TypeOf<Enum>;

      Type mState {Default};

   public:
      constexpr VerbState() noexcept = default;
      constexpr VerbState(const Type&) noexcept;

      explicit constexpr operator bool() const noexcept;
      constexpr bool operator == (const VerbState&) const noexcept = default;
      
      NOD() constexpr VerbState operator + (const VerbState&) const noexcept;
      NOD() constexpr VerbState operator - (const VerbState&) const noexcept;
      constexpr VerbState& operator += (const VerbState&) noexcept;
      constexpr VerbState& operator -= (const VerbState&) noexcept;
      
      NOD() constexpr bool operator & (const VerbState&) const noexcept;
      NOD() constexpr bool operator % (const VerbState&) const noexcept;
      
      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;
      
      constexpr void Reset() noexcept;
   };

} // namespace Langulus::Flow

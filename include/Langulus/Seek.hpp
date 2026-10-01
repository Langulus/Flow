///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <cstdint>


namespace Langulus::Flow
{
   ///                                                                        
   ///   Bits for seek functions                                              
   ///                                                                        
   enum class Seek : uint8_t {
      // Seek entities that are children of the context                 
      Below = 1,
      // Seek entities that are parents of the context                  
      Above = 2,
      // Seek objects in both directions - both parents and children    
      Duplex = Below | Above,
      // Include the current entity in the seek operation               
      Here = 4,
      // Seek everywhere                                                
      Everywhere = Duplex | Here,
      // Seek parents with this context included                        
      HereAndAbove = Above | Here,
      // Seek children with this context included                       
      HereAndBelow = Below | Here
   };

   constexpr bool operator & (const Seek& lhs, const Seek& rhs) {
      return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
   }
}

///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <RTTI/Meta.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_FLOW)
   #define LANGULUS_API_FLOW() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_FLOW() LANGULUS_IMPORT()
#endif

LANGULUS_EXCEPTION(Flow);
LANGULUS_EXCEPTION(Link);

/// Make the rest of the code aware, that Langulus::Flow has been included    
#define LANGULUS_LIBRARY_FLOW() 1


namespace Langulus::Flow
{

   using namespace Anyness;
   using RTTI::VMeta;
   using RTTI::TMeta;
   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::AMeta;


   ///                                                                        
   ///   Bits for seek functions                                              
   ///                                                                        
   enum class Seek : uint8_t {
      // Seek entities that are children of the context                 
      Below = 1,
      // Seek entities that are parents of the context                  
      Above = 2,
      // Seek objects in both directions - in parents and children      
      Duplex = Below | Above,
      // Include the current entity in the seek operation               
      Here = 4,
      // Seek everywhere                                                
      Everywhere = Duplex | Here,
      // Seek parents and this context included                         
      HereAndAbove = Above | Here,
      // Seek children and this context included                        
      HereAndBelow = Below | Here
   };

   constexpr bool operator & (const Seek& lhs, const Seek& rhs) {
      return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
   }

} // namespace Langulus::Flow


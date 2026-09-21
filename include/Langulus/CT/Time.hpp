///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Typenav.hpp>


namespace Langulus::CTTI
{
   /// Extends T by marking it as temporal. Examples:                         
   /// 1) template<> struct Time<YourType> {};                                
   /// 2) struct YourType { using CTTI_Time = Yup; };                         
   template<class T>
   struct Time;
}

LANGULUS_CTTI_CONCEPT_DECVQ(Time);

///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../Verb.hpp"


namespace Langulus::Flow::Inner
{

   ///                                                                        
   ///   A redundant data, that is used only as past, never executed          
   ///                                                                        
   struct Redundant {
      LANGULUS_CONVERTS_TO(Text);

      Many mContent;

      operator Text() const {
         return "";
      }
   };

} // namespace Langulus::Flow::Inner
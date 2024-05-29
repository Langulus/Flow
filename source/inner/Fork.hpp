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
   ///   A fork - place where the flow separates in several branches          
   ///                                                                        
   struct Fork {
      TMany<DMeta> mFilter;
      Many mBranches;
   };

} // namespace Langulus::Flow::Inner

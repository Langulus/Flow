///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
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

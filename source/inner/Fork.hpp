///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
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
      TAny<DMeta> mFilter;
      Any mBranches;
   };

} // namespace Langulus::Flow::Inner

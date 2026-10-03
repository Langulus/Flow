///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>


///                                                                           
/// MARK: Equality test                                                       
/// Compares for equality, returns source if equal to argument.               
///                                                                           
LANGULUS_DEFINE_OPERATOR(Equal, Different, " == ", " != ", 3,
   "Compares for equality, returns source if equal to argument"
);
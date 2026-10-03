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
/// MARK: Greater test                                                        
/// Compares for greatness, returns source if greater than argument.          
///                                                                           
LANGULUS_DEFINE_OPERATOR(Greater, LowerOrEqual, " > ", " <= ", 3,
   "Compares for source being greater than argument, "
   "and returns source if so"
);
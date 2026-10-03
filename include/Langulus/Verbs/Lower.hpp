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
/// MARK: Lower test                                                          
/// Compares for lower, returns source if lower than argument.                
///                                                                           
LANGULUS_DEFINE_OPERATOR(Lower, GreaterOrEqual, " < ", " >= ", 3,
   "Compares for source being lower than argument, "
   "and returns source if so"
);
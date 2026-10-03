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
/// MARK: Three-way comparison                                                
/// General purpose three-way comparison, that checks for equality, lesser,   
/// and greater at the same time.                                             
///                                                                           
LANGULUS_DEFINE_OPERATOR(Compare, Compare, " <=> ", " <=> ", 3,
   "General purpose three-way comparison, that checks for "
   "equality, lesser, and greater at the same time"
);
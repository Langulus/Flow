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
/// MARK: Conjunct/Disjunct                                                   
/// Either combines LHS and RHS as one AND container, or separates them       
/// as one OR container. Does only shallow copying.                           
///                                                                           
LANGULUS_DEFINE_OPERATOR(Conjunct, Disjunct, ", ", " or ", 1,
   "Either combines LHS and RHS as one AND container, or separates them "
   "as one OR container (does only shallow copying)"
);
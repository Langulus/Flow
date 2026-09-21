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
/// MARK: Associate/Disassociate verb                                         
///   Either performs a shallow copy, or excites/inhibits associations,       
/// depending on the context's complexity                                     
///                                                                           
LANGULUS_DEFINE_OPERATOR(Associate, Disassociate, " = ", " ~ ", 2,
   "Either performs a shallow copy, or aggregates associations, "
   "depending on the context's complexity"
);
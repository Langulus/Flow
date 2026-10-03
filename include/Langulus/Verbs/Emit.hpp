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
/// MARK: Emit/Absorb                                                         
/// Used for emitting and absorbing events. Can be used for input events, or  
/// any other abstract communication mechanism, as well as physical events    
/// between entities, depending on context.                                   
///                                                                           
LANGULUS_DEFINE_VERB(Emit, Absorb, 0,
   "Used for emitting and absorbing events. Can be used for input events, or "
   "any other abstract communication mechanism, as well as physical events "
   "between entities, depending on context"
);
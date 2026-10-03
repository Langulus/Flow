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
/// MARK: Create/Destroy verb                                                 
/// Used for allocating new elements. If the type you're creating has         
/// a producer, you need to execute the verb in the appropriate context.      
///                                                                           
LANGULUS_DEFINE_VERB(Create, Destroy, 1000,
   "Used for allocating new elements of any kind. "
   "If the type you're creating has a producer, "
   "you need to have that producer in the current context. "
   "That producer will be created automatically for you, "
   "if context allows for it"
);
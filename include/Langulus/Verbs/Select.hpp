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
/// MARK: Select/Deselect verb                                                
///   Used to focus on a part of a context, or access members.                
/// Narrows or broadens a context.                                            
///                                                                           
LANGULUS_DEFINE_OPERATOR(Select, Deselect, ".", "..", 100,
   "Used to focus on a part of a context, or access members. "
   "Narrows or broadens a context."
);
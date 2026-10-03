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
/// MARK: Catenate/Split                                                      
/// Catenates anything catenable, or splits stuff apart using a mask          
///                                                                           
LANGULUS_DEFINE_OPERATOR(Catenate, Split, " >< ", " <> ", 7,
   "Concatenates, or splits stuff apart"
);
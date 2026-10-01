///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Core.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_FLOW)
   #define LANGULUS_API_FLOW() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_FLOW() LANGULUS_IMPORT()
#endif

/// Make the rest of the code aware, that Langulus::Flow has been included    
#define LANGULUS_LIBRARY_FLOW() 1
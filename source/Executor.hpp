///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"


namespace Langulus::Flow
{

   ///                                                                        
   /// Tools for executing containers as flows                                
   ///                                                                        
   //LANGULUS_API(FLOW)
   //bool Execute(const Many&, Many&, bool silent = false);
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&, Many& output, bool silent = false);
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&, Many& output, bool& skipVerbs, bool silent = false);

   LANGULUS_API(FLOW)
   bool ExecuteAND(const Many&, Many&, Many& output, bool& skipVerbs, bool silent = false);
   LANGULUS_API(FLOW)
   bool ExecuteOR(const Many&, Many&, Many& output, bool& skipVerbs, bool silent = false);

   LANGULUS_API(FLOW)
   bool ExecuteVerb(Many&, Verb&, bool silent = false);
   LANGULUS_API(FLOW)
   bool IntegrateVerb(Many&, Verb&, bool silent = false);

} // namespace Langulus::Flow

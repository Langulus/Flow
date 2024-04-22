///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"


namespace Langulus::Flow
{

   ///                                                                        
   /// Tools for executing containers as flows                                
   ///                                                                        
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&);
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&, Many& output);
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&, Many& output, bool& skipVerbs);

   LANGULUS_API(FLOW)
   bool ExecuteAND(const Many&, Many&, Many& output, bool& skipVerbs);
   LANGULUS_API(FLOW)
   bool ExecuteOR(const Many&, Many&, Many& output, bool& skipVerbs);

   LANGULUS_API(FLOW)
   bool ExecuteVerb(Many&, Verb&);
   LANGULUS_API(FLOW)
   bool IntegrateVerb(Many&, Verb&);

} // namespace Langulus::Flow

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
   bool Execute(const Block&, Any&);
   LANGULUS_API(FLOW)
   bool Execute(const Block&, Any&, Any& output);
   LANGULUS_API(FLOW)
   bool Execute(const Block&, Any&, Any& output, bool& skipVerbs);

   LANGULUS_API(FLOW)
   bool ExecuteAND(const Block&, Any&, Any& output, bool& skipVerbs);
   LANGULUS_API(FLOW)
   bool ExecuteOR(const Block&, Any&, Any& output, bool& skipVerbs);

   LANGULUS_API(FLOW)
   bool ExecuteVerb(Any&, Verb&);
   LANGULUS_API(FLOW)
   bool IntegrateVerb(Any&, Verb&);

   NOD() LANGULUS_API(FLOW)
   bool IsExecutable(const Block&) noexcept;
   NOD() LANGULUS_API(FLOW)
   bool IsExecutableDeep(const Block&) noexcept;

} // namespace Langulus::Flow

///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"

namespace Langulus::Verbs
{
   struct Associate;
}

namespace Langulus::Flow
{

   ///                                                                        
   /// A scope is simply an Any container (binary compatible, too)            
   /// It has some additional interface for executing flows                   
   ///                                                                        
   struct Scope : Any {
      using Any::Any;
      using Any::operator ==;

      NOD() LANGULUS_API(FLOW)
      bool IsExecutable() const noexcept;
      NOD() LANGULUS_API(FLOW)
      bool IsExecutableDeep() const noexcept;

      LANGULUS_API(FLOW)
      bool Execute(Any&) const;
      LANGULUS_API(FLOW)
      bool Execute(Any&, Any& output) const;
      LANGULUS_API(FLOW)
      bool Execute(Any&, Any& output, bool& skipVerbs) const;
      LANGULUS_API(FLOW)
      bool ExecuteAND(Any&, Any& output, bool& skipVerbs) const;
      LANGULUS_API(FLOW)
      bool ExecuteOR(Any&, Any& output, bool& skipVerbs) const;

      LANGULUS_API(FLOW)
      static bool ExecuteVerb(Any&, Verb&);
      LANGULUS_API(FLOW)
      static bool IntegrateVerb(Any&, Verb&);

   protected:
      friend struct Langulus::Verbs::Associate;
      using Block::CallUnknownSemanticAssignment;
   };

} // namespace Langulus::Flow

///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"

namespace Langulus::Flow
{

   ///                                                                        
   /// A scope is simply an Any container (binary compatible, too)            
   /// It has some additional interface for executing flows                   
   ///                                                                        
   class Scope : public Any {
   public:
      using Any::Any;
      using Any::operator ==;

      //NOD() Scope Clone() const;

      NOD() bool IsExecutable() const noexcept;
      NOD() bool IsExecutableDeep() const noexcept;

      bool Execute(Any&) const;
      bool Execute(Any&, Any& output) const;
      bool Execute(Any&, Any& output, bool& skipVerbs) const;
      bool ExecuteAND(Any&, Any& output, bool& skipVerbs) const;
      bool ExecuteOR(Any&, Any& output, bool& skipVerbs) const;

      static bool ExecuteVerb(Any&, Verb&);
      static bool IntegrateVerb(Any&, Verb&);
   };

} // namespace Langulus::Flow

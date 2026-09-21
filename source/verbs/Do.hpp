///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>
#include "Langulus/CT/Executable.hpp"
#include "Langulus/CT/Deep.hpp"


///                                                                           
/// MARK: Do/Undo                                                             
///   Serves as a level of indirection between context and verb.              
/// When added as an ability to a thing, that thing can now route all         
/// verbs through itself, before executing them (or not). It basically        
/// states that the entity has the ability to "do things differently".        
/// Useful for implementing dispatchers, debuggers, interpreters of entire    
/// flows, etc.                                                               
///   For example, in the game Mindmaze, the maze is essentially              
/// an interpreter that converts a script into a maze. Different verbs        
/// do completely different things in that context - they attach hallways,    
/// rooms, place stuff in the rooms, etc. All events that happen to a         
/// maze object go through the maze's Do ability and get interpreted          
/// accordingly to the local rules.                                           
///   @attention this verb goes through all branches without doing            
///      any short-circuiting.                                                
LANGULUS_DEFINE_VERB(Do, Undo, "An indirection between context and flow. Not short-circuited.");

/*namespace Langulus::Verbs
{
   struct Do : Annies::TVerb<Do> {
      using CTTI_DefineVerb = NamedVerb<"Do", "Undo">;
      using CTTI_Info       = Yes<"An indirection between context and flow. Not short-circuited.">;

      using TVerb::TVerb;*/
      //using TVerb::operator ==;

      /*template<CT::Dense, CT::NotVoid...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::NotVoid...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);*/
      /*static bool ExecuteDefault(Many const&, Verb&);
      static bool ExecuteDefault(Many&, Verb&);
      static bool ExecuteStateless(Verb&);

      static Do In(auto&&, auto&&);
   };
}*/

namespace Langulus::Flow
{
   static bool GenericExecuteIn(CT::Dense auto&, CT::Executable auto&);
   static bool GenericExecuteDefault(Many const&, CT::Executable auto&);
   static bool GenericExecuteDefault(Many&, CT::Executable auto&);
   static bool GenericExecuteStateless(CT::Executable auto&);

   template<bool DISPATCH, bool DEFAULT, bool FALLBACK>
   size_t Execute(CT::NotVoid auto&, CT::Executable auto&);

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true>
   size_t DispatchFlat(CT::Deep auto&, CT::Executable auto&);

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true>
   size_t DispatchDeep(CT::Deep auto&, CT::Executable auto&);
}

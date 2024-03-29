///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Verb.hpp"


namespace Langulus::Verbs
{
   using namespace Flow;

   ///                                                                        
   ///   Do/Undo verb                                                         
   /// Used as a runtime dispatcher of composite types                        
   ///                                                                        
   struct Do : StaticVerb<Do> {
      LANGULUS(POSITIVE_VERB) "Do";
      LANGULUS(NEGATIVE_VERB) "Undo";
      LANGULUS(INFO) "Used as a runtime dispatcher of composite types";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);
      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };
}

namespace Langulus::Flow
{

   template<bool DISPATCH, bool DEFAULT, bool FALLBACK>
   Count Execute(CT::Data auto& context, CT::VerbBased auto& verb);

   template<bool DISPATCH, bool DEFAULT, bool FALLBACK, class... BASES>
   Count ExecuteInBases(CT::Data auto& context, CT::VerbBased auto& verb, Types<BASES...>);

   namespace Inner
   {
      template<bool DISPATCH, bool DEFAULT, bool FALLBACK, class BASE>
      Count ExecuteInBases(CT::Data auto& context, CT::VerbBased auto& verb);
   }

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true>
   Count DispatchFlat(CT::Deep auto& context, CT::VerbBased auto& verb);

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true>
   Count DispatchDeep(CT::Deep auto& context, CT::VerbBased auto& verb);

}
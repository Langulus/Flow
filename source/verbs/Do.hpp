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

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };
}

namespace Langulus::Flow
{

   template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::VerbBased V>
   Count Execute(T& context, V& verb);

   template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::VerbBased V, CT::Data... BASES>
   Count ExecuteInBases(T& context, V& verb, TTypeList<BASES...>);

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true, CT::Deep T, CT::VerbBased V>
   Count DispatchFlat(T& context, V& verb);

   template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true, CT::Deep T, CT::VerbBased V>
   Count DispatchDeep(T& context, V& verb);

}
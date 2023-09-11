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

#include "../../../source/verbs/Do.inl"
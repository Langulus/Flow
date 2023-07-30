///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
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
   ///   Compare verb                                                         
   /// Used to compare for equality, or for largeness/smallness               
   ///                                                                        
   struct Compare : StaticVerb<Compare> {
      LANGULUS(VERB) "Compare";
      LANGULUS(OPERATOR) " == ";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO) "Used to compare for equality, or largeness/smallness";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
   };
}

#include "../../../source/verbs/Compare.inl"
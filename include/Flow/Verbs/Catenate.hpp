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

   /// Catenate/Split verb                                                    
   /// Catenates anything catenable, or split stuff apart using a mask        
   struct Catenate : StaticVerb<Catenate> {
      LANGULUS(POSITIVE_VERB) "Catenate";
      LANGULUS(NEGATIVE_VERB) "Split";
      LANGULUS(POSITIVE_OPERATOR) " >< ";
      LANGULUS(NEGATIVE_OPERATOR) " <> ";
      LANGULUS(PRECEDENCE) 7;
      LANGULUS(INFO) "Catenates, or splits stuff apart";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      LANGULUS_API(FLOW) static bool ExecuteDefault(const Block&, Verb&);
      LANGULUS_API(FLOW) static bool ExecuteDefault(Block&, Verb&);
      LANGULUS_API(FLOW) static bool ExecuteStateless(Verb&);
   };
}

#include "../../../source/verbs/Catenate.inl"
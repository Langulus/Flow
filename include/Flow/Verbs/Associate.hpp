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

   /// Associate/Disassociate verb                                            
   /// Either performs a shallow copy, or aggregates associations,            
   /// depending on the context's complexity                                  
   struct Associate : StaticVerb<Associate> {
      LANGULUS(POSITIVE_VERB) "Associate";
      LANGULUS(NEGATIVE_VERB) "Disassocate";
      LANGULUS(POSITIVE_OPERATOR) " = ";
      LANGULUS(NEGATIVE_OPERATOR) " ~ ";
      LANGULUS(PRECEDENCE) 2;
      LANGULUS(INFO)
         "Either performs a shallow copy, or aggregates associations, "
         "depending on the context's complexity";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      LANGULUS_API(FLOW) static bool ExecuteDefault(Block&, Verb&);
   };
}

#include "../../../source/verbs/Associate.inl"
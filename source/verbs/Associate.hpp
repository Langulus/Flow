///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../TVerb.hpp"


namespace Langulus::Verbs
{

   using namespace Flow;


   ///                                                                        
   ///   Associate/Disassociate verb                                          
   /// Either performs a shallow copy, or excites/inhibits associations,      
   /// depending on the context's complexity                                  
   ///                                                                        
   struct Associate : TVerb<Associate> {
      LANGULUS(POSITIVE_VERB) "Associate";
      LANGULUS(NEGATIVE_VERB) "Disassociate";
      LANGULUS(POSITIVE_OPERATOR) " = ";
      LANGULUS(NEGATIVE_OPERATOR) " ~ ";
      LANGULUS(PRECEDENCE) 2;
      LANGULUS(INFO)
         "Either performs a shallow copy, or aggregates associations, "
         "depending on the context's complexity";

      using TVerb::TVerb;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(Block&, Verb&);
   };

} // namespace Langulus::Verbs

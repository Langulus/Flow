///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../TVerb.hpp"


namespace Langulus::Verbs
{

   using namespace Flow;


   ///                                                                        
   ///   Equals verb                                                          
   /// Compares for equality, returns source if equal to argument             
   ///                                                                        
   struct Equal : TVerb<Equal> {
      LANGULUS(VERB) "Equal";
      LANGULUS(OPERATOR) " == ";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO)
         "Compares for equality, returns source if equal to argument";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Many&, Verb&);
   };

} // namespace Langulus::Verbs

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
   ///   Greater verb                                                         
   /// Tests if source is greater than argument, and returns source if so     
   ///                                                                        
   struct Greater : TVerb<Greater> {
      LANGULUS(VERB) "Greater";
      LANGULUS(OPERATOR) " > ";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO)
         "Compares for source being greater than argument, "
         "and returns source if so";

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

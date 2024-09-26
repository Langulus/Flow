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
   ///   Lower verb                                                           
   /// Compares for source being less than argument, and returns source if so 
   ///                                                                        
   struct Lower : TVerb<Lower> {
      LANGULUS(VERB) "Lower";
      LANGULUS(OPERATOR) " < ";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO)
         "Compares for source being less than argument, "
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
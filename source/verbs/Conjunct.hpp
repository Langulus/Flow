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
   ///   Conjunct/Disjunct verb                                               
   /// Either combines LHS and RHS as one AND container, or separates them    
   /// as one OR container - does only shallow copying                        
   ///                                                                        
   struct Conjunct : TVerb<Conjunct> {
      LANGULUS(POSITIVE_VERB) "Conjunct";
      LANGULUS(NEGATIVE_VERB) "Disjunct";
      LANGULUS(POSITIVE_OPERATOR) ", ";
      LANGULUS(NEGATIVE_OPERATOR) " or ";
      LANGULUS(PRECEDENCE) 1;
      LANGULUS(INFO)
         "Either combines LHS and RHS as one AND container, or separates them "
         "as one OR container (does only shallow copying)";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Many&, Verb&);
      static bool ExecuteStateless(Verb&);
   };

} // namespace Langulus::Verbs

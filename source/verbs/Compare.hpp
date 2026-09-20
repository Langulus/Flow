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
   ///   Compare verb                                                         
   /// General purpose three-way comparison, that checks for equality, lesser 
   /// and greater at the same time.                                          
   ///                                                                        
   struct Compare : TVerb<Compare> {
      LANGULUS(VERB) "Compare";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO)
         "General purpose three-way comparison, that checks for "
         "equality, lesser and greater at the same time";

      using TVerb::TVerb;
      using TVerb::operator ==;

      /*template<CT::Dense, CT::NotVoid...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::NotVoid...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);*/

      static bool ExecuteDefault(const Many&, Verb&);
   };

} // namespace Langulus::Verbs

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
   ///   Compare verb                                                         
   /// Used to compare for equality, or for largeness/smallness               
   ///                                                                        
   struct Compare : TVerb<Compare> {
      LANGULUS(VERB) "Compare";
      LANGULUS(OPERATOR) " == ";
      LANGULUS(PRECEDENCE) 3;
      LANGULUS(INFO) "Used to compare for equality, or largeness/smallness";

      using TVerb::TVerb;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
   };

} // namespace Langulus::Verbs

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
   ///   Catenate/Split verb                                                  
   /// Catenates anything catenable, or split stuff apart using a mask        
   ///                                                                        
   struct Catenate : TVerb<Catenate> {
      LANGULUS(POSITIVE_VERB) "Catenate";
      LANGULUS(NEGATIVE_VERB) "Split";
      LANGULUS(POSITIVE_OPERATOR) " >< ";
      LANGULUS(NEGATIVE_OPERATOR) " <> ";
      LANGULUS(PRECEDENCE) 7;
      LANGULUS(INFO) "Catenates, or splits stuff apart";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };

} // namespace Langulus::Verbs

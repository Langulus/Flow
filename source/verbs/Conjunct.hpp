///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Verb.hpp"


namespace Langulus::Verbs
{
   using namespace Flow;

   ///                                                                        
   ///   Conjunct/Disjunct verb                                               
   /// Either combines LHS and RHS as one AND container, or separates them    
   /// as one OR container - does only shallow copying                        
   ///                                                                        
   struct Conjunct : StaticVerb<Conjunct> {
      LANGULUS(POSITIVE_VERB) "Conjunct";
      LANGULUS(NEGATIVE_VERB) "Disjunct";
      LANGULUS(POSITIVE_OPERATOR) ", ";
      LANGULUS(NEGATIVE_OPERATOR) " or ";
      LANGULUS(PRECEDENCE) 1;
      LANGULUS(INFO)
         "Either combines LHS and RHS as one AND container, or separates them "
         "as one OR container (does only shallow copying)";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };
}
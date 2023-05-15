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

   /// Exponent/Root verb                                                     
   /// Performs exponentiation or root                                        
   struct Exponent : ArithmeticVerb<Exponent, true> {
      LANGULUS(POSITIVE_VERB) "Exponent";
      LANGULUS(NEGATIVE_VERB) "Root";
      LANGULUS(POSITIVE_OPERATOR) "^";
      LANGULUS(NEGATIVE_OPERATOR) "^^";
      LANGULUS(PRECEDENCE) 6;
      LANGULUS(INFO) "Performs exponentiation or root";

      using ArithmeticVerb::ArithmeticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);

      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, const Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, Block&, Verb&);
   };
}

#include "../../../source/verbs/Exponent.inl"
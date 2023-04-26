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

   /// Interact                                                               
   /// Used for processing user events, such as mouse movement, keyboard,     
   /// joystick and any other input                                           
   struct Interact : StaticVerb<Interact> {
      LANGULUS(VERB) "Interact";
      LANGULUS(INFO) 
         "Used for processing user events, such as mouse movement, "
         "keyboard, joystick and any other input";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);
   };

}

#include "../../../source/verbs/Interpret.inl"
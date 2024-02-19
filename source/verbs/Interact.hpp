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
   ///   Interact                                                             
   /// Used for processing user events, such as mouse movement, keyboard,     
   /// joystick and any other input                                           
   ///                                                                        
   struct Interact : TVerb<Interact> {
      LANGULUS(VERB) "Interact";
      LANGULUS(INFO) 
         "Used for processing user events, such as mouse movement, "
         "keyboard, joystick and any other input";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);
   };

} // namespace Langulus::Verbs

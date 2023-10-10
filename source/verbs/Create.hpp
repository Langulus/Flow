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
   ///   Create/Destroy verb                                                  
   /// Used for allocating new elements. If the type you're creating has      
   /// a producer, you need to execute the verb in the correct context        
   ///                                                                        
   struct Create : StaticVerb<Create> {
      LANGULUS(POSITIVE_VERB) "Create";
      LANGULUS(NEGATIVE_VERB) "Destroy";
      LANGULUS(PRECEDENCE) 1000;
      LANGULUS(INFO)
         "Used for allocating new elements. "
         "If the type you're creating has   a producer, "
         "you need to execute the verb in a matching producer, "
         "or that producer will be created automatically for you, if possible";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      static void SetMembers(Any&, const Any&);
   };
}
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
   ///   Create/Destroy verb                                                  
   /// Used for allocating new elements. If the type you're creating has      
   /// a producer, you need to execute the verb in the correct context        
   ///                                                                        
   struct Create : TVerb<Create> {
      LANGULUS(POSITIVE_VERB) "Create";
      LANGULUS(NEGATIVE_VERB) "Destroy";
      LANGULUS(PRECEDENCE) 1000;
      LANGULUS(INFO)
         "Used for allocating new elements. "
         "If the type you're creating has   a producer, "
         "you need to execute the verb in a matching producer, "
         "or that producer will be created automatically for you, if possible";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(Many&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      static void SetMembers(Many&, const Many&);
   };

} // namespace Langulus::Verbs

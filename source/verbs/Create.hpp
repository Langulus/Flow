///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>


///                                                                           
/// MARK: Create/Destroy verb                                                 
///   Used for allocating new elements. If the type you're creating has       
/// a producer, you need to execute the verb in the correct context           
///                                                                           
LANGULUS_DEFINE_VERB(Create, Destroy,
   "Used for allocating new elements of any kind. "
   "If the type you're creating has a producer, "
   "you need to have that producer in the current context. "
   "That producer will be created automatically for you, "
   "if context allows for it"
);

/*namespace Langulus::Verbs
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
         ;

      using TVerb::TVerb;
      using TVerb::operator ==;*/

      /*template<CT::Dense, CT::NotVoid...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::NotVoid...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);*/

      /*static bool ExecuteDefault(Many&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      static void SetMembers(Many&, const Many&);
   };

}*/ // namespace Langulus::Verbs

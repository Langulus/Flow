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
   ///   Interpret                                                            
   /// Performs conversion                                                    
   ///                                                                        
   struct Interpret : TVerb<Interpret> {
      LANGULUS(VERB) "Interpret";
      LANGULUS(OPERATOR) " => ";
      LANGULUS(INFO) "Performs conversion";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      template<CT::Decayed TO, CT::Decayed FROM>
      static TO To(const FROM&);

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
   };
   

   ///                                                                        
   ///   Statically optimized interpret verb                                  
   ///   @tparam AS - what are we converting to?                              
   ///                                                                        
   template<CT::Data AS>
   struct InterpretAs : Interpret {
      LANGULUS_BASES(Interpret);
      using Interpret::Interpret;
      using Type = AS;

      InterpretAs();
      static bool ExecuteDefault(const Block&, Verb&);
   };

} // namespace Langulus::Verbs
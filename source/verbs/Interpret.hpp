///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>


namespace Langulus::Verbs
{
   struct Interpret;
}

namespace Langulus::CTTI
{
   /// External definition required prior to defining Verbs::Interpret        
   /// in order to avoid incompleteness.                                      
   template<>
   struct DefineVerb<Verbs::Interpret> : NamedVerb<"Interpret"> {};
}

namespace Langulus::Verbs
{
   ///                                                                        
   ///   Interpret                                                            
   /// Performs conversion                                                    
   ///                                                                        
   struct Interpret : Annies::TVerb<Interpret> {
      using CTTI_DefineVerbOp = NamedOperator<" => ">;
      using CTTI_Info         = Yes<"Performs conversion">;

      using TVerb::TVerb;
      using TVerb::operator ==;

      /*template<CT::Dense, CT::NotVoid...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::NotVoid...>
      static constexpr auto Of() noexcept;*/

      template<CT::Decayed TO, CT::Decayed FROM> requires (not Same<TO, FROM>)
      static TO To(const FROM&);

      //static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Many&, Verb&);
   };
}
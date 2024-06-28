///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../TVerb.hpp"
#include <Anyness/Trait.hpp>


namespace Langulus::Verbs
{

   using namespace Flow;


   ///                                                                        
   ///   Select/Deselect verb                                                 
   /// Used to focus on a part of a container, or access members              
   ///                                                                        
   struct Select : TVerb<Select> {
      LANGULUS(POSITIVE_VERB) "Select";
      LANGULUS(NEGATIVE_VERB) "Deselect";
      LANGULUS(POSITIVE_OPERATOR) ".";
      LANGULUS(NEGATIVE_OPERATOR) "..";
      LANGULUS(PRECEDENCE) 100;
      LANGULUS(INFO)
         "Used to focus on a part of a container, or access members";

      using TVerb::TVerb;
      using TVerb::operator ==;

      template<CT::Dense, CT::Data...>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense, CT::Data...>
      static constexpr auto Of() noexcept;

      static bool ExecuteIn(CT::Dense auto&, Verb&);

      static bool ExecuteDefault(const Many&, Verb&);
      static bool ExecuteDefault(Many&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      template<bool MUTABLE>
      static bool DefaultSelect(Many&, Verb&);
      template<bool MUTABLE>
      static bool PerIndex(Many&, TMany<Trait>&, TMeta, CT::Meta auto, const TMany<Index>&);
      template<bool MUTABLE>
      static bool SelectByMeta(const TMany<Index>&, DMeta, Many&, TMany<Trait>&, TMany<const RTTI::Ability*>&);
   };

} // namespace Langulus::Verbs

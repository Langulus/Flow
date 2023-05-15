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

   /// Select/Deselect verb                                                   
   /// Used to focus on a part of a container, or access members              
   struct Select : StaticVerb<Select> {
      LANGULUS(POSITIVE_VERB) "Select";
      LANGULUS(NEGATIVE_VERB) "Deselect";
      LANGULUS(POSITIVE_OPERATOR) ".";
      LANGULUS(NEGATIVE_OPERATOR) "..";
      LANGULUS(PRECEDENCE) 100;
      LANGULUS(INFO)
         "Used to focus on a part of a container, or access members";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      template<bool MUTABLE>
      static bool DefaultSelect(Block&, Verb&);
      template<bool MUTABLE, class META>
      static bool PerIndex(Block&, TAny<Trait>&, TMeta, META, const TAny<Index>&);
      template<bool MUTABLE>
      static bool SelectByMeta(const TAny<Index>&, DMeta, Block&, TAny<Trait>&, TAny<const RTTI::Ability*>&);
   };
}

#include "../../../source/verbs/Select.inl"
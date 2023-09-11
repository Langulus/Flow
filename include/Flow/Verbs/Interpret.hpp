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
   ///   Interpret                                                            
   /// Performs conversion                                                    
   ///                                                                        
   struct Interpret : StaticVerb<Interpret> {
      LANGULUS(VERB) "Interpret";
      LANGULUS(OPERATOR) " => ";
      LANGULUS(INFO) "Performs conversion";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<class TO, class FROM>
      static TO To(const FROM&);

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

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

#include "../../../source/verbs/Interpret.inl"
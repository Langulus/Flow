///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Interact.hpp"
#include "../TVerb.inl"

#define VERBOSE_INTERACT(...) //Logger::Verbose(__VA_ARGS__)


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Interact::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Interact(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Interact(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Interact::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Interact(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Interact(verb, args...);
         };
      }
   }
      
   /// Execute the interact verb in a specific context                        
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   bool Interact::ExecuteIn(CT::Dense auto& context, Verb& verb) {
      static_assert(
         Interact::AvailableFor<Deref<decltype(context)>>(),
         "Verb is not available for this context, "
         "this shouldn't be reached by flow"
      );
      context.Interact(verb);
      return verb.IsDone();
   }

} // namespace Langulus::Verbs

#undef VERBOSE_INTERACT
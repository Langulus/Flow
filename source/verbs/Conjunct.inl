///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Conjunct.hpp"
#include "../TVerb.inl"


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Conjunct::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Conjunct(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Conjunct(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Conjunct::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Conjunct(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Conjunct(verb, args...);
         };
      }
   }

   /// Execute the conjunction/disjunction verb in a specific context         
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Conjunct::ExecuteIn(T& context, Verb& verb) {
      static_assert(Conjunct::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Conjunct(verb);
      return verb.IsDone();
   }

   /// Default conjunction/disjunction                                        
   /// Produces a shallow copy of the provided context and arguments          
   ///   @param context - the block to execute in                             
   ///   @param verb - conjunction/disjunction verb                           
   inline bool Conjunct::ExecuteDefault(const Block&, Verb& verb) {
      Any joined;
      if (verb.GetMass() < 0) {
         joined.SmartPush(IndexBack, verb.GetSource(), DataState::Or);
         joined.SmartPush(IndexBack, verb.GetArgument(), DataState::Or);
      }
      else {
         joined.SmartPush(IndexBack, verb.GetSource());
         joined.SmartPush(IndexBack, verb.GetArgument());
      }

      verb << Abandon(joined);
      return true;
   }

   /// Stateless conjunction/disjunction                                       
   /// Essentially forwards the arguments to the output                        
   ///   @param verb - the conjunction/disjunction verb                        
   ///   @return true if verb was satisfied                                    
   inline bool Conjunct::ExecuteStateless(Verb& verb) {
      verb << verb.GetArgument();
      return true;
   }

} // namespace Langulus::Verbs

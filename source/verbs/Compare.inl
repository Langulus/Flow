///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Compare.hpp"
#include "../TVerb.inl"

#define VERBOSE_COMPARE(...) //Logger::Verbose(__VA_ARGS__)


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Compare::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0) {
         return requires (const T& t, Verb& v) { t.Compare(v); }
             or requires (const T& t) { {t == t} -> CT::Bool; }
             or requires (const T& t) { {t != t} -> CT::Bool; }
             or requires (const T& t) { {t >  t} -> CT::Bool; }
             or requires (const T& t) { {t <  t} -> CT::Bool; }
             or requires (const T& t) { {t <= t} -> CT::Bool; }
             or requires (const T& t) { {t >= t} -> CT::Bool; }
             or requires (const T& t) { t <=> t; };
      }
      else {
         return requires (const T& t, Verb& v, A... a) { t.Compare(v, a...); }
             or requires (const T& t, A... a) { {((t == a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { {((t != a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { {((t >  a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { {((t <  a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { {((t <= a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { {((t >= a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { ((t <=> a) == ...); };
      }
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Compare::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Compare(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Compare(verb, args...);
         };
      }
   }

   /// Execute the comparison verb in a specific context                      
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Compare::ExecuteIn(T& context, Verb& verb) {
      static_assert(Compare::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Compare(verb);
      return verb.IsDone();
   }

   /// Execute the default verb in an immutable context                       
   /// Returns immutable results                                              
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Compare::ExecuteDefault(const Block& context, Verb& verb) {
      if (verb.IsMissing() or not context or context.IsMissing())
         return false;

      // Scan verb argument for elements interpretable as the context   
      // Consider the hierarchy                                         
      verb.ForEach([&](const Block&) {
         TODO(); //compare
      });

      return verb.IsDone();
   }

} // namespace Langulus::Verbs

#undef VERBOSE_COMPARE
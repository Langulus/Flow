///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "GreaterOrEqual.hpp"
#include "../TVerb.inl"

#if 0
   #define VERBOSE_COMPARE(...) Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_COMPARE(...) LANGULUS(NOOP)
#endif


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool GreaterOrEqual::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0) {
         return requires (const T& t, Verb& v) { t.Compare(v); }
             or requires (const T& t, Verb& v) { t.GreaterOrEqual(v); }
             or requires (const T& t) { {t >= t} -> CT::Bool; }
             or requires (const T& t) { t <=> t; };
      }
      else {
         return requires (const T& t, Verb& v, A... a) { t.Compare(v, a...); }
             or requires (const T& t, Verb& v, A... a) { t.GreaterOrEqual(v, a...); }
             or requires (const T& t, A... a) { {((t >= a) and ...)} -> CT::Bool; }
             or requires (const T& t, A... a) { ((t <=> a) == ...); };
      }
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto GreaterOrEqual::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->GreaterOrEqual(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->GreaterOrEqual(verb, args...);
         };
      }
   }

   /// Execute the comparison verb in a specific context                      
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool GreaterOrEqual::ExecuteIn(T& context, Verb& verb) {
      static_assert(GreaterOrEqual::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.GreaterOrEqual(verb);
      return verb.IsDone();
   }

   /// Execute the default verb in an immutable context                       
   /// Returns immutable results                                              
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool GreaterOrEqual::ExecuteDefault(const Many& context, Verb& verb) {
      if (verb.IsMissing() or not context or context.IsMissing())
         return false;

      // Scan verb argument for elements interpretable as the context   
      // Consider the hierarchy                                         
      verb.ForEach([&](const Many&) {
         TODO(); //compare
      });

      return verb.IsDone();
   }

} // namespace Langulus::Verbs

#undef VERBOSE_COMPARE
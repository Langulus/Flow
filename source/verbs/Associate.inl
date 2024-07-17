///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Associate.hpp"
#include "../TVerb.inl"

#if 0
   #define VERBOSE_ASSOCIATE(...) Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_ASSOCIATE(...) LANGULUS(NOOP)
#endif


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Associate::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Associate(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Associate(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Associate::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Associate(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Associate(verb, args...);
         };
      }
   }

   /// Execute the association/dissociation verb in a specific context        
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   bool Associate::ExecuteIn(CT::Dense auto& context, Verb& verb) {
      static_assert(
         Associate::AvailableFor<Deref<decltype(context)>>(),
         "Verb is not available for this context, "
         "this shouldn't be reached by flow"
      );
      context.Associate(verb);
      return verb.IsDone();
   }

   /// Execute the default verb in a context                                  
   ///   @param lhs - the context to execute in                               
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Associate::ExecuteDefault(Many& lhs, Verb& verb) {
      const auto& rhs = verb.GetArgument();

      if (lhs.IsConstant() or lhs.GetCount() != rhs.GetCount())
         // Can't overwrite a constant context                          
         return false;
      else if (lhs.IsMissing() or rhs.IsMissing())
         // Can't associate missing stuff                               
         return false;
      else if (lhs.IsExecutableDeep() or rhs.IsExecutableDeep())
         // Can't associate unexecuted verbs                            
         return false;
      else if (not lhs.IsExact(rhs.GetType()))
         // Can't associate unrelated types                             
         return false;

      // Attempt directly refering, if possible                         
      // This will happen only if types are exactly the same            
      // This is a default (fallback) routine, let's keep things simple 
      try { lhs.AssignWithIntent(Refer(rhs)); }
      catch (...) { return false; }

      // At this point, context has a copy of verb's argument           
      // Just make sure it goes to output                               
      verb << lhs;
      return true;
   }

} // namespace Langulus::Verbs

#undef VERBOSE_ASSOCIATE
///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Do.inl"
#include "Interpret.inl"
#include "Catenate.inl"

#define VERBOSE_ASSOCIATE(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Data T, CT::Data... A>
   constexpr bool Associate::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Associate(v); };
      else
         return requires (T& t, Verb& v, A... a) { t.Associate(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Data T, CT::Data... A>
   constexpr auto Associate::Of() noexcept {
      if constexpr (!Associate::AvailableFor<T, A...>()) {
         return nullptr;
      }
      else if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Associate(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Associate(verb, args...);
         };
      }
   }

   /// Execute the association/dissociation verb in a specific context        
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Data T>
   bool Associate::ExecuteIn(T& context, Verb& verb) {
      static_assert(Associate::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Associate(verb);
      return verb.IsDone();
   }

   /// Execute the default verb in a context                                  
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Associate::ExecuteDefault(Block& context, Verb& verb) {
      auto& lhs = ReinterpretCast<Scope>(context);
      auto& rhs = ReinterpretCast<Scope>(verb.GetArgument());
      if (lhs.IsConstant() || lhs.GetCount() != rhs.GetCount())
         // Can't overwrite a constant context                          
         return false;
      else if (lhs.IsMissing() || rhs.IsMissing())
         // Can't associate missing stuff                               
         return false;
      else if (lhs.IsExecutableDeep() || rhs.IsExecutableDeep())
         // Can't associate unexecuted verbs                            
         return false;
      else if (!lhs.IsExact(rhs.GetType()))
         // Can't associate unrelated types                             
         return false;

      // Attempt directly copying, if possible                          
      // This will happen only if types are exactly the same            
      // This is a default (fallback) routine, let's keep things simple 
      try {
         lhs.CallUnknownSemanticAssignment(lhs.GetCount(), Copy(rhs));
      }
      catch (...) {
         return false;
      }

      // At this point, context has a copy of verb's argument           
      // Just make sure it goes to output                               
      verb << Any {context};
      return true;
   }

} // namespace Langulus::Verbs


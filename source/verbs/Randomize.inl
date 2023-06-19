///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Arithmetic.inl"
#include <Flow/Verbs/Randomize.hpp>
#include "../Verb.inl"

#define VERBOSE_RAND(...) //Logger::Verbose(__VA_ARGS__)

namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data... A>
   constexpr bool Randomize::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 1)
         return requires (T& t, Verb& v, A... a) { t.Randomize(v, a...); };
      else
         return false;
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data... A>
   constexpr auto Randomize::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Randomize(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Randomize(verb, args...);
         };
      }
   }

   /// Execute the modulate verb in a specific context                        
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Randomize::ExecuteIn(T& context, Verb& verb) {
      static_assert(Randomize::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Randomize(verb);
      return verb.IsDone();
   }

   /// Operate in a number of types                                           
   ///   @tparam ...T - the list of types to operate on                       
   ///                  order matters!                                        
   ///   @param context - the original context                                
   ///   @param common - the base to operate on                               
   ///   @param verb - the original verb                                      
   ///   @return if at least one of the types matched verb                    
   template<CT::Data... T>
   bool Randomize::OperateOnTypes(const Block& context, const Block& common, Verb& verb) {
      return ((
         common.CastsTo<T, true>() && ArithmeticVerb::Vector<T>(
            context, common, verb,
               [](const T* lhs, const T* rhs) -> T {
                  return *lhs % *rhs;
               }
         )
      ) || ...);
   }

   /// Operate in a number of types (destructive version)                     
   ///   @tparam ...T - the list of types to operate on                       
   ///                  order matters!                                        
   ///   @param context - the original context                                
   ///   @param common - the base to operate on                               
   ///   @param verb - the original verb                                      
   ///   @return if at least one of the types matched verb                    
   template<CT::Data... T>
   bool Randomize::OperateOnTypes(const Block& context, Block& common, Verb& verb) {
      return ((
         common.CastsTo<T, true>() && ArithmeticVerb::Vector<T>(
            context, common, verb,
               [](T* lhs, const T* rhs) {
                  *lhs %= *rhs;
               }
         )
      ) || ...);
   }

   /// Default multiply/divide in an immutable context                        
   ///   @param context - the block to execute in                             
   ///   @param verb - multiply/divide verb                                   
   inline bool Randomize::ExecuteDefault(const Block& context, Verb& verb) {
      const auto common = context.ReinterpretAs(verb);
      if (common.CastsTo<A::Number>()) {
         return OperateOnTypes<
            Float, Double,
            int32_t, uint32_t, int64_t, uint64_t,
            int8_t, uint8_t, int16_t, uint16_t
         >(context, common, verb);
      }

      return false;
   }

   /// Default multiply/divide in mutable context                             
   ///   @param context - the block to execute in                             
   ///   @param verb - multiply/divide verb                                   
   inline bool Randomize::ExecuteDefault(Block& context, Verb& verb) {
      const auto common = context.ReinterpretAs(verb);
      if (common.CastsTo<A::Number>()) {
         return OperateOnTypes<
            Float, Double,
            int32_t, uint32_t, int64_t, uint64_t,
            int8_t, uint8_t, int16_t, uint16_t
         >(context, common, verb);
      }

      return false;
   }

} // namespace Langulus::Verbs

#undef VERBOSE_RAND
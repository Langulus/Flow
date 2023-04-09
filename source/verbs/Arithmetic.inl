///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Verb.hpp"

namespace Langulus::Flow
{

   /// Directly reinterprets lhs and rhs as the provided T and uses provided  
   /// operator on each of the elements                                       
   ///   @tparam T - type to interpret as                                     
   ///   @param lhs - left operand                                            
   ///   @param rhs - right operand                                           
   template<class VERB, bool NOEXCEPT>
   template<CT::Data T>
   LANGULUS(INLINED)
   bool ArithmeticVerb<VERB, NOEXCEPT>::Vector(const Block& original, const Block& lhs, Verb& rhs, Operator<T> o) noexcept (NOEXCEPT) {
      //TODO use TSIMD to batch compute
      //TODO once vulkan module is available, lock and replace the ExecuteDefault in
      // MVulkan to incorporate compute shader for even batcher batching!!1
      //TODO detect underflows and overflows
      auto result = Block::From<T>();
      result.AllocateFresh(result.RequestSize(lhs.GetCount()));
      result.mCount = lhs.GetCount();
      const T* ilhs = lhs.GetRawAs<T>();
      const T* const ilhsEnd = ilhs + lhs.GetCount();
      const T* irhs = rhs.GetRawAs<T>();
      T* ires = result.template GetRawAs<T>();
      while (ilhs != ilhsEnd)
         *(ires++) = o(ilhs++, irhs++);

      // Interpret back to the original and push to verb output         
      rhs << result.ReinterpretAs(original);
      result.Free();
      return true;
   }

   /// Directly reinterprets lhs and rhs as the provided T and uses provided  
   /// operator on each of the elements. This doesn't reallocate, but directly
   /// writes to lhs (destructive operation)                                  
   ///   @tparam T - type to interpret as                                     
   ///   @param lhs - left operand                                            
   ///   @param rhs - right operand                                           
   template<class VERB, bool NOEXCEPT>
   template<CT::Data T>
   LANGULUS(INLINED)
   bool ArithmeticVerb<VERB, NOEXCEPT>::Vector(const Block& original, Block& lhs, Verb& rhs, OperatorMutable<T> o) noexcept (NOEXCEPT) {
      //TODO use TSIMD to batch compute
      //TODO once vulkan module is available, lock and replace the ExecuteDefault in
      // MVulkan to incorporate compute shader for even batcher batching!!1
      //TODO detect underflows and overflows
      T* ilhs = lhs.GetRawAs<T>();
      const T* const ilhsEnd = ilhs + lhs.GetCount();
      const T* irhs = rhs.GetRawAs<T>();
      while (ilhs != ilhsEnd)
         o(ilhs++, irhs++);

      // We're editing through the reinterpretation, but we must return 
      // the original                                                   
      rhs << Any {original};
      return true;
   }

   /// Directly reinterprets lhs and rhs as the provided T and uses provided  
   /// operator on each of the elements. This is the scalar version, which    
   /// means that rhs is not iterated, only first element is used             
   ///   @tparam T - type to interpret as                                     
   ///   @param lhs - left operand                                            
   ///   @param rhs - right operand                                           
   template<class VERB, bool NOEXCEPT>
   template<CT::Data T>
   LANGULUS(INLINED)
   bool ArithmeticVerb<VERB, NOEXCEPT>::Scalar(const Block& original, const Block& lhs, Verb& rhs, Operator<T> o) noexcept (NOEXCEPT) {
      //TODO use TSIMD to batch compute
      //TODO once vulkan module is available, lock and replace the ExecuteDefault in
      // MVulkan to incorporate compute shader for even batcher batching!!1
      //TODO detect underflows and overflows
      TAny<T> result;
      result.AllocateFresh(result.RequestSize(lhs.GetCount()));
      result.mCount = lhs.GetCount();
      const T* ilhs = lhs.GetRawAs<T>();
      const T* const ilhsEnd = ilhs + lhs.GetCount();
      const T& irhs = *rhs.GetRawAs<T>();
      T* ires = result.template GetRawAs<T>();
      while (ilhs != ilhsEnd)
         *(ires++) = o(ilhs++, &irhs);

      // Interpret back to the original and push to verb output         
      rhs << result.ReinterpretAs(original);
      return true;
   }

   /// Directly reinterprets lhs and rhs as the provided T and uses provided  
   /// operator on each of the elements. This is the scalar version, which    
   /// means that rhs is not iterated, only first element is used             
   /// This doesn't reallocate, but directly writes to lhs (destructive)      
   ///   @tparam T - type to interpret as                                     
   ///   @param lhs - left operand                                            
   ///   @param rhs - right operand                                           
   template<class VERB, bool NOEXCEPT>
   template<CT::Data T>
   LANGULUS(INLINED)
   bool ArithmeticVerb<VERB, NOEXCEPT>::Scalar(const Block& original, Block& lhs, Verb& rhs, OperatorMutable<T> o) noexcept (NOEXCEPT) {
      //TODO use TSIMD to batch compute
      //TODO once vulkan module is available, lock and replace the ExecuteDefault in
      // MVulkan to incorporate compute shader for even batcher batching!!1
      //TODO detect underflows and overflows
      T* ilhs = lhs.GetRawAs<T>();
      const T* const ilhsEnd = ilhs + lhs.GetCount();
      const T& irhs = *rhs.GetRawAs<T>();
      while (ilhs != ilhsEnd)
         o(ilhs++, &irhs);

      // We're editing through the reinterpretation, but we must return 
      // the original                                                   
      rhs << Any {original};
      return true;
   }

} // namespace Langulus::Flow


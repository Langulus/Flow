///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../../include/Flow/Verbs/Catenate.hpp"
#include "../Verb.inl"

namespace Langulus
{
   
   /// Number of digits in a value                                            
   /// Credit goes to http://stackoverflow.com/questions/1489830              
   NOD() constexpr LANGULUS(INLINED) Count CountDigits(uint8_t x) noexcept {
      return (x < 10u ? 1 : (x < 100u ? 2 : 3));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(int8_t x) noexcept {
      return CountDigits(static_cast<uint8_t>(::std::abs(x)));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(uint16_t x) noexcept {
      return (x < 10u ? 1 : (x < 100u ? 2 : (x < 1000u ? 3 : (x < 10000u ? 4 : 5))));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(int16_t x) noexcept {
      return CountDigits(static_cast<uint16_t>(::std::abs(x)));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(uint32_t x) noexcept {
      return
         (x < 10u ? 1 :
         (x < 100u ? 2 :
         (x < 1000u ? 3 :
         (x < 10000u ? 4 :
         (x < 100000u ? 5 :
         (x < 1000000u ? 6 :
         (x < 10000000u ? 7 :
         (x < 100000000u ? 8 :
         (x < 1000000000u ? 9 : 10)))))))));
   }

   NOD() constexpr LANGULUS(INLINED) Count pcNumDigits(int32_t x) noexcept {
      return CountDigits(static_cast<uint32_t>(::std::abs(x)));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(uint64_t x) noexcept {
      return
         (x < 10ull ? 1 :
         (x < 100ull ? 2 :
         (x < 1000ull ? 3 :
         (x < 10000ull ? 4 :
         (x < 100000ull ? 5 :
         (x < 1000000ull ? 6 :
         (x < 10000000ull ? 7 :
         (x < 100000000ull ? 8 :
         (x < 1000000000ull ? 9 :
         (x < 10000000000ull ? 10 :
         (x < 100000000000ull ? 11 :
         (x < 1000000000000ull ? 12 :
         (x < 10000000000000ull ? 13 :
         (x < 100000000000000ull ? 14 :
         (x < 1000000000000000ull ? 15 :
         (x < 10000000000000000ull ? 16 :
         (x < 100000000000000000ull ? 17 :
         (x < 1000000000000000000ull ? 18 :
         (x < 10000000000000000000ull ? 19 : 20
      )))))))))))))))))));
   }

   NOD() constexpr LANGULUS(INLINED) Count CountDigits(int64_t x) noexcept {
      // http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs 
      int const mask = x >> (sizeof(int64_t) * 8 - 1);
      return CountDigits(static_cast<uint64_t>((x + mask) ^ mask));
   }

   /// Count digits in real numbers                                           
   /// The dot in the real number is considered a digit, too                  
   ///   @param x - real number to cound digits of                            
   template<CT::Real T>
   NOD() constexpr LANGULUS(INLINED) Count CountDigits(T x) noexcept {
      T floored;
      T fraction {::std::abs(::std::modf(x, &floored))};
      if (fraction == 0)
         return CountDigits(static_cast<uint64_t>(floored));

      floored = ::std::abs(floored);
      T limit {1};
      Count fract_numbers {};
      while (fraction < limit && limit < T {1000}) {
         fraction *= T {10};
         limit *= T {10};
         ++fract_numbers;
      }

      return CountDigits(static_cast<uint64_t>(floored)) + fract_numbers + Count {1};
   }

   /// Concatenate two numbers                                                
   ///   @param lhs - left number                                             
   ///   @param rhs - right number                                            
   ///   @return the concatenation of the two numbers                         
   template<CT::Number T>
   NOD() LANGULUS(INLINED) T ConcatenateNumbers(const T& lhs, const T& rhs) {
      T result {lhs};
      result *= ::std::pow(T {10}, static_cast<T>(CountDigits(rhs)));
      result += rhs;
      return result;
   }

} // namespace Langulus


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data... A>
   constexpr bool Catenate::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Catenate(v); };
      else
         return requires (T& t, Verb& v, A... a) { t.Catenate(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data... A>
   constexpr auto Catenate::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Catenate(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Catenate(verb, args...);
         };
      }
   }

   /// Execute the catenation/splitting verb in a specific context            
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Catenate::ExecuteIn(T& context, Verb& verb) {
      static_assert(Catenate::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Catenate(verb);
      return verb.IsDone();
   }

   /// Default catenation/splitting in an immutable context                   
   /// Produces a shallow copy of the catenated context and arguments         
   ///   @param context - the block to execute in                             
   ///   @param verb - catenation/splitting verb                              
   inline bool Catenate::ExecuteDefault(const Block& context, Verb& verb) {
      if (verb.IsMissing()) {
         // Don't catenate immediately missing elements                 
         return false;
      }

      if (!verb) {
         verb << Any {context};
         return true;
      }

      //TODO split
      TAny<Any> shallow;
      shallow << Any {context};
      shallow << verb.GetArgument();
      verb << Abandon(shallow);
      return true;
   }

   /// Default catenation/splitting in a mutable context                      
   /// Reuses the context, by catenating/splitting inside it if possible      
   ///   @param context - the block to execute in                             
   ///   @param verb - catenation/splitting verb                              
   inline bool Catenate::ExecuteDefault(Block& context, Verb& verb) {
      if (verb.IsMissing()) {
         // Don't catenate immediately missing elements                 
         return false;
      }

      if (!verb) {
         verb << Any {context};
         return true;
      }

      //TODO split
      reinterpret_cast<Any&>(context).SmartPush(Move(verb.GetArgument()));
      verb << Any {context};
      return true;
   }

   /// A stateless catenation - just results in RHS                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Catenate::ExecuteStateless(Verb& verb) {
      verb << Move(verb.GetArgument());
      return true;
   }

} // namespace Langulus::Verbs

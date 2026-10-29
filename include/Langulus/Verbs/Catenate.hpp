///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>


///                                                                           
/// MARK: Catenate/Split                                                      
/// Catenates anything catenable, or splits stuff apart using a mask          
///                                                                           
LANGULUS_DEFINE_OPERATOR(Catenate, Split, " >< ", " <> ", 7,
   "Concatenates, or splits stuff apart"
);

namespace Langulus
{
   /// Count digits in real numbers.                                          
   /// The dot in the real number is considered a digit, too.                 
   /// Credit goes to http://stackoverflow.com/questions/1489830              
   ///   @param x real number to count digits of                              
   template<CT::Integer T>
   constexpr size_t CountDigits(T x) noexcept {
      if constexpr (CT::UnsignedInteger8<T>)
         return (x < 10u ? 1 : (x < 100u ? 2 : 3));
      else if constexpr (CT::SignedInteger8<T>)
         return CountDigits(static_cast<uint8_t>(x < 0 ? -x : x));
      else if constexpr (CT::UnsignedInteger16<T>)
         return (x < 10u ? 1 : (x < 100u ? 2 : (x < 1000u ? 3 : (x < 10000u ? 4 : 5))));
      else if constexpr (CT::SignedInteger16<T>)
         return CountDigits(static_cast<uint16_t>(x < 0 ? -x : x));
      else if constexpr (CT::UnsignedInteger32<T>) {
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
      else if constexpr (CT::SignedInteger32<T>)
         return CountDigits(static_cast<uint32_t>(x < 0 ? -x : x));
      else if constexpr (CT::UnsignedInteger64<T>) {
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
      else if constexpr (CT::SignedInteger64<T>) {
         // http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs 
         int const mask = x >> (sizeof(int64_t) * 8 - 1);
         return CountDigits(static_cast<uint64_t>((x + mask) ^ mask));
      }
      else static_assert(false, "Unimplemented integer");
   }

   /// Count digits in real numbers                                           
   ///   @param x real number to count digits of                              
   template<CT::Real T>
   constexpr size_t CountDigits(T x) noexcept {
      T floored;
      T fraction = ::std::abs(::std::modf(x, &floored));
      if (fraction == 0)
         return CountDigits(static_cast<uint64_t>(floored));

      if (floored < 0)
         floored = -floored;

      T limit = 1;
      size_t fract_numbers = 0;
      while (fraction < limit and limit < T {1000}) {
         fraction *= T {10};
         limit *= T {10};
         ++fract_numbers;
      }

      return CountDigits(static_cast<uint64_t>(floored)) + fract_numbers + size_t {1};
   }

   /// Concatenate two numbers                                                
   ///   @return the concatenation of the two numbers                         
   template<CT::Number T>
   T ConcatenateNumbers(const T& lhs, const T& rhs) {
      T result = lhs;
      result *= ::std::pow(T {10}, static_cast<T>(CountDigits(rhs)));
      result += rhs;
      return result;
   }
}
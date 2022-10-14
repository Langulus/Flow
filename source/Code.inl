///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Code.hpp"

namespace Langulus::Flow
{

   /// Remove elements from the left side of Code code                        
   ///   @param offset - the number of elements to discard from the front     
   ///   @return a shallow-copied container with the correct offset           
   inline Code Code::RightOf(Offset o) const {
      return Code {Text::Crop(o, mCount - o)};
   }

   /// Remove elements from the right side of Code code                       
   ///   @param offset - the number of elements to remain in container        
   ///   @return a shallow-copied container with the correct offset           
   inline Code Code::LeftOf(Offset o) const {
      return Code {Text::Crop(0, o)};
   }

   /// Check if the Code container begins with special elements, such as      
   /// special characters or escape sequences, like colors                    
   ///   @return true if the first symbol is special                          
   inline bool Code::StartsWithSpecial() const noexcept {
      const auto& letter = (*this)[0];
      return !IsEmpty() && letter > 0 && letter < 32;
   }

   /// Check if the Code container begins with skippable elements, such as    
   /// tabs or spaces, or special characters/sequences                        
   ///   @return true if the first symbol is a spacer                         
   inline bool Code::StartsWithSkippable() const noexcept {
      const auto& letter = (*this)[0];
      return !IsEmpty() && letter > 0 && letter <= 32;
   }

   /// Check if the Code code container begins with skippable elements        
   ///   @return true if the first symbol is a spacer                         
   inline bool Code::EndsWithSkippable() const noexcept {
      return !IsEmpty() && last() > 0 && last() <= 32;
   }

   /// Check if the Code code container begins with a letter or underscore    
   ///   @return true if the first symbol is a letter/underscore              
   inline bool Code::StartsWithLetter() const noexcept {
      const auto c = (*this)[0];
      return !IsEmpty() && (::std::isalpha(c) || c == '_');
   }

   /// Check if the Code code container ends with a letter or underscore      
   ///   @return true if the last symbol is a letter/underscore               
   inline bool Code::EndsWithLetter() const noexcept {
      const auto c = last();
      return !IsEmpty() && (::std::isalpha(c) || c == '_');
   }

   /// Check if the Code code container begins with a number                  
   ///   @return true if the first symbol is a number                         
   inline bool Code::StartsWithDigit() const noexcept {
      return !IsEmpty() && ::std::isdigit((*this)[0]);
   }

   /// Check if the Code code container ends with a number                    
   ///   @return true if the last symbol is a number                          
   inline bool Code::EndsWithDigit() const noexcept {
      return !IsEmpty() && ::std::isdigit(last());
   }

   /// Check if the Code code container begins with an operator               
   ///   @param i - the operator to check for                                 
   ///   @return true if the operator matches                                 
   inline bool Code::StartsWithOperator(Offset i) const noexcept {
      const Size tokenSize = mOperators[i].mToken.size();
      if (!tokenSize || mCount < tokenSize)
         return false;

      const auto token = Code(mOperators[i].mToken);
      const auto remainder = RightOf(tokenSize);
      const auto endsWithALetter = token.EndsWithLetter();
      return tokenSize > 0 && MatchesLoose(token) == tokenSize
         && (GetCount() == tokenSize 
            || (endsWithALetter && (!remainder.StartsWithLetter() && !remainder.StartsWithDigit()))
            || !endsWithALetter
         );
   }

   /// Append a built-in operator to the code                                 
   ///   @param o - the built-in operator enumerator                          
   ///   @return a reference to this code for chaining                        
   inline Code& Code::operator += (Operator o) {
      Text::operator += (mOperators[o].mToken);
      return *this;
   }

} // namespace Langulus::Flow

namespace Langulus
{

   /// Make a code literal                                                    
   inline Flow::Code operator "" _code(const char* text, ::std::size_t size) {
      return {text, size};
   }

} // namespace Langulus

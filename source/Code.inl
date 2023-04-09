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

   /// Copy-constructor from Text                                             
   ///   @param other - the Text container to shallow-copy                    
   LANGULUS(INLINED)
   Code::Code(const Text& other)
      : Text {other} { }

   /// Move-constructor from Text                                             
   ///   @param other - the Text container to move                            
   LANGULUS(INLINED)
   Code::Code(Text&& other)
      : Text {Forward<Text>(other)} { }

   /// Generate code from operator                                            
   ///   @param op - the operator to stringify                                
   LANGULUS(INLINED)
   Code::Code(Operator op)
      : Text {Disown(GlobalOperators[op].mToken.data())} { }

   /// Remove elements from the left side of Code code                        
   ///   @param offset - the number of elements to discard from the front     
   ///   @return a shallow-copied container with the correct offset           
   LANGULUS(INLINED)
   Code Code::RightOf(Offset o) const {
      return Code {Text::Crop(o, GetCount() - o)};
   }

   /// Remove elements from the right side of Code code                       
   ///   @param offset - the number of elements to remain in container        
   ///   @return a shallow-copied container with the correct offset           
   LANGULUS(INLINED)
   Code Code::LeftOf(Offset o) const {
      return Code {Text::Crop(0, o)};
   }

   /// Check if the Code container begins with special elements, such as      
   /// special characters or escape sequences, like colors                    
   ///   @return true if the first symbol is special                          
   LANGULUS(INLINED)
   bool Code::StartsWithSpecial() const noexcept {
      const auto& letter = (*this)[0];
      return !IsEmpty() && letter > 0 && letter < 32;
   }

   /// Check if the Code container begins with skippable elements, such as    
   /// tabs or spaces, or special characters/sequences                        
   ///   @return true if the first symbol is a spacer                         
   LANGULUS(INLINED)
   bool Code::StartsWithSkippable() const noexcept {
      const auto& letter = (*this)[0];
      return !IsEmpty() && letter > 0 && letter <= 32;
   }

   /// Check if the Code code container begins with skippable elements        
   ///   @return true if the first symbol is a spacer                         
   LANGULUS(INLINED)
   bool Code::EndsWithSkippable() const noexcept {
      return !IsEmpty() && last() > 0 && last() <= 32;
   }

   /// Check if the Code code container begins with a letter or underscore    
   ///   @return true if the first symbol is a letter/underscore              
   LANGULUS(INLINED)
   bool Code::StartsWithLetter() const noexcept {
      const auto c = (*this)[0];
      return !IsEmpty() && (::std::isalpha(c) || c == '_');
   }

   /// Check if the Code code container ends with a letter or underscore      
   ///   @return true if the last symbol is a letter/underscore               
   LANGULUS(INLINED)
   bool Code::EndsWithLetter() const noexcept {
      const auto c = last();
      return !IsEmpty() && (::std::isalpha(c) || c == '_');
   }

   /// Check if the Code code container begins with a number                  
   ///   @return true if the first symbol is a number                         
   LANGULUS(INLINED)
   bool Code::StartsWithDigit() const noexcept {
      return !IsEmpty() && ::std::isdigit((*this)[0]);
   }

   /// Check if the Code code container ends with a number                    
   ///   @return true if the last symbol is a number                          
   LANGULUS(INLINED)
   bool Code::EndsWithDigit() const noexcept {
      return !IsEmpty() && ::std::isdigit(last());
   }

} // namespace Langulus::Flow

namespace Langulus
{

   /// Make a code literal                                                    
   LANGULUS(INLINED)
   Flow::Code operator "" _code(const char* text, ::std::size_t size) {
      return {text, size};
   }

} // namespace Langulus

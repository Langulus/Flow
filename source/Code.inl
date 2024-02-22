///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
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
   LANGULUS(INLINED)
   Code Code::RightOf(Offset offset) const {
      return offset < mCount ? Text::Crop(offset) : Code {};
   }

   /// Remove elements from the right side of Code code                       
   ///   @param offset - the number of elements to remain in container        
   ///   @return a shallow-copied container with the correct offset           
   LANGULUS(INLINED)
   Code Code::LeftOf(Offset offset) const {
      return offset > 0 ? Text::Crop(0, offset) : Code {};
   }

   /// Check if the Code container begins with special elements, such as      
   /// special characters or escape sequences, like colors                    
   ///   @return true if the first symbol is special                          
   LANGULUS(INLINED)
   bool Code::StartsWithSpecial() const noexcept {
      const auto& letter = (*this)[0];
      return letter > 0 and letter < 32;
   }

   /// Check if the Code container begins with skippable elements, such as    
   /// tabs or spaces, comment blocks, or special character sequences (TODO)  
   ///   @return true if the first symbol is skippable                        
   LANGULUS(INLINED)
   bool Code::StartsWithSkippable() const noexcept {
      if (IsEmpty())
         return false;

      const auto& letter = (*this)[0];
      if (letter > 0 and letter <= 32)
         return true;

      const auto asview = Token {*this};
      return asview.starts_with("//") or asview.starts_with("/*");
   }

   /// Check if the Code container begins with skippable elements             
   ///   @return true if the first symbol is a spacer                         
   LANGULUS(INLINED)
   bool Code::EndsWithSkippable() const noexcept {
      return *last() > 0 and *last() <= 32;
   }

   /// Check if the Code code container begins with a letter or underscore    
   ///   @return true if the first symbol is a letter/underscore              
   LANGULUS(INLINED)
   bool Code::StartsWithLetter() const noexcept {
      const auto c = (*this)[0];
      return IsAlpha(c) or c == '_';
   }

   /// Check if the Code code container ends with a letter or underscore      
   ///   @return true if the last symbol is a letter/underscore               
   LANGULUS(INLINED)
   bool Code::EndsWithLetter() const noexcept {
      const auto c = last();
      return IsAlpha(*c) or *c == '_';
   }

   /// Check if the Code code container begins with a number                  
   ///   @return true if the first symbol is a number                         
   LANGULUS(INLINED)
   bool Code::StartsWithDigit() const noexcept {
      return IsDigit((*this)[0]);
   }

   /// Check if the Code code container ends with a number                    
   ///   @return true if the last symbol is a number                          
   LANGULUS(INLINED)
   bool Code::EndsWithDigit() const noexcept {
      return IsDigit(*last());
   }
   
   /// Concatenate two text containers                                        
   ///   @param rhs - right hand side                                         
   ///   @return the concatenated text container                              
   template<class T> requires CT::Codifiable<Desem<T>> LANGULUS(INLINED)
   Code Code::operator + (T&& rhs) const {
      return Text::ConcatInner<Code>(Forward<T>(rhs));
   }

   /// Concatenate (destructively) text containers                            
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   template<class T> requires CT::Codifiable<Desem<T>> LANGULUS(INLINED)
   Code& Code::operator += (T&& rhs) {
      return Text::ConcatRelativeInner<Code>(Forward<T>(rhs));
   }

} // namespace Langulus::Flow

namespace Langulus
{

   /// Make a code literal                                                    
   LANGULUS(INLINED)
   Flow::Code operator "" _code(const char* text, ::std::size_t size) {
      return Anyness::Text::From(text, size);
   }

} // namespace Langulus

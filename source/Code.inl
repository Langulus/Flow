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
      return Text::Crop(o);
   }

   /// Remove elements from the right side of Code code                       
   ///   @param offset - the number of elements to remain in container        
   ///   @return a shallow-copied container with the correct offset           
   LANGULUS(INLINED)
   Code Code::LeftOf(Offset o) const {
      return Text::Crop(0, o);
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
   template<class T> requires CT::Codifiable<Desem<T>>
   Code Code::operator + (T&& rhs) const {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;

      if constexpr (CT::Block<B>) {
         if constexpr (CT::Typed<B>) {
            if constexpr (CT::Similar<Letter, TypeOf<B>>) {
               // We can concat directly                                
               return Block::ConcatBlock<Code>(S::Nest(rhs));
            }
            else if constexpr (CT::DenseCharacter<TypeOf<B>>) {
               // We're concatenating with different type of characters 
               // - do UTF conversions here                             
               TODO();
            }
            else LANGULUS_ERROR("Can't concatenate with this container");
         }
         else {
            // Type-erased concat                                       
            return Block::ConcatBlock<Code>(S::Nest(rhs));
         }
      }
      else {
         // RHS isn't Block, try to convert it to Text, and nest        
         return operator + (static_cast<Code>(DesemCast(rhs)));
      }
   }

   /// Concatenate (destructively) text containers                            
   ///   @param rhs - right hand side                                         
   ///   @return a reference to this container                                
   template<class T> requires CT::Codifiable<Desem<T>>
   Code& Code::operator += (T&& rhs) {
      using S = SemanticOf<T>;
      using B = TypeOf<S>;

      if constexpr (CT::Block<B>) {
         if constexpr (CT::Typed<B>) {
            if constexpr (CT::Similar<Letter, TypeOf<B>>) {
               // We can concat directly                                
               Block::InsertBlock<Code, void>(IndexBack, S::Nest(rhs));
            }
            else if constexpr (CT::DenseCharacter<TypeOf<B>>) {
               // We're concatenating with different type of characters 
               // - do UTF conversions here                             
               TODO();
            }
            else LANGULUS_ERROR("Can't concatenate with this container");
         }
         else {
            // Type-erased concat                                       
            Block::InsertBlock<Code, void>(IndexBack, S::Nest(rhs));
         }
      }
      else {
         // RHS isn't Block, try to convert it to Text, and nest        
         return operator += (static_cast<Code>(DesemCast(rhs)));
      }

      return *this;
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

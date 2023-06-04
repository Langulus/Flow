///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Scope.hpp"
#include "Construct.hpp"

namespace Langulus::Flow
{

   ///                                                                        
   ///   Langulus code container and parser, as well as keyword database      
   ///                                                                        
   struct Code : Text {
      LANGULUS_BASES(Text);

      enum Operator {
         OpenScope = 0,
         CloseScope,
         OpenCode,
         CloseCode,
         OpenComment,
         CloseComment,
         OpenString,
         CloseString,
         OpenStringAlt,
         CloseStringAlt,
         OpenCharacter,
         CloseCharacter,
         OpenByte,
         Future,
         Past,
         Constant,
         Mass,
         Rate,
         Time,
         Priority,

         OpCounter,
         NoOperator = OpCounter,
         ReflectedOperator,
         ReflectedVerb
      };

      struct OperatorProperties {
         Token mToken;
         Real mPrecedence;
         bool mCharge;
      };

      /// Built-in operator properties                                        
      static constexpr OperatorProperties GlobalOperators[Code::OpCounter] = {
         { "(", 0, false },      // OpenScope
         { ")", 0, false },      // CloseScope
         { "[", 0, false },      // OpenCode
         { "]", 0, false },      // CloseCode
         { "|", 0, false },      // OpenComment
         { "|", 0, false },      // CloseComment
         { "\"", 0, false },     // OpenString
         { "\"", 0, false },     // CloseString
         { "`", 0, false },      // OpenStringAlt
         { "`", 0, false },      // CloseStringAlt
         { "'", 0, false },      // OpenCharacter
         { "'", 0, false },      // CloseCharacter
         { "0x", 0, false },     // OpenByte
         { "??", 0, false },     // Future
         { "?", 0, false },      // Past
         { "const", 0, false },  // Constant
         { "*", 0, true },       // Mass
         { "^", 0, true },       // Frequency
         { "@", 0, true },       // Time
         { "!", 0, true }        // Priority
      };

      using Text::Text;

      LANGULUS_API(FLOW) Code(const Text&);
      LANGULUS_API(FLOW) Code(Text&&);

      LANGULUS_API(FLOW) explicit Code(Operator);

      NOD() LANGULUS_API(FLOW) Scope Parse(bool optimize = true) const;

      NOD() LANGULUS_API(FLOW) Code RightOf(Offset) const;
      NOD() LANGULUS_API(FLOW) Code LeftOf(Offset) const;
      NOD() LANGULUS_API(FLOW) bool StartsWithSpecial() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithSkippable() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithSkippable() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithLetter() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithLetter() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithDigit() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithDigit() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithOperator(Offset) const noexcept;

      using Text::operator +=;
      LANGULUS_API(FLOW) Code& operator += (Operator);

      template<class T>
      Code& TypeSuffix();

      NOD() LANGULUS_API(FLOW) static bool IsReserved(const Text&);
      NOD() LANGULUS_API(FLOW) static bool IsValidKeyword(const Text&);

   protected:
      /// Parser for unknown expressions                                      
      /// An unknown-expressions will be scanned to figure what it contains   
      struct LANGULUS_API(FLOW) UnknownParser {
         NOD() static Offset Parse(const Code&, Any&, Real, bool optimize);
      };

      /// Parser for keyword expressions                                      
      /// A key-expression is any expression that begins with a letter        
      struct LANGULUS_API(FLOW) KeywordParser {
         NOD() static Offset Parse(const Code&, Any&, bool allowCharge = true);
         NOD() static bool Peek(const Code&) noexcept;
         NOD() static Token Isolate(const Code&) noexcept;
         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            NOD() static const RTTI::Meta* Disambiguate(Offset, const Code&, const Token&);
         #endif
      };

      /// Parser for skipping expressions                                     
      /// A skip-expression is any that begins with escapes, tabs, or spaces  
      struct LANGULUS_API(FLOW) SkippedParser {
         NOD() static Offset Parse(const Code&);
         NOD() static bool Peek(const Code&) noexcept;
      };

      /// Parser for number expressions                                       
      /// A num-expression is any that begins with a digit, a minus           
      /// followed by a digit, or a dot followed by a digit                   
      struct LANGULUS_API(FLOW) NumberParser {
         NOD() static Offset Parse(const Code&, Any&);
         NOD() static bool Peek(const Code&) noexcept;
      };

      /// Parser for operators                                                
      /// An op-expression is one matching the built-in ones, or one matching 
      /// one in reflected verb database, where LHS is not DMeta or VMeta     
      struct LANGULUS_API(FLOW) OperatorParser {
         NOD() static Offset Parse(Operator, const Code&, Any&, Real, bool optimize);
         NOD() static Operator PeekBuiltin(const Code&) noexcept;
         NOD() static Operator Peek(const Code&) noexcept;
         NOD() static Token Isolate(const Code&) noexcept;

      private:
         NOD() static Offset ParseContent(const Code&, Any&, bool optimize);
         NOD() static Offset ParseString(Code::Operator, const Code&, Any&);
         NOD() static Offset ParseBytes(const Code&, Any&);
         NOD() static Offset ParsePhase(Code::Operator, Any&);
         NOD() static Offset ParseConst(Any&);
         NOD() static Offset ParseReflected(Verb&, const Code&, Any&, bool optimize);

         static void InsertContent(Any&, Any&);
      };

      /// Parser for chargers                                                 
      /// A charge-expression is any operator *^@! after a DMeta or VMeta     
      struct LANGULUS_API(FLOW) ChargeParser {
         NOD() static Offset Parse(const Code&, Charge&);
         NOD() static Operator Peek(const Code&) noexcept;
      };
   };

} // namespace Langulus::Flow

#include "Code.inl"

namespace Langulus::Verbs
{

   using Flow::Scope;
   using Flow::Construct;

} // namespace Langulus::Verbs

namespace Langulus
{
   Flow::Code operator "" _code(const char*, ::std::size_t);
}
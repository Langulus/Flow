///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Executor.hpp"
#include <Anyness/Text.hpp>


namespace Langulus::Flow
{
   struct Code;
}

namespace Langulus::CT
{
   namespace Inner
   {
   
      /// Workaround, because of MSVC ICEs introduced in 19.40.33811.0        
      /// Hopefully it will be resolved by them one day                       
      template<class T>
      consteval bool CodifiableByOperator_AvoidMSVC_ICE() {
         return std::is_object_v<T> and requires (const T& a) {
            a.operator ::Langulus::Flow::Code();
         };
      }

      /// Do types have an explicit or implicit cast operator to Code         
      template<class...T>
      concept CodifiableByOperator = 
         (CodifiableByOperator_AvoidMSVC_ICE<T>() and ...);

      /// Does Code has an explicit/implicit constructor that accepts T       
      template<class...T>
      concept CodifiableByConstructor = requires (const T&...a) {
         ((::Langulus::Flow::Code {a}), ...); };

   } // namespace Langulus::CT::Inner

   /// A codifiable type is one that has either an implicit or explicit       
   /// cast operator to Code type, or can be used to explicitly initialize a  
   /// Code container                                                         
   template<class...T>
   concept Codifiable = ((Inner::CodifiableByOperator<T>
        or Inner::CodifiableByConstructor<T>) and ...);

} // namespace Langulus::CT

namespace Langulus::Flow
{

   ///                                                                        
   ///   Langulus code container, parser, serializer and deserializer         
   ///                                                                        
   struct Code : A::Code {
      LANGULUS(NAME) "Code";
      LANGULUS(FILES) "flow";
      LANGULUS(ACT_AS) Code;
      LANGULUS_BASES(A::Code);

      /// The presence of this structure makes Code a serializer              
      struct SerializationRules : Text::SerializationRules {
         // Code serializer can't be lossy - it's isomorphic            
         static constexpr bool CriticalFailure = true;
         static constexpr bool SkipElements = false;
      };

      using Operator = SerializationRules::Operator;

      using A::Code::Code;

      template<CT::BuiltinNumber T> requires (not CT::Character<T>)
      explicit Code(const T&);

      using A::Code::operator ==;

      NOD() LANGULUS_API(FLOW) Many Parse(bool optimize = true) const;

      NOD() LANGULUS_API(FLOW) Code RightOf(Offset) const IF_UNSAFE(noexcept);
      NOD() LANGULUS_API(FLOW) Code LeftOf(Offset) const IF_UNSAFE(noexcept);
      NOD() LANGULUS_API(FLOW) bool StartsWithSpecial() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithSkippable() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithSkippable() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithLetter() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithLetter() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithDigit() const noexcept;
      NOD() LANGULUS_API(FLOW) bool EndsWithDigit() const noexcept;
      NOD() LANGULUS_API(FLOW) bool StartsWithOperator(Offset) const noexcept;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Codifiable<Deint<T>>
      NOD() Code operator + (T&&) const;

      template<class T> requires CT::Codifiable<Deint<T>>
      Code& operator += (T&&);

      template<class T>
      Code& TypeSuffix();

      NOD() LANGULUS_API(FLOW) static bool IsReserved(const Text&);
      NOD() LANGULUS_API(FLOW) static bool IsValidKeyword(const Text&);

   protected:
      /// Parser for unknown expressions                                      
      /// An unknown-expressions will be scanned to figure what it contains   
      struct LANGULUS_API(FLOW) UnknownParser {
         NOD() static Offset Parse(const Code&, Many&, Real, bool optimize);
      };

      /// Parser for keyword expressions                                      
      /// A key-expression is any expression that begins with a letter        
      struct LANGULUS_API(FLOW) KeywordParser {
         NOD() static Offset Parse(const Code&, Many&, bool allowCharge = true);
         NOD() static bool Peek(const Code&) noexcept;
         NOD() static Token Isolate(const Code&) noexcept;
         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            NOD() static AMeta Disambiguate(Offset, const Code&, const Token&);
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
         NOD() static Offset Parse(const Code&, Many&);
         NOD() static bool Peek(const Code&) noexcept;
      };

      /// Parser for operators                                                
      /// An op-expression is one matching the built-in ones, or one matching 
      /// one in reflected verb database, where LHS is not DMeta or VMeta     
      struct LANGULUS_API(FLOW) OperatorParser {
         NOD() static Offset Parse(Operator, const Code&, Many&, Real, bool optimize);
         NOD() static Operator PeekBuiltin(const Code&) noexcept;
         NOD() static Operator Peek(const Code&) noexcept;
         NOD() static Token Isolate(const Code&) noexcept;

      private:
         NOD() static Offset ParseContent(Code::Operator, const Code&, Many&, bool optimize);
         NOD() static Offset ParseString(Code::Operator, const Code&, Many&);
         NOD() static Offset ParseBytes(const Code&, Many&);
         NOD() static Offset ParseKeyword(Code::Operator, const Code&, Many&);
         NOD() static Offset ParsePhase(Code::Operator, Many&);
         NOD() static Offset ParseReflected(Verb&, const Code&, Many&, bool optimize);

         static void InsertContent(Many&, Many&);
      };

      /// Parser for chargers                                                 
      /// A charge-expression is any operator *^@! after a DMeta or VMeta     
      struct LANGULUS_API(FLOW) ChargeParser {
         NOD() static Offset Parse(const Code&, Charge&);
         NOD() static Operator Peek(const Code&) noexcept;
      };
   };

} // namespace Langulus::Flow

namespace Langulus
{
   /// Convenience operator for code string literals                          
   Flow::Code operator ""_code(const char*, ::std::size_t);
}

LANGULUS_DEFINE_CONSTANT(Yes,     true,    "Yes",     "The true boolean value");
LANGULUS_DEFINE_CONSTANT(True,    true,    "True",    "The true boolean value");
LANGULUS_DEFINE_CONSTANT(No,      false,   "No",      "The false boolean value");
LANGULUS_DEFINE_CONSTANT(False,   false,   "False",   "The false boolean value");
LANGULUS_DEFINE_CONSTANT(Null,    nullptr, "Null",    "Nothing, literally");
LANGULUS_DEFINE_CONSTANT(Nothing, nullptr, "Nothing", "Nothing, literally");

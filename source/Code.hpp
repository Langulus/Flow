///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Executor.hpp"
#include "Langulus/RTTI/Definition.hpp"
#include <Langulus/Text.hpp>
#include <Langulus/CT/Charged.hpp>


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
   }

   /// A codifiable type is one that has either an implicit or explicit       
   /// cast operator to Code type, or can be used to explicitly initialize a  
   /// Code container                                                         
   template<class...T>
   concept Codifiable = ((Inner::CodifiableByOperator<T>
        or Inner::CodifiableByConstructor<T>) and ...);
}

namespace Langulus::Flow
{
   ///                                                                        
   ///   Langulus code container, parser, serializer and deserializer         
   ///                                                                        
   struct Code : Annies::Text {
      using CTTI_Named = Yes<"Code">;
      using CTTI_Files = Yes<"flow">;
      using CTTI_Bases = Annies::Text;
      using Operator = Serial::Operator;

      using Annies::Text::Text;

      explicit Code(CT::Number auto const&);

      LANGULUS_API(FLOW) Many Parse(bool optimize = true) const;
      LANGULUS_API(FLOW) Code RightOf(size_t) const assumptious;
      LANGULUS_API(FLOW) Code LeftOf(size_t) const assumptious;
      LANGULUS_API(FLOW) bool StartsWithSpecial() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithSkippable() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithSkippable() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithLetter() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithLetter() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithDigit() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithDigit() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithOperator(size_t) const noexcept;

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Codifiable<Deint<T>>
      Code operator + (T&&) const;

      template<class T> requires CT::Codifiable<Deint<T>>
      Code& operator += (T&&);

      template<class T>
      Code& TypeSuffix();

      LANGULUS_API(FLOW) static bool IsReserved(const Text&);
      LANGULUS_API(FLOW) static bool IsValidKeyword(const Text&);

   protected:
      /// Parser for unknown expressions                                      
      /// An unknown-expressions will be scanned to figure what it contains   
      struct LANGULUS_API(FLOW) UnknownParser {
         static auto Parse(const Code&, Many&, Real, bool optimize) -> size_t;
      };

      /// Parser for keyword expressions                                      
      /// A key-expression is any expression that begins with a letter        
      struct LANGULUS_API(FLOW) KeywordParser {
         static auto Parse(const Code&, Many&, bool allowCharge = true) -> size_t;
         static bool Peek(const Code&) noexcept;
         static auto Isolate(const Code&) noexcept -> Token;
         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            static auto Disambiguate(size_t, const Code&, const Token&) -> RTTI::Inner::Definition const*;
         #endif
      };

      /// Parser for skipping expressions                                     
      /// A skip-expression is any that begins with escapes, tabs, or spaces  
      struct LANGULUS_API(FLOW) SkippedParser {
         static auto Parse(const Code&) -> size_t;
         static bool Peek(const Code&) noexcept;
      };

      /// Parser for number expressions                                       
      /// A num-expression is any that begins with a digit, a minus           
      /// followed by a digit, or a dot followed by a digit                   
      struct LANGULUS_API(FLOW) NumberParser {
         static auto Parse(const Code&, Many&) -> size_t;
         static bool Peek(const Code&) noexcept;
      };

      /// Parser for operators                                                
      /// An op-expression is one matching the built-in ones, or one matching 
      /// one in reflected verb database, where LHS is not DMeta or VMeta     
      struct LANGULUS_API(FLOW) OperatorParser {
         static auto Parse(Operator, const Code&, Many&, Real, bool optimize) -> size_t;
         static auto PeekBuiltin(const Code&) noexcept -> Operator;
         static auto Peek(const Code&) noexcept -> Operator;
         static auto Isolate(const Code&) noexcept -> Token;

      private:
         static auto ParseContent(Operator, const Code&, Many&, bool optimize) -> size_t;
         static auto ParseString(Operator, const Code&, Many&) -> size_t;
         static auto ParseBytes(const Code&, Many&) -> size_t;
         static auto ParseKeyword(Operator, const Code&, Many&) -> size_t;
         static auto ParsePhase(Operator, Many&) -> size_t;
         static auto ParseReflected(Verb&, const Code&, Many&, bool optimize) -> size_t;

         static void InsertContent(Many&, Many&);
      };

      /// Parser for chargers                                                 
      /// A charge-expression is any operator *^@! after a DMeta or VMeta     
      struct LANGULUS_API(FLOW) ChargeParser {
         static auto Parse(const Code&, Charge&) -> size_t;
         static auto Peek(const Code&) noexcept -> Operator;
      };
   };
}

namespace Langulus::CTTI
{
   /// The presence of this structure makes Code a CT::Serializer             
   template<>
   struct Serializer<Flow::Code> : Serializer<Annies::Text> {
      // Code serializer can't be lossy - it's isomorphic               
      static constexpr bool CriticalFailure = true;
      static constexpr bool SkipElements = false;
   };
}

namespace Langulus
{
   /// Convenience operator for code string literals                          
   auto operator ""_code(const char*, ::std::size_t) -> Flow::Code;
}

LANGULUS_DEFINE_CONSTANT(Yes,     true,    "Yes",     "The true boolean value");
LANGULUS_DEFINE_CONSTANT(True,    true,    "True",    "The true boolean value");
LANGULUS_DEFINE_CONSTANT(No,      false,   "No",      "The false boolean value");
LANGULUS_DEFINE_CONSTANT(False,   false,   "False",   "The false boolean value");
LANGULUS_DEFINE_CONSTANT(Null,    nullptr, "Null",    "Nothing, literally");
LANGULUS_DEFINE_CONSTANT(Nothing, nullptr, "Nothing", "Nothing, literally");

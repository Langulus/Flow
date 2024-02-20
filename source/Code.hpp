///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Executor.hpp"


namespace Langulus::Flow
{
   struct Code;
}

namespace Langulus::CT
{
   namespace Inner
   {
   
      /// Do types have an explicit or implicit cast operator to Code         
      template<class...T>
      concept CodifiableByOperator = requires (T&...a) {
         ((a.operator ::Langulus::Flow::Code()), ...); };

      /// Does Code has an explicit/implicit constructor that accepts T       
      template<class...T>
      concept CodifiableByConstructor = requires (T&...a) {
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
      LANGULUS(FILES) "flow";
      LANGULUS_BASES(A::Code);

      /// The presence of this structure makes Code a serializer              
      struct SerializationRules : Text::SerializationRules {
         // Code serializer can't be lossy - it's isomorphic            
         static constexpr bool CriticalFailure = true;
         static constexpr bool SkipElements = false;
      };

      using Operator = SerializationRules::Operator;

      using A::Code::Code;
      using A::Code::operator ==;

      /*Code(const Text&);
      Code(Text&&);*/

      NOD() LANGULUS_API(FLOW) Any Parse(bool optimize = true) const;

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

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      template<class T> requires CT::Codifiable<Desem<T>>
      NOD() Code operator + (T&&) const;

      template<class T> requires CT::Codifiable<Desem<T>>
      Code& operator += (T&&);

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

namespace Langulus
{
   /// Convenience operator for code string literals                          
   Flow::Code operator "" _code(const char*, ::std::size_t);
}
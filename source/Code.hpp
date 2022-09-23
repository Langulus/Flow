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
	///	Langulus code container and parser, as well as keyword database		
	///																								
	class Code : public Text {
		LANGULUS_BASES(Text);
	public:
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
			Sparse,
			Mass,
			Frequency,
			Time,
			Priority,
			OpCounter,
			NoOperator = OpCounter,
			ReflectedOperator,
			ReflectedVerb
		};

		using Text::Text;
		
		explicit Code(Operator);

		Code(Disowned<Code>&&) noexcept;
		Code(Abandoned<Code>&&) noexcept;

		NOD() Scope Parse(bool optimize = true) const;
		NOD() Code Clone() const;

		NOD() Code RightOf(Offset) const;
		NOD() Code LeftOf(Offset) const;
		NOD() bool StartsWithSpecial() const noexcept;
		NOD() bool StartsWithSkippable() const noexcept;
		NOD() bool EndsWithSkippable() const noexcept;
		NOD() bool StartsWithLetter() const noexcept;
		NOD() bool EndsWithLetter() const noexcept;
		NOD() bool StartsWithDigit() const noexcept;
		NOD() bool EndsWithDigit() const noexcept;
		NOD() bool StartsWithOperator(Offset) const noexcept;

		using Text::operator +=;
		Code& operator += (Operator);

		template<class T>
		Code& TypeSuffix();

		NOD() static bool IsReserved(const Text&);
		NOD() static bool IsValidKeyword(const Text&);

	protected:
		/// Parser for unknown expressions													
		/// An unknown-expressions will be scanned to figure what it contains	
		struct UnknownParser {
			NOD() static Offset Parse(const Code&, Any&, Real, bool optimize);
		};

		/// Parser for keyword expressions													
		/// A key-expression is any expression that begins with a letter			
		struct KeywordParser {
			NOD() static Offset Parse(const Code&, Any&, bool allowCharge = true);
			NOD() static bool Peek(const Code&) noexcept;
			NOD() static Token Isolate(const Code&) noexcept;
		};

		/// Parser for skipping expressions													
		/// A skip-expression is any that begins with escapes, tabs, or spaces	
		struct SkippedParser {
			NOD() static Offset Parse(const Code&);
			NOD() static bool Peek(const Code&) noexcept;
		};

		/// Parser for number expressions													
		/// A num-expression is any that begins with a digit, a minus				
		/// followed by a digit, or a dot followed by a digit							
		struct NumberParser {
			NOD() static Offset Parse(const Code&, Any&);
			NOD() static bool Peek(const Code&) noexcept;
		};

		/// Parser for operators																
		/// An op-expression is one matching the built-in ones, or one matching	
		/// one in reflected verb database, where LHS is not DMeta or VMeta		
		struct OperatorParser {
			NOD() static Offset Parse(Operator, const Code&, Any&, Real, bool optimize);
			NOD() static Operator PeekBuiltin(const Code&) noexcept;
			NOD() static Operator Peek(const Code&) noexcept;
			NOD() static Token Isolate(const Code&) noexcept;

		private:
			NOD() static Offset ParseContent(const Code&, Any&, bool optimize);
			NOD() static Offset ParseString(Code::Operator, const Code&, Any&);
			NOD() static Offset ParseBytes(const Code&, Any&);
			NOD() static Offset ParsePhase(Code::Operator, const Code&, Any&, bool optimize);
			NOD() static Offset ParseConst(const Code&, Any&, bool optimize);
			NOD() static Offset ParseSparse(const Code&, Any&, bool optimize);
			NOD() static Offset ParseReflected(Verb&, const Code&, Any&, bool optimize);

			static void InsertContent(Any&, Any&);
		};

		/// Parser for chargers																	
		/// A charge-expression is any operator *^@! after a DMeta or VMeta		
		struct ChargeParser {
			NOD() static Offset Parse(const Code&, Charge&);
			NOD() static Operator Peek(const Code&) noexcept;
		};

		struct OperatorProperties {
			Token mToken;
			Real mPrecedence;
			bool mCharge;
		};

		/// Built-in operators database														
		static const OperatorProperties mOperators[OpCounter];
	};

} // namespace Langulus::Flow

#include "Code.inl"

namespace Langulus::Verbs
{

	using ::Langulus::Flow::Scope;
	using ::Langulus::Flow::Construct;

} // namespace Langulus::Verbs

namespace Langulus
{
	Flow::Code operator "" _code(const char*, ::std::size_t);
}
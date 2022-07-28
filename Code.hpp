#pragma once
#include "Scope.hpp"
#include "Construct.hpp"

namespace Langulus::Flow
{

	///																								
	///	Langulus code container and parser, as well as keyword database		
	///																								
	class Code : public Text {
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
			Past,
			Future,
			//Context,
			Associate,
			Missing,
			And,
			Or,
			Select,
			Mass,
			Frequency,
			Time,
			Priority,
			Add,
			Subtract,
			Multiply,
			Divide,
			Power,
			//As,
			OpenByte,
			CloseByte,

			OpCounter
		};

	protected:
		/// Parser for unknown expressions													
		/// An unknown-expressions will be scanned to figure what it contains	
		struct UnknownParser {
			NOD() static Offset Parse(const Code&, Any&, int priority, bool optimize);
		};

		/// Parser for keyword expressions													
		/// A key-expression is any expression that begins with a letter			
		struct KeywordParser {
			NOD() static Offset Parse(const Code&, Any&);
			NOD() static bool Peek(const Code&) noexcept;
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
			NOD() static Offset Parse(const Code&, Any&, int priority, bool optimize);
			NOD() static bool Peek(const Code&) noexcept;

			NOD() static Offset ParseContent(const Code&, Any&, bool optimize);
			NOD() static Offset ParseString(Code::Operator, const Code&, Any&);
			NOD() static Offset ParsePolarize(Code::Operator, const Code&, Any&, bool optimize);
			NOD() static Offset ParseMissing(const Code&, Any&);

			static void InsertContent(Any&, Any&);
		};

		/// Parser for chargers																	
		/// A charge-expression is any operator *^@! after a DMeta or VMeta		
		struct ChargeParser {
			NOD() static Offset Parse(const Code&, Any&, int priority, bool optimize);
			NOD() static bool Peek(const Code&) noexcept;
			NOD() static bool IsChargable(const Any&) noexcept;
		};

		struct MetaConstant {
			Token mToken;
			Any mValue;
		};

		static THashMap<Text, MetaConstant> mConstants;

		struct OperatorProperties {
			Token mToken;
			Token mTokenWithSpacing;
			int mPriority;
			bool mCharge;
		};

		static const OperatorProperties mOperators[OpCounter];

	public:
		using Text::Text;
		explicit Code(Operator);

		NOD() Any Parse(bool optimize = true) const;
		NOD() Code Clone() const;

		NOD() Code LeftOf(Offset) const;
		NOD() Code RightOf(Offset) const;
		NOD() bool StartsWithSpecial() const noexcept;
		NOD() bool StartsWithSkippable() const noexcept;
		NOD() bool EndsWithSkippable() const noexcept;
		NOD() bool StartsWithLetter() const noexcept;
		NOD() bool EndsWithLetter() const noexcept;
		NOD() bool StartsWithDigit() const noexcept;
		NOD() bool EndsWithDigit() const noexcept;
		NOD() bool StartsWithOperator(Offset) const noexcept;

		template<class T>
		Code& TypeSuffix();

		NOD() Code StandardToken() const;

		NOD() static bool IsReserved(const Text&);
		NOD() static bool IsValidKeyword(const Text&);
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
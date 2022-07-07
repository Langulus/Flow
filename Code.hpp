#pragma once
#include "Scope.hpp"
#include "Construct.hpp"

namespace Langulus::Flow
{

	///																								
	///	CODE CONTAINER AND PARSER															
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
			PolarizeLeft,
			PolarizeRight,
			//Context,
			Copy,
			Missing,
			AndSeparator,
			OrSeparator,
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

		struct TokenProperties {
			Token mToken;
			Token mTokenWithSpacing;
			int mPriority;
			bool mCharge;
		};

		static const TokenProperties Token[OpCounter];

		using Text::Text;

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

	Code operator "" _code(const char*, ::std::size_t);

} // namespace Langulus::Flow

#include "Code.inl"

namespace Langulus::Verbs
{

	using ::Langulus::Flow::Scope;
	using ::Langulus::Flow::Construct;

} // namespace Langulus::Verbs
#pragma once
#include "Verb.hpp"
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
			Context,
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
			As,
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

		/*Code(const Text&);
		Code(Text&&) noexcept;

		explicit Code(const Charge&);
		explicit Code(const Verb&);
		explicit Code(const Map&);
		explicit Code(const Hash&);
		explicit Code(Code::Operator);*/

		NOD() Any Parse(bool optimize = true) const;
		NOD() Code Clone() const;

		/*template<class ANYTHING>
		Code& operator += (const ANYTHING&);*/

		NOD() Code CropLeft(Offset) const;
		NOD() Code CropRight(Offset) const;
		NOD() bool IsSkippable() const noexcept;
		NOD() bool IsSkippableRev() const noexcept;
		NOD() bool IsLetter() const noexcept;
		NOD() bool IsLetterRev() const noexcept;
		NOD() bool IsNumber() const noexcept;
		NOD() bool IsNumberRev() const noexcept;
		NOD() bool IsOperator(Offset) const noexcept;

		template<class T>
		Code& TypeSuffix();

		NOD() Code StandardToken() const;

		NOD() static bool IsReserved(const Text&);
		NOD() static bool IsValidKeyword(const Text&);
	};

	/*NOD() Code operator + (const Code&, const Code&);
	NOD() Code operator + (const Code&, const Text&);
	NOD() Code operator + (const Text&, const Code&);

	template<CT::NotText T>
	NOD() Code operator + (const T&, const Code&);

	template<CT::NotText T>
	NOD() Code operator + (const Code&, const T&);*/

	Code operator "" _code(const char*, ::std::size_t);

} // namespace Langulus::Flow

#include "Code.inl"

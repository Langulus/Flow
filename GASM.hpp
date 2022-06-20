#pragma once
#include "Verb.hpp"
#include "Construct.hpp"

LANGULUS_DECLARE_EXCEPTION(GASM);

namespace PCFW::Flow
{

	///																								
	///	GASM CODE CONTAINER AND PARSER													
	///																								
	class LANGULUS_MODULE(FLOW) GASM : public Text {
		REFLECT(GASM);
	public:
		enum Operator : pcptr {
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
			LiteralText mToken;
			LiteralText mTokenWithSpacing;
			int mPriority;
			bool mCharge;
		};

		static const TokenProperties Token[OpCounter];

		using Text::Text;

		GASM(const Text&);
		GASM(Text&&) noexcept;

		explicit GASM(const Charge&);
		explicit GASM(const ChargedVerbID&);
		explicit GASM(const Map&);
		explicit GASM(const Hash&);
		explicit GASM(GASM::Operator);

		NOD() Any Parse(bool optimize = true) const;
		NOD() GASM Clone() const;

		template<class ANYTHING>
		GASM& operator += (const ANYTHING&);

		NOD() GASM CropLeft(pcptr) const;
		NOD() GASM CropRight(pcptr) const;
		NOD() bool IsSkippable() const noexcept;
		NOD() bool IsSkippableRev() const noexcept;
		NOD() bool IsLetter() const noexcept;
		NOD() bool IsLetterRev() const noexcept;
		NOD() bool IsNumber() const noexcept;
		NOD() bool IsNumberRev() const noexcept;
		NOD() bool IsOperator(pcptr) const noexcept;

		template<class T>
		GASM& TypeSuffix();

		NOD() GASM StandardToken() const;

		NOD() static bool IsReserved(const Text&);
		NOD() static bool IsValidKeyword(const Text&);
	};

	NOD() GASM operator + (const GASM&, const GASM&);
	NOD() GASM operator + (const GASM&, const Text&);
	NOD() GASM operator + (const Text&, const GASM&);

	template<class T>
	NOD() GASM operator + (const T&, const GASM&) requires (!IsText<T>);

	template<class T>
	NOD() GASM operator + (const GASM&, const T&) requires (!IsText<T>);

	GASM operator "" _gasm(const char*, std::size_t);

} // namespace PCFW::PCGASM

namespace PCFW
{

	/// Explicit specialization for making code not deep								
	template<> constexpr bool pcIsDeep<Flow::GASM> = pcIsDeep<Memory::Text>;
	template<> constexpr bool pcIsDeep<Flow::GASM*> = pcIsDeep<Memory::Text>;
	template<> constexpr bool pcIsDeep<const Flow::GASM*> = pcIsDeep<Memory::Text>;

}

#include "GASM.inl"

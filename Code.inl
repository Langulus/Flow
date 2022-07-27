#pragma once
#include "Code.hpp"

namespace Langulus::Flow
{

	/// Construct by referencing a Text container										
	///	@param other - the text container to use										
	/*inline Code::Code(const Text& other)
		: Text {other} {}

	/// Construct by moving a Text container												
	///	@param other - the text container to use										
	inline Code::Code(Text&& other) noexcept
		: Text {Forward<Text>(other)} {}*/

	/// Remove elements from the left side of Code code								
	///	@param offset - the number of elements to discard from the front		
	///	@return a shallow-copied container with the correct offset				
	inline Code Code::LeftOf(Offset o) const {
		return Code {Text::Crop(o, mCount - o)};
	}

	/// Remove elements from the right side of Code code								
	///	@param offset - the number of elements to remain in container			
	///	@return a shallow-copied container with the correct offset				
	inline Code Code::RightOf(Offset o) const {
		return Code {Text::Crop(0, o)};
	}

	/// Check if the Code container begins with special elements, such as		
	/// special characters or escape sequences, like colors							
	///	@return true if the first symbol is special									
	inline bool Code::StartsWithSpecial() const noexcept {
		const auto& letter = (*this)[0];
		return GetCount() > 0 && letter > 0 && letter < 32;
	}

	/// Check if the Code container begins with skippable elements, such as		
	/// tabs or spaces, or special characters/sequences								
	///	@return true if the first symbol is a spacer									
	inline bool Code::StartsWithSkippable() const noexcept {
		const auto& letter = (*this)[0];
		return GetCount() > 0 && letter > 0 && letter <= 32;
	}

	/// Check if the Code code container begins with skippable elements			
	///	@return true if the first symbol is a spacer									
	inline bool Code::EndsWithSkippable() const noexcept {
		return GetCount() > 0 && last() > 0 && last() <= 32;
	}

	/// Check if the Code code container begins with a letter or underscore		
	///	@return true if the first symbol is a letter/underscore					
	inline bool Code::StartsWithLetter() const noexcept {
		return GetCount() > 0 && (::std::isalpha((*this)[0]) || (*this)[0] == '_');
	}

	/// Check if the Code code container ends with a letter or underscore		
	///	@return true if the last symbol is a letter/underscore					
	inline bool Code::EndsWithLetter() const noexcept {
		return GetCount() > 0 && (::std::isalpha(last()) || last() == '_');
	}

	/// Check if the Code code container begins with a number						
	///	@return true if the first symbol is a number									
	inline bool Code::StartsWithDigit() const noexcept {
		return GetCount() > 0 && ::std::isdigit((*this)[0]);
	}

	/// Check if the Code code container ends with a number							
	///	@return true if the last symbol is a number									
	inline bool Code::EndsWithDigit() const noexcept {
		return GetCount() > 0 && ::std::isdigit(last());
	}

	/// Check if the Code code container begins with an operator					
	///	@param i - the operator to check for											
	///	@return true if the operator matches											
	inline bool Code::StartsWithOperator(Offset i) const noexcept {
		const Size tokenSize = Code::Token[i].mToken.size();
		if (!tokenSize || mCount < tokenSize)
			return false;

		const auto token = Code(Code::Token[i].mToken);
		const auto remainder = LeftOf(tokenSize);
		const auto endsWithALetter = token.EndsWithLetter();
		return tokenSize > 0 && MatchesLoose(token) == tokenSize
			&& (GetCount() == tokenSize 
				|| (endsWithALetter && (!remainder.StartsWithLetter() && !remainder.StartsWithDigit()))
				|| !endsWithALetter
			);
	}

	/// A type naming convention for standard number types							
	///	@return the suffix depending on the template argument						
	template<class T>
	Code& Code::TypeSuffix() {
		if constexpr (CT::UnsignedInteger<T>) {
			*this += "u";
			if constexpr (sizeof(T) * 8 != 32)
				*this += sizeof(T) * 8;
		}
		else if constexpr (CT::SignedInteger<T>) {
			*this += "i";
			if constexpr (sizeof(T) * 8 != 32)
				*this += sizeof(T) * 8;
		}
		else if constexpr (CT::Same<T, float>)
			*this += "f";
		else if constexpr (CT::Same<T, double>)
			*this += "d";
		else if constexpr (CT::Bool<T>)
			*this += "b";
		else
			*this += MetaData::Of<T>();
		return *this;
	}

	/// Generate a standard token from the current container							
	///	@return the token																		
	inline Code Code::StandardToken() const {
		Code result = *this;
		result += ",";
		result += *this;
		result += "Ptr,";
		result += *this;
		result += "ConstPtr";
		return result;
	}

	/// Concatenate Code with Code															
	inline Code operator + (const Code& lhs, const Code& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains Code type		
		return Code {static_cast<const Text&>(lhs) + static_cast<const Text&>(rhs)};
	}

	/// Concatenate Text with Code, Code always dominates								
	inline Code operator + (const Text& lhs, const Code& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains Code type		
		return Code {lhs + static_cast<const Text&>(rhs)};
	}

	/// Concatenate Code with Text, Code always dominates								
	inline Code operator + (const Code& lhs, const Text& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains Code type		
		return Code {static_cast<const Text&>(lhs) + rhs};
	}

	/// Destructive concatenation of Code with anything								
	/*template<class ANYTHING>
	Code& Code::operator += (const ANYTHING& rhs) {
		if constexpr (IsText<ANYTHING>) {
			Text::template operator += <Text>(static_cast<const Text&>(rhs));
		}
		else {
			Code converted;
			TConverter<ANYTHING, Code>::Convert(rhs, converted);
			operator += (converted);
		}
		return *this;
	}

	/// Concatenate anything with Code														
	template<CT::NotText T>
	NOD() Code operator + (const T& lhs, const Code& rhs) {
		Code converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// Concatenate Code with anything														
	template<CT::NotText T>
	NOD() Code operator + (const Code& lhs, const T& rhs) {
		Code converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}*/

} // namespace Langulus::Flow

namespace Langulus
{

	/// Make a code literal																		
	inline Flow::Code operator "" _code(const char* text, ::std::size_t size) {
		return Anyness::Text {text, size};
	}

}
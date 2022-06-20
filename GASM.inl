namespace PCFW::Flow
{

	/// Construct by referencing a Text container										
	///	@param other - the text container to use										
	inline GASM::GASM(const Text& other)
		: Text {other} {}

	/// Construct by moving a Text container												
	///	@param other - the text container to use										
	inline GASM::GASM(Text&& other) noexcept
		: Text {pcForward<Text>(other)} {}

	/// Remove elements from the left side of GASM code								
	///	@param offset - the number of elements to discard from the front		
	///	@return a shallow-copied container with the correct offset				
	inline GASM GASM::CropLeft(pcptr offset) const {
		return Text::Crop(offset, mCount - offset);
	}

	/// Remove elements from the right side of GASM code								
	///	@param offset - the number of elements to remain in container			
	///	@return a shallow-copied container with the correct offset				
	inline GASM GASM::CropRight(pcptr offset) const {
		return Text::Crop(0, offset);
	}

	/// Check if the GASM code container begins with skippable elements			
	///	@return true if the first symbol is a spacer									
	inline bool GASM::IsSkippable() const noexcept {
		return GetCount() > 0 && pcIsSpecialChar((*this)[0]);
	}

	/// Check if the GASM code container begins with skippable elements			
	///	@return true if the first symbol is a spacer									
	inline bool GASM::IsSkippableRev() const noexcept {
		return GetCount() > 0 && last() > 0 && last() <= 32;
	}

	/// Check if the GASM code container begins with a letter or underscore		
	///	@return true if the first symbol is a letter/underscore					
	inline bool GASM::IsLetter() const noexcept {
		return GetCount() > 0 && (pcIsLetter((*this)[0]) || (*this)[0] == '_');
	}

	/// Check if the GASM code container ends with a letter or underscore		
	///	@return true if the last symbol is a letter/underscore					
	inline bool GASM::IsLetterRev() const noexcept {
		return GetCount() > 0 && (pcIsLetter(last()) || last() == '_');
	}

	/// Check if the GASM code container begins with a number						
	///	@return true if the first symbol is a number									
	inline bool GASM::IsNumber() const noexcept {
		return GetCount() > 0 && pcIsNumber((*this)[0]);
	}

	/// Check if the GASM code container ends with a number							
	///	@return true if the last symbol is a number									
	inline bool GASM::IsNumberRev() const noexcept {
		return GetCount() > 0 && pcIsNumber(last());
	}

	/// Check if the GASM code container begins with an operator					
	///	@param i - the operator to check for											
	///	@return true if the operator matches											
	inline bool GASM::IsOperator(pcptr i) const noexcept {
		const pcptr tokenSize = GASM::Token[i].mToken.size();
		if (mCount < tokenSize)
			return false;
		const auto token = GASM(GASM::Token[i].mToken);
		const auto remainder = CropLeft(tokenSize);
		const auto endsWithALetter = token.IsLetterRev();
		return tokenSize > 0 && MatchesLoose(token) == tokenSize
			&& (GetCount() == tokenSize 
				|| (endsWithALetter && (!remainder.IsLetter() && !remainder.IsNumber()))
				|| !endsWithALetter
			);
	}

	/// A type naming convention for standard number types							
	///	@return the suffix depending on the template argument						
	template<class T>
	GASM& GASM::TypeSuffix() {
		if constexpr (UnsignedInteger<T>) {
			*this += "u";
			if constexpr (sizeof(T) * 8 != 32)
				*this += sizeof(T) * 8;
		}
		else if constexpr (SignedInteger<T>) {
			*this += "i";
			if constexpr (sizeof(T) * 8 != 32)
				*this += sizeof(T) * 8;
		}
		else if constexpr (Same<T, pcr32>)
			*this += "f";
		else if constexpr (Same<T, pcr64>)
			*this += "d";
		else if constexpr (Boolean<T>)
			*this += "b";
		else
			*this += DataID::Of<T>;
		return *this;
	}

	/// Generate a standard token from the current container							
	///	@return the token																		
	inline GASM GASM::StandardToken() const {
		GASM result = *this;
		result += ",";
		result += *this;
		result += "Ptr,";
		result += *this;
		result += "ConstPtr";
		return result;
	}

	/// Concatenate GASM with GASM															
	inline GASM operator + (const GASM& lhs, const GASM& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains GASM type		
		return static_cast<const Text&>(lhs) + static_cast<const Text&>(rhs);
	}

	/// Concatenate Text with GASM, GASM always dominates								
	inline GASM operator + (const Text& lhs, const GASM& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains GASM type		
		return lhs + static_cast<const Text&>(rhs);
	}

	/// Concatenate GASM with Text, GASM always dominates								
	inline GASM operator + (const GASM& lhs, const Text& rhs) {
		// It's essentially the same, as concatenating Text with Text		
		// with the only difference being, that it retains GASM type		
		return static_cast<const Text&>(lhs) + rhs;
	}

	/// Destructive concatenation of GASM with anything								
	template<class ANYTHING>
	GASM& GASM::operator += (const ANYTHING& rhs) {
		if constexpr (IsText<ANYTHING>) {
			Text::template operator += <Text>(static_cast<const Text&>(rhs));
		}
		else {
			GASM converted;
			TConverter<ANYTHING, GASM>::Convert(rhs, converted);
			operator += (converted);
		}
		return *this;
	}

	/// Concatenate anything with GASM														
	template<class T>
	NOD() GASM operator + (const T& lhs, const GASM& rhs) requires (!IsText<T>) {
		GASM converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	/// Concatenate GASM with anything														
	template<class T>
	NOD() GASM operator + (const GASM& lhs, const T& rhs) requires (!IsText<T>) {
		GASM converted;
		converted += lhs;
		converted += rhs;
		return converted;
	}

	inline GASM operator "" _gasm(const char* text, std::size_t size) {
		return GASM{ text, size };
	}

} // namespace PCFW::PCGASM


namespace PCFW::Memory
{

	using Verb = Flow::Verb;
	using Construct = Flow::Construct;

	/// Employ TConverter and verbs to define the Block's high level AsCast		
	/// Get templated element by casting it automatically if type mismatches	
	/// Checks range, density, type and special indices, and converts				
	/// This is the slowest access function												
	/// It attempts to cast only one element												
	///	@param idx - the complex index to use											
	///	@return a shallow copy of the element											
	template<RTTI::ReflectedData T, bool FATAL_FAILURE>
	T Block::AsCast(Index idx) const {
		static_assert(DefaultConstructible<T>,
			"T must be default-constructible in order to use AsCast");

		// Constrain the index															
		idx = Constrain(idx);
		if (idx.IsSpecial()) {
			throw Except::BadAccess(pcLogFuncError
				<< "Can't reference special index " << idx);
		}

		const auto idxu = pcptr(idx.mIndex);
		try {
			// Try directly interfacing, doing a shallow copy					
			return As<T>(idxu);
		}
		catch (const Except::BadAccess&) {
			// ... or if failed, try conversion										
			// if conversion failed, a default constructed instance will	
			// be returned																	
			auto interpreter = Verb::From<Verbs::Interpret>({}, DataID::Reflect<T>());
			Any element {GetElement(idxu)};
			if (Verb::ExecuteVerb(element, interpreter))
				return interpreter.GetOutput().template As<T>();

			// If reached, then failure - return default-constructed			
			// Or throw if failure was fatal											
			if constexpr (FATAL_FAILURE)
				throw Except::BadAccess(pcLogFuncError << "Conversion failed");
			else return {};
		}
	}

	/// Employ TConverter and verbs to define the Block's high level AsCast		
	/// Get templated element by casting it automatically if type mismatches	
	/// Checks range, density, type and special indices, and converts				
	/// This is the slowest access function												
	/// It attempts to cast all elements at once to the desired type				
	///	@return a shallow copy of the element											
	template<RTTI::ReflectedData T, bool FATAL_FAILURE>
	T Block::AsCast() const {
		static_assert(DefaultConstructible<T>,
			"T must be default-constructible in order to use AsCast");

		try {
			// Try directly interfacing, doing a shallow copy					
			return As<T>();
		}
		catch (const Except::BadAccess&) {
			// Can't directly access, we must create the result				
			// Make sure we're creating something concrete						
			auto meta = DataID::Reflect<T>();
			meta = meta->GetConcreteMeta();
			if (GetCountElementsDeep() == 1) {
				// Convert single argument to requested type						
				// If a direct copy is available it will be utilized			
				auto interpreter = Verb::From<Verbs::Interpret>({}, meta);
				if (Verb::DispatchDeep(const_cast<Block&>(*this), interpreter) && !interpreter.GetOutput().IsEmpty()) {
					// Success																
					return interpreter.GetOutput().template As<T>();
				}
			}
		
			// Either Interpret verb didn't do the trick, or multiple		
			// items were provided, so we need to inspect members, and		
			// satisfy them one by one													
			const auto concreteConstruct = Construct::From(meta, Any{*this});
			auto creator = Verb::From<Verbs::Create>({}, &concreteConstruct);
			if (!Verb::DispatchEmpty(creator) || creator.GetOutput().IsEmpty()) {
				// Failure																	
				if constexpr (FATAL_FAILURE) {
					throw Except::BadConstruction(pcLogFuncError
						<< "Can't construct " << meta->GetToken()
						<< " from " << concreteConstruct);
				}
				else return {};
			}

			// Success																		
			return creator.GetOutput().template As<T>();


			// ... or if failed, try conversion										
			// if conversion failed, a default constructed instance will	
			// be returned																	
			/*auto interpreter = Verb::From<Verbs::Interpret>({}, DataID::Reflect<T>());
			Any wrapped {*this};
			if (Verb::ExecuteVerb(wrapped, interpreter))
				return interpreter.GetOutput().template As<T>();

			// If reached, then failure - return default-constructed			
			// Or throw if failure was fatal											
			if constexpr (FATAL_FAILURE)
				throw Except::BadAccess(pcLogFuncError << "Conversion failed");
			else return {};*/
		}
	}

} // namespace PCFW::Memory
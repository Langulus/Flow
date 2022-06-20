namespace PCFW::Flow
{

	/// Create content descriptor from a type and include a constructor			
	///	@param type - the type we're constructing										
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<RTTI::ReflectedData DATA>
	Construct Construct::From(DMeta type, DATA&& arguments) {
		return Construct(type) << pcForward<DATA>(arguments);
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param type - the type we're constructing										
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<RTTI::ReflectedData DATA>
	Construct Construct::From(DMeta type, const DATA& arguments) {
		return Construct(type) << arguments;
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<RTTI::ReflectedData T, RTTI::ReflectedData DATA>
	Construct Construct::From(DATA&& arguments) {
		return Construct::From<DATA>(DataID::Reflect<T>(), pcForward<DATA>(arguments));
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<RTTI::ReflectedData T, RTTI::ReflectedData DATA>
	Construct Construct::From(const DATA& arguments) {
		return Construct::From<DATA>(DataID::Reflect<T>(), arguments);
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<RTTI::ReflectedData T>
	Construct Construct::From() {
		return Construct(DataID::Reflect<T>());
	}

	inline bool Construct::operator != (const Construct& other) const noexcept {
		return !(*this == other);
	}

	/// Check if contained data can be interpreted as a given type					
	/// Interpreting means reading compatiblity											
	template<RTTI::ReflectedData T>
	bool Construct::InterpretsAs() const {
		return InterpretsAs(DataID::Reflect<T>());
	}

	/// Check if contained data fully matches a given type							
	template<RTTI::ReflectedData T>
	bool Construct::Is() const {
		return Is(DataID::Of<pcDecay<T>>);
	}

	inline const Any& Construct::GetAll() const noexcept {
		return mArguments;
	}

	inline Any& Construct::GetAll() noexcept {
		return mArguments;
	}

	inline const Charge& Construct::GetCharge() const noexcept {
		return mCharge;
	}

	inline Charge& Construct::GetCharge() noexcept {
		return mCharge;
	}

	inline DMeta Construct::GetMeta() const noexcept {
		return mHeader;
	}

	inline bool Construct::IsEmpty() const noexcept {
		return mArguments.IsEmpty();
	}

	template<RTTI::ReflectedData T>
	Construct Construct::CloneAs() const {
		return Clone(DataID::Reflect<T>());
	}

	/// Copy items to the construct															
	///	@param whatever - the thing you wish to push									
	template<RTTI::ReflectedData T>
	Construct& Construct::operator << (const T& whatever) {
		if (mArguments.SmartPush<Any>(whatever))
			ResetHash();
		return *this;
	}

	/// Merge items to the construct's arguments											
	///	@param whatever - the thing you wish to push									
	template<RTTI::ReflectedData T>
	Construct& Construct::operator <<= (const T& whatever) {
		if constexpr (Same<T, Trait>)
			return Set(pcVal(whatever));
		else {
			if (mArguments.FindDeep(whatever) == uiNone)
				*this << whatever;
			return *this;
		}
	}

	/// Get traits from constructor															
	///	@return pointer to found traits or nullptr if none found					
	template<RTTI::ReflectedTrait TRAIT>
	const Trait* Construct::Get(const pcptr& index) const {
		return Get(TRAIT::Reflect(), index);
	}

} // namespace PCFW::PCGASM

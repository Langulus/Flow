namespace PCFW::Flow
{

	/// Default construction																	
	constexpr Charge::Charge(real mass, real freq, real time, real prio) noexcept
		: mMass {mass}
		, mFrequency {freq}
		, mTime {time}
		, mPriority {prio} {}

	/// Compare charges																			
	constexpr bool Charge::operator == (const Charge& ext) const noexcept {
		return pcNear(mMass, ext.mMass)
			&& pcNear(mFrequency, ext.mFrequency)
			&& pcNear(mTime, ext.mTime)
			&& pcNear(mPriority, ext.mPriority);
	}

	constexpr bool Charge::operator != (const Charge& ext) const noexcept {
		return !(*this == ext);
	}

	/// Check if charge is default															
	constexpr bool Charge::IsDefault() const noexcept {
		return mMass == DefaultMass && 
			mFrequency == DefaultFrequency && 
			mTime == DefaultTime && 
			mPriority == DefaultPriority;
	}

	/// Get the hash of the charge															
	inline Hash Charge::GetHash() const noexcept {
		return pcHash(mMass) | pcHash(mFrequency) | pcHash(mTime) | pcHash(mPriority);
	}

	/// Default construction																	
	///	@param id - the verb																	
	///	@param charge - the verb charge													
	///	@param shortCircuited - whether or not to short-circuit the verb		
	constexpr ChargedVerbID::ChargedVerbID(VMeta id, Charge charge, bool shortCircuited) noexcept
		: mID {id}
		, mCharge {charge}
		, mShortCircuited {shortCircuited} {}

	/// Compare charged verbs for equality													
	///	@param ext - the charged verb to compare against							
	///	@return true if charged verbs match												
	constexpr bool ChargedVerbID::operator == (const ChargedVerbID& ext) const noexcept {
		return mID == ext.mID && mCharge == ext.mCharge && mShortCircuited == ext.mShortCircuited;
	}

	/// Compare charged verbs for inequality												
	///	@param ext - the charged verb to compare against							
	///	@return true if charged verbs do not match									
	constexpr bool ChargedVerbID::operator != (const ChargedVerbID& ext) const noexcept {
		return mID != ext.mID || mCharge != ext.mCharge || mShortCircuited != ext.mShortCircuited;
	}

	/// Get the hash of the charged verb													
	///	@return the hash value																
	inline Hash ChargedVerbID::GetHash() const noexcept {
		return mID->GetID().GetHash() | mCharge.GetHash() | pcHash(mShortCircuited);
	}

	/// Create a verb using a static verb type											
	///	@tparam T - the verb to use														
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	///	@return the verb																		
	template<RTTI::ReflectedVerb T>
	Verb Verb::From(const Any& s, const Any& a, const Any& o) {
		return {T::ID, s, a, o};
	}

	/// Check if verb is a specific type													
	///	@tparam T - the verb to compare against										
	///	@return true if verbs match														
	template<RTTI::ReflectedVerb T>
	bool Verb::Is() const noexcept {
		return Is(T::ID);
	}

	/// Check if verb has been satisfied at least once									
	///	@return true if verb has been satisfied at least once						
	inline bool Verb::IsDone() const noexcept {
		return mSuccesses > 0;
	}

	/// Satisfy verb once																		
	inline void Verb::Done() noexcept {
		++mSuccesses;
	}

	/// Reset verb satisfaction, clear output 											
	inline void Verb::Undo() noexcept {
		mSuccesses = 0;
		mOutput.Reset();
	}

	/// Invert the verb (use the antonym)													
	///	@return a reference to self														
	inline Verb& Verb::Invert() noexcept {
		mVerb.mCharge.mMass *= real(-1);
		return *this;
	}

	/// Set the verb ID																			
	///	@param call - the verb ID to assign												
	///	@return a reference to self														
	inline Verb& Verb::SetVerb(const VerbID& call) noexcept {
		mVerb.mID = call.GetMeta();
		return *this;
	}

	/// Set the verb mass																		
	///	@param energy - the mass to set													
	///	@return a reference to self														
	inline Verb& Verb::SetMass(const real energy) noexcept {
		mVerb.mCharge.mMass = energy;
		return *this;
	}

	/// Set the verb frequency																	
	///	@param energy - the frequency to set											
	///	@return a reference to self														
	inline Verb& Verb::SetFrequency(const real energy) noexcept {
		mVerb.mCharge.mFrequency = energy;
		return *this;
	}

	/// Set the verb time																		
	///	@param energy - the time to set													
	///	@return a reference to self														
	inline Verb& Verb::SetTime(const real energy) noexcept {
		mVerb.mCharge.mTime = energy;
		return *this;
	}

	/// Set the verb priority																	
	///	@param energy - the priority to set												
	///	@return a reference to self														
	inline Verb& Verb::SetPriority(const real energy) noexcept {
		mVerb.mCharge.mPriority = energy;
		return *this;
	}

	/// Set the verb mass, frequency, time, and priority (a.k.a. charge)			
	///	@param charge - the charge to set												
	///	@return a reference to self														
	inline Verb& Verb::SetCharge(const Charge& charge) noexcept {
		mVerb.mCharge = charge;
		return *this;
	}

	/// Set the verb source by shallow copy												
	///	@param source - the source to set												
	///	@return a reference to self														
	inline Verb& Verb::SetSource(const Any& source) {
		mSource = source;
		return *this;
	}

	/// Set the verb source by moving														
	///	@param source - the source to set												
	///	@return a reference to self														
	inline Verb& Verb::SetSource(Any&& source) noexcept {
		mSource = pcForward<Any>(source);
		return *this;
	}

	/// Set the verb argument by shallow copy												
	///	@param argument - the argument to set											
	///	@return a reference to self														
	inline Verb& Verb::SetArgument(const Any& argument) {
		mArgument = argument;
		return *this;
	}

	/// Set the verb argument by moving														
	///	@param argument - the argument to set											
	///	@return a reference to self														
	inline Verb& Verb::SetArgument(Any&& argument) noexcept {
		mArgument = pcForward<Any>(argument);
		return *this;
	}

	/// Set the verb output by shallow copy												
	///	@param output - the output to set												
	///	@return a reference to self														
	inline Verb& Verb::SetOutput(const Any& output) {
		mOutput = output;
		return *this;
	}

	/// Set the verb output by moving														
	///	@param output - the output to set												
	///	@return a reference to self														
	inline Verb& Verb::SetOutput(Any&& output) noexcept {
		mOutput = pcForward<Any>(output);
		return *this;
	}

	/// Set source, argument, and output													
	///	@param s - source																		
	///	@param a - argument																	
	///	@param o - output																		
	///	@return a reference to self														
	inline Verb& Verb::SetAll(const Any& s, const Any& a, const Any& o) {
		mSource = s;
		mArgument = a;
		mOutput = o;
		return *this;
	}

	/// Check if verb is satisfied at least once											
	///	@param rhs - flag to compare against											
	///	@return true if verb satisfaction matches rhs								
	inline bool Verb::operator == (const bool rhs) const noexcept {
		return IsDone() == rhs; 
	}

	/// Check if verb is not satisfied 														
	///	@param rhs - flag to compare against											
	///	@return true if verb satisfaction doesn't match rhs						
	inline bool Verb::operator != (const bool rhs) const noexcept {
		return IsDone() != rhs; 
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has larger or equal priority							
	inline bool Verb::operator < (const Verb& ext) const noexcept {
		return mVerb.mCharge.mPriority < ext.mVerb.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller or equal priority							
	inline bool Verb::operator > (const Verb& ext) const noexcept {
		return mVerb.mCharge.mPriority > ext.mVerb.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller priority										
	inline bool Verb::operator >= (const Verb& ext) const noexcept {
		return mVerb.mCharge.mPriority >= ext.mVerb.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has larger priority										
	inline bool Verb::operator <= (const Verb& rhs) const noexcept {
		return mVerb.mCharge.mPriority <= rhs.mVerb.mCharge.mPriority;
	}

	/// Get the verb id																			
	///	@return verb ID																		
	inline VerbID Verb::GetID() const noexcept {
		return mVerb.mID ? mVerb.mID->GetID() : uvInvalid;
	}

	/// Get the verb id and charge															
	///	@return verb charge																	
	inline const ChargedVerbID& Verb::GetChargedID() const noexcept {
		return mVerb;
	}

	/// Get a switchable value of the verb ID												
	///	@return the switch equivalent of a verb ID									
	inline auto Verb::GetSwitch() const noexcept {
		return GetID().GetHash().GetValue();
	}

	/// Get the verb meta																		
	///	@return the verb meta																
	inline VMeta Verb::GetMeta() const noexcept {
		return mVerb.mID;
	}

	/// Get the verb mass (a.k.a. magnitude)												
	///	@return the current mass															
	inline real Verb::GetMass() const noexcept {
		return mVerb.mCharge.mMass;
	}

	/// Get the verb frequency																	
	///	@return the current frequency														
	inline real Verb::GetFrequency() const noexcept {
		return mVerb.mCharge.mFrequency; 
	}

	/// Get the verb time 																		
	///	@return the current time															
	inline real Verb::GetTime() const noexcept {
		return mVerb.mCharge.mTime;
	}

	/// Get the verb priority 																	
	///	@return the current priority														
	inline real Verb::GetPriority() const noexcept {
		return mVerb.mCharge.mPriority;
	}

	/// Get verb source																			
	///	@return the verb source																
	inline Any& Verb::GetSource() noexcept {
		return mSource;
	}

	/// Get verb source (constant)															
	///	@return the verb source																
	inline const Any& Verb::GetSource() const noexcept {
		return mSource;
	}

	/// Get verb argument																		
	///	@return the verb argument															
	inline Any& Verb::GetArgument() noexcept {
		return mArgument;
	}

	/// Get verb argument (constant)															
	///	@return the verb argument															
	inline const Any& Verb::GetArgument() const noexcept {
		return mArgument;
	}

	/// Get verb output																			
	///	@return the verb output																
	inline Any& Verb::GetOutput() noexcept {
		return mOutput;
	}

	/// Get verb output (constant)															
	///	@return the verb output																
	inline const Any& Verb::GetOutput() const noexcept {
		return mOutput;
	}

	/// Check if verb outputs to a register trait										
	///	@tparam T - the trait to check for												
	///	@return true if verb outputs to the selected trait							
	template<RTTI::ReflectedTrait T>
	bool Verb::OutputsTo() const noexcept {
		return mOutput.GetCount() == 1 
			&& mOutput.Is<TraitID>() 
			&& mOutput.Get<TraitID>() == T::ID;
	}

	/// Push anything to output via shallow copy, satisfying the verb once		
	///	@tparam T - the type of the data to push (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator << (const T& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;

			mOutput.SmartPush<pcDecay<T>>(pcVal(data), DState::Default, true, true, uiBack);
			Done();
			return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << data;
		Done();
		return *this;
	}

	/// Output anything to the back by a move												
	///	@tparam T - the type of the data to move (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator << (T&& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;

			mOutput.SmartPush<pcDecay<T>>(pcVal(data), DState::Default, true, true, uiBack);
			Done();
			return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << pcForward<T>(data);
		Done();
		return *this;
	}

	/// Output anything to the front by a shallow copy									
	///	@tparam T - the type of the data to push (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator >> (const T& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;

			mOutput.SmartPush<pcDecay<T>>(pcVal(data), DState::Default, true, true, uiFront);
			Done();
			return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >> data;
		Done();
		return *this;
	}

	/// Output anything to the front by a move											
	///	@tparam T - the type of the data to move (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator >> (T&& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;

			mOutput.SmartPush<pcDecay<T>>(pcVal(data), DState::Default, true, true, uiFront);
			Done();
			return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >> pcForward<T>(data);
		Done();
		return *this;
	}

	/// Merge anything to output's back														
	///	@tparam T - the type of the data to merge										
	///	@param data - the data to merge													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator <<= (const T& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput <<= data;
		Done();
		return *this;
	}

	/// Merge anything to output's front													
	///	@tparam T - the type of the data to merge										
	///	@param data - the data to merge													
	///	@return a reference to this verb for chaining								
	template<RTTI::ReflectedData T>
	Verb& Verb::operator >>= (const T& data) {
		if constexpr (pcIsDeep<T>) {
			// Avoid pushing empty blocks												
			if (pcVal(data).IsEmpty())
				return *this;
		}

		if constexpr (Sparse<T>) {
			if (!PCMEMORY.CheckJurisdiction(MetaData::Of<T>(), data))
				throw Except::BadReference(pcLogFuncError
					<< "Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >>= data;
		Done();
		return *this;
	}

} // namespace PCFW::Flow

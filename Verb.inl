#pragma once
#include "Verb.hpp"

namespace Langulus::Flow
{

	/// Default construction																	
	constexpr Charge::Charge(Real mass, Real freq, Real time, Real prio, bool shortCircuited) noexcept
		: mMass {mass}
		, mFrequency {freq}
		, mTime {time}
		, mPriority {prio}
		, mShortCircuited {shortCircuited} {}

	/// Compare charges																			
	constexpr bool Charge::operator == (const Charge& ext) const noexcept {
		return mMass == ext.mMass 
			&& mFrequency == ext.mFrequency 
			&& mTime == ext.mTime
			&& mPriority == ext.mPriority
			&& mShortCircuited == ext.mShortCircuited;
	}

	/// Check if charge is default															
	constexpr bool Charge::IsDefault() const noexcept {
		return *this == Charge {};
	}

	/// Get the hash of the charge															
	inline Hash Charge::GetHash() const noexcept {
		return HashData(mMass) 
			| HashData(mFrequency) 
			| HashData(mTime) 
			| HashData(mPriority) 
			| HashData(mShortCircuited); //TODO check hashing | add HashDataArray?
	}

	/// Create a verb using a static verb type											
	///	@tparam T - the verb to use														
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	///	@return the verb																		
	template<CT::Verb T>
	Verb Verb::From(const Charge& c, const Any& s, const Any& a, const Any& o) {
		return {MetaVerb::Of<T>(), c, s, a, o};
	}

	/// Check if verb is a specific type													
	///	@tparam T - the verb to compare against										
	///	@return true if verbs match														
	template<CT::Verb T>
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
		mCharge.mMass *= Real {-1};
		return *this;
	}

	/// Set the verb ID																			
	///	@param call - the verb ID to assign												
	///	@return a reference to self														
	inline Verb& Verb::SetVerb(VMeta verb) noexcept {
		mVerb = verb;
		return *this;
	}

	/// Set the verb mass																		
	///	@param energy - the mass to set													
	///	@return a reference to self														
	inline Verb& Verb::SetMass(const Real energy) noexcept {
		mCharge.mMass = energy;
		return *this;
	}

	/// Set the verb frequency																	
	///	@param energy - the frequency to set											
	///	@return a reference to self														
	inline Verb& Verb::SetFrequency(const Real energy) noexcept {
		mCharge.mFrequency = energy;
		return *this;
	}

	/// Set the verb time																		
	///	@param energy - the time to set													
	///	@return a reference to self														
	inline Verb& Verb::SetTime(const Real energy) noexcept {
		mCharge.mTime = energy;
		return *this;
	}

	/// Set the verb priority																	
	///	@param energy - the priority to set												
	///	@return a reference to self														
	inline Verb& Verb::SetPriority(const Real energy) noexcept {
		mCharge.mPriority = energy;
		return *this;
	}

	/// Set the verb mass, frequency, time, and priority (a.k.a. charge)			
	///	@param charge - the charge to set												
	///	@return a reference to self														
	inline Verb& Verb::SetCharge(const Charge& charge) noexcept {
		mCharge = charge;
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
		mSource = Forward<Any>(source);
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
		mArgument = Forward<Any>(argument);
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
		mOutput = Forward<Any>(output);
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

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has larger or equal priority							
	inline bool Verb::operator < (const Verb& ext) const noexcept {
		return mCharge.mPriority < ext.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller or equal priority							
	inline bool Verb::operator > (const Verb& ext) const noexcept {
		return mCharge.mPriority > ext.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller priority										
	inline bool Verb::operator >= (const Verb& ext) const noexcept {
		return mCharge.mPriority >= ext.mCharge.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has larger priority										
	inline bool Verb::operator <= (const Verb& rhs) const noexcept {
		return mCharge.mPriority <= rhs.mCharge.mPriority;
	}

	/// Get the verb id																			
	///	@return verb ID																		
	inline VMeta Verb::GetVerb() const noexcept {
		return mVerb;
	}

	/// Get the verb id and charge															
	///	@return verb charge																	
	inline const Charge& Verb::GetCharge() const noexcept {
		return mCharge;
	}

	/// Get the verb mass (a.k.a. magnitude)												
	///	@return the current mass															
	inline Real Verb::GetMass() const noexcept {
		return mCharge.mMass;
	}

	/// Get the verb frequency																	
	///	@return the current frequency														
	inline Real Verb::GetFrequency() const noexcept {
		return mCharge.mFrequency; 
	}

	/// Get the verb time 																		
	///	@return the current time															
	inline Real Verb::GetTime() const noexcept {
		return mCharge.mTime;
	}

	/// Get the verb priority 																	
	///	@return the current priority														
	inline Real Verb::GetPriority() const noexcept {
		return mCharge.mPriority;
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
	template<CT::Trait T>
	bool Verb::OutputsTo() const noexcept {//TODO remove this?
		return mOutput.GetCount() == 1 
			&& mOutput.Is<TMeta>() 
			&& mOutput.Get<TMeta>()->Is<T>();
	}

	/// Push anything to output via shallow copy, satisfying the verb once		
	///	@tparam T - the type of the data to push (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator << (const T& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;

			mOutput.SmartPush<true, true, T>(data, DataState::Default, Index::Back);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << data;
		Done();
		return *this;
	}

	/// Output anything to the back by a move												
	///	@tparam T - the type of the data to move (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator << (T&& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;

			mOutput.SmartPush<true, true, T>(Forward<T>(data), DataState::Default, Index::Back);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << Forward<T>(data);
		Done();
		return *this;
	}

	/// Output anything to the front by a shallow copy									
	///	@tparam T - the type of the data to push (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator >> (const T& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;

			mOutput.SmartPush<true, true, T>(data, DataState::Default, Index::Front);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >> data;
		Done();
		return *this;
	}

	/// Output anything to the front by a move											
	///	@tparam T - the type of the data to move (deducible)						
	///	@param data - the data to push													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator >> (T&& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;

			mOutput.SmartPush<true, true, T>(Forward<T>(data), DataState::Default, Index::Front);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >> Forward<T>(data);
		Done();
		return *this;
	}

	/// Merge anything to output's back														
	///	@tparam T - the type of the data to merge										
	///	@param data - the data to merge													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator <<= (const T& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput <<= data;
		Done();
		return *this;
	}

	/// Merge anything to output's front													
	///	@tparam T - the type of the data to merge										
	///	@param data - the data to merge													
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator >>= (const T& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Inner::Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >>= data;
		Done();
		return *this;
	}

} // namespace Langulus::Flow

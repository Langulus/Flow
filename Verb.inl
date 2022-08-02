#pragma once
#include "Verb.hpp"

namespace Langulus::Flow
{

	/// Charge construction																		
	///	@param mass - the mass charge														
	///	@param freq - the frequency charge												
	///	@param time - the time charge														
	///	@param prio - the priority charge												
	constexpr Charge::Charge(Real mass, Real freq, Real time, Real prio) noexcept
		: mMass {mass}
		, mFrequency {freq}
		, mTime {time}
		, mPriority {prio} {}

	/// Compare charges																			
	///	@param rhs - the charge to compare against									
	///	@return true if both charges match exactly									
	constexpr bool Charge::operator == (const Charge& rhs) const noexcept {
		return mMass == rhs.mMass
			&& mFrequency == rhs.mFrequency
			&& mTime == rhs.mTime
			&& mPriority == rhs.mPriority;
	}

	/// Check if charge is default															
	///	@return true if charge is default												
	constexpr bool Charge::IsDefault() const noexcept {
		return *this == Charge {};
	}

	/// Get the hash of the charge															
	///	@return the hash of the charge													
	inline Hash Charge::GetHash() const noexcept {
		return HashData(mMass, mFrequency, mTime, mPriority);
	}

	/// Reset the charge to the default														
	inline void Charge::Reset() noexcept {
		mMass = DefaultMass;
		mFrequency = DefaultFrequency;
		mTime = DefaultTime;
		mPriority = DefaultPriority;
	}


	/// Check if verb is of a specific type												
	///	@tparam T - the verb to compare against										
	///	@return true if verbs match														
	template<CT::Data... T>
	bool Verb::IsVerb() const noexcept {
		static_assert((CT::Verb<T> && ...),
			"Provided types must be verb definitions");
		return (IsVerb(MetaVerb::Of<T>()) || ...);
	}

	/// Check if verb has been satisfied at least once									
	///	@return true if verb has been satisfied at least once						
	inline bool Verb::IsDone() const noexcept {
		return mSuccesses > 0;
	}

	/// Check if verb is short-circuited													
	///	@return true if verb is short-circuited										
	inline bool Verb::IsShortCircuited() const noexcept {
		return mShortCircuited;
	}

	/// Get the number of successful execution of the verb							
	///	@return the number of successful executions									
	inline Count Verb::GetSuccesses() const noexcept {
		return mSuccesses;
	}

	/// Satisfy verb a number of times														
	inline void Verb::Done(Count c) noexcept {
		mSuccesses = c;
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
		mMass *= Real {-1};
		return *this;
	}

	/// Set the verb ID																			
	///	@param verb - the verb to assign													
	///	@return a reference to self														
	inline Verb& Verb::SetVerb(VMeta verb) noexcept {
		mVerb = verb;
		return *this;
	}

	/// Set the verb mass																		
	///	@param mass - the mass to set														
	///	@return a reference to self														
	inline Verb& Verb::SetMass(const Real mass) noexcept {
		mMass = mass;
		return *this;
	}

	/// Set the verb frequency																	
	///	@param frequency - the frequency to set										
	///	@return a reference to self														
	inline Verb& Verb::SetFrequency(const Real frequency) noexcept {
		mFrequency = frequency;
		return *this;
	}

	/// Set the verb time																		
	///	@param time - the time to set														
	///	@return a reference to self														
	inline Verb& Verb::SetTime(const Real time) noexcept {
		mTime = time;
		return *this;
	}

	/// Set the verb priority																	
	///	@param priority - the priority to set											
	///	@return a reference to self														
	inline Verb& Verb::SetPriority(const Real priority) noexcept {
		mPriority = priority;
		return *this;
	}

	/// Set the verb mass, frequency, time, and priority (a.k.a. charge)			
	///	@param charge - the charge to set												
	///	@return a reference to self														
	inline Verb& Verb::SetCharge(const Charge& charge) noexcept {
		Charge::operator = (charge);
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
		Any::operator = (argument);
		return *this;
	}

	/// Set the verb argument by moving														
	///	@param argument - the argument to set											
	///	@return a reference to self														
	inline Verb& Verb::SetArgument(Any&& argument) noexcept {
		Any::operator = (Forward<Any>(argument));
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
	/*inline Verb& Verb::SetAll(const Any& s, const Any& a, const Any& o) {
		mSource = s;
		Any::operator = (a);
		mOutput = o;
		return *this;
	}*/

	/// Compare verbs																				
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (const Verb& rhs) const noexcept {
		return (mVerb == rhs.mVerb || (mVerb && mVerb->Is(rhs.mVerb)))
			&& mSource == rhs.mSource
			&& Any::operator == (rhs)
			&& mOutput == rhs.mOutput
			&& mShortCircuited == rhs.mShortCircuited;
	}

	/// Compare verb types for equality														
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (VMeta rhs) const noexcept {
		return IsVerb(rhs); 
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
		return mPriority < ext.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller or equal priority							
	inline bool Verb::operator > (const Verb& ext) const noexcept {
		return mPriority > ext.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has smaller priority										
	inline bool Verb::operator >= (const Verb& ext) const noexcept {
		return mPriority >= ext.mPriority;
	}

	/// Compare verb priorities																
	///	@param rhs - the verb to compare against										
	///	@return true if rhs has larger priority										
	inline bool Verb::operator <= (const Verb& rhs) const noexcept {
		return mPriority <= rhs.mPriority;
	}

	/// Get the verb id																			
	///	@return verb ID																		
	inline VMeta Verb::GetVerb() const noexcept {
		return mVerb;
	}

	/// Get the verb id and charge															
	///	@return verb charge																	
	inline const Charge& Verb::GetCharge() const noexcept {
		return static_cast<const Charge&>(*this);
	}

	/// Get the verb mass (a.k.a. magnitude)												
	///	@return the current mass															
	inline Real Verb::GetMass() const noexcept {
		return mMass;
	}

	/// Get the verb frequency																	
	///	@return the current frequency														
	inline Real Verb::GetFrequency() const noexcept {
		return mFrequency; 
	}

	/// Get the verb time 																		
	///	@return the current time															
	inline Real Verb::GetTime() const noexcept {
		return mTime;
	}

	/// Get the verb priority 																	
	///	@return the current priority														
	inline Real Verb::GetPriority() const noexcept {
		return mPriority;
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
		return static_cast<Any&>(*this);
	}

	/// Get verb argument (constant)															
	///	@return the verb argument															
	inline const Any& Verb::GetArgument() const noexcept {
		return static_cast<const Any&>(*this);
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

			mOutput.SmartPush<IndexBack, true, true, T>(data);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Allocator::CheckAuthority(MetaData::Of<T>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << data;
		Done();
		return *this;
	}

	/// Output anything to the back by a move												
	///	@tparam T - the type of the data to move (deducible)						
	///	@param data - the data to move to output										
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator << (T&& data) {
		if constexpr (CT::Deep<T>) {
			auto& denseData = DenseCast(data);

			// Avoid pushing empty blocks												
			if (denseData.IsEmpty())
				return *this;

			mOutput.SmartPush<IndexBack, true, true>(Move(denseData));
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Allocator::CheckAuthority(MetaData::Of<Decay<T>>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput << Forward<T>(data);
		Done();
		return *this;
	}

	/// Output anything to the front by a shallow copy									
	///	@tparam T - the type of the data to push (deducible)						
	///	@param data - the data to push to output										
	///	@return a reference to this verb for chaining								
	template<CT::Data T>
	Verb& Verb::operator >> (const T& data) {
		if constexpr (CT::Deep<T>) {
			// Avoid pushing empty blocks												
			if (DenseCast(data).IsEmpty())
				return *this;

			mOutput.SmartPush<IndexFront, true, true, T>(data);
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Allocator::CheckAuthority(MetaData::Of<Decay<T>>(), data))
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
			auto& denseData = DenseCast(data);

			// Avoid pushing empty blocks												
			if (denseData.IsEmpty())
				return *this;

			mOutput.SmartPush<IndexFront, true, true>(Move(denseData));
			Done();
			return *this;
		}

		if constexpr (CT::Sparse<T>) {
			if (!Allocator::CheckAuthority(MetaData::Of<Decay<T>>(), data))
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
			if (!Allocator::CheckAuthority(MetaData::Of<Decay<T>>(), data))
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
			if (!Allocator::CheckAuthority(MetaData::Of<Decay<T>>(), data))
				Throw<Except::Reference>(
					"Pushing unowned pointer to verb is a baaaaad idea");
		}

		mOutput >>= data;
		Done();
		return *this;
	}

	/// Finalize a dispatch execution by setting satisfaction state and output	
	///	@tparam OR - whether the dispatch happened in an OR context or not	
	///	@param successes - number of successes											
	///	@param output - the output container											
	///	@return the number of successes for the verb									
	template<bool OR>
	Count Verb::CompleteDispatch(const Count successes, Abandoned<Any>&& output) {
		if (mShortCircuited) {
			// If reached, this will result in failure in OR-context, or	
			// success if AND, as long as the verb is short-circuited		
			if constexpr (OR)
				mSuccesses = 0;
			else
				mSuccesses = successes;
		}
		else {
			// If verb is not short-circuited, then a single success			
			// is always enough															
			mSuccesses = successes;
		}

		// Set output																		
		if (mSuccesses) {
			output.mValue.Optimize();
			mOutput = output.Forward<Any>();
		}
		else mOutput.Reset();

		return mSuccesses;
	}

	/// Check if reflected abilities in T support this verb							
	/// This is a slow runtime check, use statically optimized variants inside	
	/// specific verbs if you know them at compile time								
	///	@return true if the ability exists												
	template<CT::Data T>
	bool Verb::GenericAvailableFor() const noexcept {
		const auto meta = MetaData::Of<Decay<T>>();
		return meta && meta->template GetAbility<CT::Mutable<T>>(mVerb, GetType());
	}

	/// Execute a known/unknown verb in an unknown context							
	/// This is a slow runtime procedure, use statically optimized variants		
	/// inside specific verbs if you know them at compile time						
	///	@attention assumes that if T is deep, it contains exactly one item	
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb was executed												
	template<CT::Data T, CT::Data V>
	bool Verb::GenericExecuteIn(T& context, V& verb) {
		static_assert(CT::Verb<V>, "V must be a verb");

		if constexpr (!CT::Deep<T> && !CT::Same<V, Verb>) {
			// Always prefer statically optimized routine when available	
			// Literally zero ability searching overhead!						
			if constexpr (V::template AvailableFor<T>())
				return V::ExecuteIn(context, verb);
			return false;
		}
		else {
			// Search for the ability via RTTI										
			const auto meta = context.GetType();
			if constexpr (CT::DerivedFrom<V, Verbs::Interpret> && requires { typename V::Type; }) {
				// Scan for a reflected converter as statically as possible	
				using TO = typename V::Type;
				const auto found = meta->template GetConverter<TO>();
				if (!found)
					return false;

				TAny<TO> result;
				result.template Allocate<false, true>(1);
				found(result.GetRaw(), context.GetRaw());
				verb << Abandon(result);
			}
			else {
				// Find ability at runtime												
				if (verb.template IsVerb<Verbs::Interpret>()) {
					// Scan for a reflected converter by scanning argument	
					const auto to = verb.template As<DMeta>();
					const auto found = meta->GetConverter(to);
					if (!found)
						return false;

					Any result = Any::FromMeta(to);
					result.Allocate<false, true>(1);
					found(result.GetRaw(), context.GetRaw());
					verb << Abandon(result);
				}
				else {
					// Scan for any another ability									
					const auto found = meta->template GetAbility<CT::Mutable<T>>(verb.mVerb, verb.GetType());
					if (!found)
						return false;

					found(SparseCast(context), verb);
				}
			}
		}

		return verb.IsDone();
	}

	/// Execute an unknown verb with its default behavior inside a mutable		
	/// context - this is a slow runtime procedure, use statically optimized	
	/// variants inside specific verbs if you know them at compile time			
	///	@attention assumes that context contains exactly one item				
	///	@tparam V - the verb type (deducible)											
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if verb was executed												
	template<CT::Data V>
	bool Verb::GenericExecuteDefault(Block& context, V& verb) {
		static_assert(CT::Verb<V>, "V must be a verb");

		if constexpr (!CT::Same<V, Verb>) {
			// Always prefer statically optimized routine when available	
			// Literally zero ability searching overhead!						
			if constexpr (CT::DefaultableVerb<V>)
				return V::ExecuteDefault(context, verb);
		}
		else {
			// Execute appropriate default routine by RTTI						
			if (verb.mVerb->mDefaultInvocationMutable) {
				verb.mVerb->mDefaultInvocationMutable(context, verb);
				return verb.IsDone();
			}
			else if (verb.mVerb->mDefaultInvocationConstant) {
				verb.mVerb->mDefaultInvocationConstant(context, verb);
				return verb.IsDone();
			}
		}

		return false;
	}

	/// Execute an unknown verb with its default behavior inside a constant		
	/// context - this is a slow runtime procedure, use statically optimized	
	/// variants inside specific verbs if you know them at compile time			
	///	@attention assumes that context contains exactly one item				
	///	@tparam V - the verb type (deducible)											
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if verb was executed												
	template<CT::Data V>
	bool Verb::GenericExecuteDefault(const Block& context, V& verb) {
		static_assert(CT::Verb<V>, "V must be a verb");

		if constexpr (!CT::Same<V, Verb>) {
			// Always prefer statically optimized routine when available	
			// Literally zero ability searching overhead!						
			if constexpr (CT::DefaultableVerbConstant<V>)
				return V::ExecuteDefault(context, verb);
		}
		else {
			// Execute appropriate default routine by RTTI						
			if (verb.mVerb->mDefaultInvocationConstant) {
				verb.mVerb->mDefaultInvocationConstant(context, verb);
				return verb.IsDone();
			}
		}

		return false;
	}

	/// Execute an unknown verb without context											
	/// This is a slow runtime procedure, use statically optimized variants		
	/// inside specific verbs if you know them at compile time						
	///	@tparam V - the verb type (deducible)											
	///	@param verb - the verb instance to execute									
	///	@return true if verb was executed												
	template<CT::Data V>
	bool Verb::GenericExecuteStateless(V& verb) {
		static_assert(CT::Verb<V>, "V must be a verb");

		if constexpr (!CT::Same<V, Verb>) {
			// Always prefer statically optimized routine when available	
			// Literally zero ability searching overhead!						
			if constexpr (CT::StatelessVerb<V>)
				return V::ExecuteStateless(verb);
		}
		else {
			// Execute appropriate stateless routine by RTTI					
			if (verb.mVerb->mStatelessInvocation) {
				verb.mVerb->mStatelessInvocation(verb);
				return verb.IsDone();
			}
		}

		return false;
	}

} // namespace Langulus::Flow

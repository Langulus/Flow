#include "Verb.hpp"
#include "Code.hpp"
#include "verbs/Interpret.inl"

#define DELEGATION_VERBOSE(a) pcLogSelfVerbose << a

namespace Langulus::Flow
{

	/// Serialize charge as code																
	Charge::operator Code() const {
		Code code;
		if (mMass != Charge::DefaultMass) {
			code += Code::Mass;
			code += mMass;
		}
		if (mFrequency != Charge::DefaultFrequency) {
			code += Code::Frequency;
			code += mFrequency;
		}
		if (mTime != Charge::DefaultTime) {
			code += Code::Time;
			code += mTime;
		}
		if (mPriority != Charge::DefaultPriority) {
			code += Code::Priority;
			code += mPriority;
		}
		return code;
	}

	/// Serialize charge as debug (same as code)											
	Charge::operator Debug() const {
		return Debug {Charge::operator Code()};
	}

	/// Scale the mass of a charge															
	constexpr Charge Charge::operator * (const Real& scalar) const noexcept {
		return {mMass * scalar, mFrequency, mTime, mPriority};
	}

	/// Scale the frequency of a charge														
	constexpr Charge Charge::operator ^ (const Real& scalar) const noexcept {
		return {mMass, mFrequency * scalar, mTime, mPriority};
	}

	/// Scale the mass of a charge															
	constexpr Charge& Charge::operator *= (const Real& scalar) noexcept {
		mMass *= scalar;
		return *this;
	}

	/// Scale the frequency of a charge														
	constexpr Charge& Charge::operator ^= (const Real& scalar) noexcept {
		mFrequency *= scalar;
		return *this;
	}

	/// Disown-construct a verb																
	///	@param other - the verb to disown and copy									
	Verb::Verb(Disowned<Verb>&& other) noexcept
		: Any {other.Forward<Any>()}
		, Charge {other.mValue}
		, mVerb {other.mValue.mVerb}
		, mSource {Disown(other.mValue.mSource)}
		, mOutput {Disown(other.mValue.mOutput)}
		, mState {other.mValue.mState} { }

	/// Abandon-construct a verb																
	///	@param other - the verb to abandon and move									
	Verb::Verb(Abandoned<Verb>&& other) noexcept
		: Any {other.Forward<Any>()}
		, Charge {other.mValue}
		, mVerb {other.mValue.mVerb}
		, mSource {Abandon(other.mValue.mSource)}
		, mOutput {Abandon(other.mValue.mOutput)}
		, mState {other.mValue.mState} { }

	/// Manual constructor with verb meta 													
	///	@param verb - the verb type														
	Verb::Verb(VMeta verb)
		: Any {}
		, mVerb {verb} { }

	/// Disown-assign a verb																	
	///	@param other - the verb to disown and copy									
	///	@return a reference to this verb													
	Verb& Verb::operator = (Disowned<Verb>&& other) {
		Any::operator = (other.Forward<Any>());
		Charge::operator = (other.mValue);
		mVerb = other.mValue.mVerb;
		mSource = Disown(other.mValue.mSource);
		mOutput = Disown(other.mValue.mOutput);
		mState = other.mValue.mState;
		return *this;
	}

	/// Abandon-assign a verb																	
	///	@param other - the verb to abandon and move									
	///	@return a reference to this verb													
	Verb& Verb::operator = (Abandoned<Verb>&& other) {
		Any::operator = (other.Forward<Any>());
		Charge::operator = (other.mValue);
		mVerb = other.mValue.mVerb;
		mSource = Abandon(other.mValue.mSource);
		mOutput = Abandon(other.mValue.mOutput);
		mState = other.mValue.mState;
		return *this;
	}

	/// Hash the verb																				
	///	@return the hash of the content													
	Hash Verb::GetHash() const {
		return HashData(mVerb->mHash, mSource, GetArgument(), mOutput);
	}

	/// Multiply verb mass																		
	///	@param rhs - the mass to multiply by											
	///	@return a new verb, with the modified mass									
	Verb Verb::operator * (const Real& rhs) const {
		Verb shallowCopy = *this;
		shallowCopy.mMass *= rhs;
		return shallowCopy;
	}

	/// Multiply verb frequency																
	///	@param rhs - the frequency to multiply by										
	///	@return a new verb, with the modified frequency								
	Verb Verb::operator ^ (const Real& rhs) const {
		Verb shallowCopy = *this;
		shallowCopy.mFrequency *= rhs;
		return shallowCopy;
	}

	/// Multiply verb mass (destructively)													
	///	@param rhs - the mass to multiply by											
	///	@return a reference to this verb													
	Verb& Verb::operator *= (const Real& rhs) noexcept {
		mMass *= rhs;
		return *this;
	}

	/// Multiply verb frequency (destructively)											
	///	@param rhs - the frequency to multiply by										
	///	@return a reference to this verb													
	Verb& Verb::operator ^= (const Real& rhs) noexcept {
		mFrequency *= rhs;
		return *this;
	}

	/// Partial copy, copies only charge, verb, and short-circuitness				
	///	@param other - the verb to use as base											
	///	@return the partially copied verb												
	Verb Verb::PartialCopy() const noexcept {
		return {mVerb, Any{}, GetCharge(), mState};
	}

	/// Clone the verb																			
	///	@return the cloned verb																
	Verb Verb::Clone() const {
		Verb clone {mVerb, Any::Clone(), GetCharge(), mState};
		clone.mSource = mSource.Clone();
		clone.mOutput = mOutput.Clone();
		clone.mSuccesses = mSuccesses;
		return clone;
	}

	/// Reset all verb members and energy													
	void Verb::Reset() {
		mVerb = {};
		Any::Reset();
		Charge::Reset();
		mSource.Reset();
		mOutput.Reset();
		mSuccesses = {};
	};

	/// Check if verb id matches																
	///	@param id - the verb id to check													
	///	@return true if verb id matches													
	bool Verb::VerbIs(VMeta id) const noexcept {
		return !mVerb ? !id : mVerb->Is(id);
	}

	/// Check if a verb is valid for the given priority								
	///	@param priority - the priority to check										
	///	@return true if this verb's priority matches the provided one			
	bool Verb::Validate(const Index& priority) const noexcept {
		return int(mPriority) == priority.mIndex || priority == IndexAll;
	}
	
	/// Change the verb's circuitry															
	///	@param toggle - enable or disable short-circuit								
	///	@return a reference to this verb for chaining								
	Verb& Verb::ShortCircuit(bool toggle) noexcept {
		if (toggle)
			mState -= VerbState::LongCircuited;
		else
			mState += VerbState::LongCircuited;
		return *this;
	}

	/// Change the verb's castness															
	///	@param toggle - enable or disable multicast									
	///	@return a reference to this verb for chaining								
	Verb& Verb::Multicast(bool toggle) noexcept {
		if (toggle)
			mState -= VerbState::Monocast;
		else
			mState += VerbState::Monocast;
		return *this;
	}

	/// Get the verb token																		
	///	@return the token as a literal													
	Token Verb::GetToken() const {
		if (!mVerb)
			return MetaVerb::DefaultToken;

		return mMass < 0 ? mVerb->mTokenReverse : mVerb->mToken;
	}

	/// Serialize verb to code																	
	Verb::operator Code() const {
		Code result;

		if (mSuccesses) {
			// If verb has been executed, just dump the output					
			result += Verbs::Interpret::To<Code>(mOutput);
			return result;
		}

		// If reached, then verb hasn't been executed yet						
		// Let's check if there's a source in which verb is executed		
		if (mSource.IsValid()) {
			result += Verbs::Interpret::To<Code>(mSource);
			result += ' ';
		}

		// After the source, we decide whether to write . and verb token,	
		// or simply an operator, depending on the verb definition			
		bool enscope = true;
		if (!mVerb) {
			// An invalid verb is always written as token						
			result += MetaVerb::DefaultToken;
		}
		else {
			// A valid verb is written either as token, or as operator		
			if (mMass < 0) {
				if (!mVerb->mOperatorReverse.empty() && (GetCharge() * -1).IsDefault()) {
					// Write as operator													
					result += mVerb->mOperatorReverse;
					enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
				}
				else {
					// Write as token														
					result += mVerb->mTokenReverse;
					result += Verbs::Interpret::To<Code>(GetCharge() * -1);
				}
			}
			else {
				if (!mVerb->mOperator.empty() && GetCharge().IsDefault()) {
					// Write as operator													
					result += mVerb->mOperator;
					enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
				}
				else {
					// Write as token														
					result += mVerb->mToken;
					result += Verbs::Interpret::To<Code>(GetCharge());
				}
			}
		}

		if (enscope)
			result += Code::OpenScope;

		if (IsValid())
			result += Verbs::Interpret::To<Code>(GetArgument());

		if (enscope)
			result += Code::CloseScope;

		return result;
	}

	/// Serialize verb for logger																
	Verb::operator Debug() const {
		Code result;

		if (mSuccesses) {
			// If verb has been executed, just dump the output					
			result += Verbs::Interpret::To<Debug>(mOutput);
			return Debug {result};
		}

		// If reached, then verb hasn't been executed yet						
		// Let's check if there's a source in which verb is executed		
		if (mSource.IsValid()) {
			result += Verbs::Interpret::To<Debug>(mSource);
			result += ' ';
		}

		// After the source, we decide whether to write . and verb token,	
		// or simply an operator, depending on the verb definition			
		bool enscope = true;
		if (!mVerb) {
			// An invalid verb is always written as token						
			result += MetaVerb::DefaultToken;
		}
		else {
			// A valid verb is written either as token, or as operator		
			if (mMass < 0) {
				if (!mVerb->mOperatorReverse.empty() && (GetCharge() * -1).IsDefault()) {
					// Write as operator													
					result += mVerb->mOperatorReverse;
					enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
				}
				else {
					// Write as token														
					result += mVerb->mTokenReverse;
					result += Verbs::Interpret::To<Debug>(GetCharge() * -1);
				}
			}
			else {
				if (!mVerb->mOperator.empty() && GetCharge().IsDefault()) {
					// Write as operator													
					result += mVerb->mOperator;
					enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
				}
				else {
					// Write as token														
					result += mVerb->mToken;
					result += Verbs::Interpret::To<Debug>(GetCharge());
				}
			}
		}

		if (enscope)
			result += Code::OpenScope;

		if (IsValid())
			result += Verbs::Interpret::To<Debug>(GetArgument());

		if (enscope)
			result += Code::CloseScope;

		return Debug {result};
	}

} // namespace Langulus::Flow


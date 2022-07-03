#include "Verb.hpp"
#include "Code.hpp"

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
		return Charge::operator Code();
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


	/// Manual constructor with verb meta 													
	///	@param call - the verb ID															
	///	@param charge - the charge															
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	///	@param shortCircuit - short circuit												
	Verb::Verb(VMeta call, const Any& s, const Any& a, const Any& o, const Charge& charge, bool shortCircuit)
		: Charge {charge}
		, mVerb {call}
		, mSource {s}
		, mArgument {a}
		, mOutput {o}
		, mShortCircuited {shortCircuit} { }

	/// Hash the verb																				
	///	@return the hash of the content													
	Hash Verb::GetHash() const {
		return HashData(mVerb->mHash, mSource, mArgument, mOutput);
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

	/// Partial copy (doesn't copy source, argument, and output)					
	///	@param other - the verb to use as base											
	///	@return the partially copied verb												
	Verb Verb::PartialCopy() const noexcept {
		return {mVerb, GetCharge()};
	}

	/// Clone the verb																			
	///	@return the cloned verb																
	Verb Verb::Clone() const {
		Verb clone {mVerb, GetCharge()};
		clone.mSource = mSource.Clone();
		clone.mArgument = mArgument.Clone();
		clone.mOutput = mOutput.Clone();
		clone.mSuccesses = mSuccesses;
		clone.mShortCircuited = mShortCircuited;
		return clone;
	}

	/// Reset all verb members and energy													
	void Verb::Reset() {
		mVerb = {};
		Charge::Reset();
		mSource.Reset();
		mArgument.Reset();
		mOutput.Reset();
		mSuccesses = {};
	};

	/// Check if verb id matches																
	///	@param id - the verb id to check													
	///	@return true if verb id matches													
	bool Verb::Is(VMeta id) const noexcept {
		return !mVerb ? !id : mVerb->Is(id);
	}

	/// Check if a verb is valid for the given priority								
	///	@param priority - the priority to check										
	///	@return true if this verb's priority matches the provided one			
	bool Verb::Validate(const Index& priority) const noexcept {
		return int(mPriority) == priority.mIndex || priority == Index::All;
	}
	
	/// Change the verb's circuitry															
	///	@param toggle - enable or disable short-circuit								
	///	@return a reference to this verb for chaining								
	Verb& Verb::ShortCircuit(bool toggle) noexcept {
		mShortCircuited = toggle;
		return *this;
	}

	/// Get the verb token																		
	///	@return the token as a literal													
	Token Verb::GetToken() const {
		if (!mVerb)
			return MetaVerb::DefaultToken;

		return mMass < 0 ? mVerb->mTokenReverse : mVerb->mToken;
	}

	/// Flat check if memory block contains executable verbs							
	///	@param block - the block to check for verbs									
	///	@return true if the flat memory block contains verbs						
	bool Verb::IsScopeExecutable(const Block& block) noexcept {
		if (block.Is<Verb>())
			return true;

		bool result{};
		EitherDoThis
			// Scan deeper into traits, because they're not marked deep		
			// They are deep only in respect to execution						
			block.ForEach([&result](const Trait& trait) {
				result = Verb::IsScopeExecutable(trait);
				return !result;
			})
		OrThis
			// Scan deeper into constructs, because they're not marked deep
			// They are deep only in respect to execution						
			block.ForEach([&result](const Construct& construct) {
				result = Verb::IsScopeExecutable(construct.GetAll());
				return !result;
			});

		return result;
	}

	/// Deep (slower) check if memory block contains executable verbs				
	///	@param block - the block to deeply iterate in search for verbs			
	///	@return true if the deep or flat memory block contains verbs			
	bool Verb::IsScopeExecutableDeep(const Block& block) {
		if (Verb::IsScopeExecutable(block))
			return true;

		bool result{};
		block.ForEachDeep([&result](const Block& group) {
			result = Verb::IsScopeExecutable(group);
			return !result;
		});

		return result;
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
		if (mSource.IsValid())
			result += Verbs::Interpret::To<Code>(mSource);

		// After the source, we decide whether to write . and verb token,	
		// or simply an operator, depending on the verb definition			
		bool enscope = true;
		if (!mVerb) {
			// An invalid verb is always written as token						
			if (mSource.IsValid())
				result += Code::Select;
			result += MetaVerb::DefaultToken;
		}
		else {
			// A valid verb is written either as token, or as operator		
			if (mMass < 0) {
				if (!mVerb->mOperatorReverse.empty() && (GetCharge() * -1).IsDefault()) {
					// Write as operator													
					result += mVerb->mOperatorReverse;
					enscope = false;
				}
				else {
					// Write as token														
					if (mSource.IsValid())
						result += Code::Select;
					result += mVerb->mTokenReverse;
					result += Verbs::Interpret::To<Code>(GetCharge() * -1);
				}
			}
			else {
				if (!mVerb->mOperator.empty() && GetCharge().IsDefault()) {
					// Write as operator													
					result += mVerb->mOperator;
					enscope = false;
				}
				else {
					// Write as token														
					if (mSource.IsValid())
						result += Code::Select;
					result += mVerb->mToken;
					result += Verbs::Interpret::To<Code>(GetCharge());
				}
			}
		}

		if (enscope)
			result += Code::OpenScope;

		if (mArgument.IsValid())
			result += Verbs::Interpret::To<Code>(mArgument);

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
			return result;
		}

		// If reached, then verb hasn't been executed yet						
		// Let's check if there's a source in which verb is executed		
		if (mSource.IsValid())
			result += Verbs::Interpret::To<Debug>(mSource);

		// After the source, we decide whether to write . and verb token,	
		// or simply an operator, depending on the verb definition			
		bool enscope = true;
		if (!mVerb) {
			// An invalid verb is always written as token						
			if (mSource.IsValid())
				result += Code::Select;
			result += MetaVerb::DefaultToken;
		}
		else {
			// A valid verb is written either as token, or as operator		
			if (mMass < 0) {
				if (!mVerb->mOperatorReverse.empty() && (GetCharge() * -1).IsDefault()) {
					// Write as operator													
					result += mVerb->mOperatorReverse;
					enscope = false;
				}
				else {
					// Write as token														
					if (mSource.IsValid())
						result += Code::Select;
					result += mVerb->mTokenReverse;
					result += Verbs::Interpret::To<Debug>(GetCharge() * -1);
				}
			}
			else {
				if (!mVerb->mOperator.empty() && GetCharge().IsDefault()) {
					// Write as operator													
					result += mVerb->mOperator;
					enscope = false;
				}
				else {
					// Write as token														
					if (mSource.IsValid())
						result += Code::Select;
					result += mVerb->mToken;
					result += Verbs::Interpret::To<Debug>(GetCharge());
				}
			}
		}

		if (enscope)
			result += Code::OpenScope;

		if (mArgument.IsValid())
			result += Verbs::Interpret::To<Debug>(mArgument);

		if (enscope)
			result += Code::CloseScope;

		return result;
	}

} // namespace Langulus::Flow


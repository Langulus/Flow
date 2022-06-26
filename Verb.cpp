#include "Verb.hpp"
#include "Code.hpp"

#define DELEGATION_VERBOSE(a) pcLogSelfVerbose << a

namespace Langulus::Flow
{

	/// Manual constructor with verb meta 													
	///	@param call - the verb ID															
	///	@param charge - the charge															
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	///	@param shortCircuit - short circuit												
	Verb::Verb(VMeta call, const Any& s, const Any& a, const Any& o, const Charge& charge, bool shortCircuit)
		: mVerb {call}
		, mCharge {charge}
		, mSource {s}
		, mArgument {a}
		, mOutput {o}
		, mShortCircuited {shortCircuit} { }

	/// Hash the verb																				
	///	@return the hash of the content													
	Hash Verb::GetHash() const {
		return mVerb->GetHash() | mSource.GetHash() | mArgument.GetHash() | mOutput.GetHash();
	}

	/// Compare verbs for equality															
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (const Verb& rhs) const {
		return mVerb == rhs.mVerb
			&& mCharge == rhs.mCharge
			&& mSource == rhs.mSource
			&& mArgument == rhs.mArgument
			&& mOutput == rhs.mOutput;
	}

	/// Compare verb types for equality														
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (VMeta rhs) const noexcept {
		return Is(rhs); 
	}

	/// Partial copy (doesn't copy source, argument, and output)					
	///	@param other - the verb to use as base											
	///	@return the partially copied verb												
	Verb Verb::PartialCopy() const noexcept {
		return {mVerb, mCharge};
	}

	/// Clone the verb																			
	///	@return the cloned verb																
	Verb Verb::Clone() const {
		Verb clone {mVerb, mCharge};
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
		mCharge = {};
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
		return int(mCharge.mPriority) == priority.mIndex || priority == Index::All;
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

		return mCharge.mMass < 0 ? mVerb->mTokenReverse : mVerb->mToken;
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

	/// Serialize verb to Code																	
	Verb::operator Code() const {
		Code result;
		result += GetVerb();
		result += GetCharge();
		result += Code::OpenScope;
		if (mSource.IsValid()) {
			result += pcSerialize<Code>(mSource);
			result += Code::Context;
		}
		if (mArgument.IsValid())
			result += pcSerialize<Code>(mArgument);
		if (mOutput.IsValid()) {
			result += Code::As;
			result += pcSerialize<Code>(mOutput);
		}
		result += Code::CloseScope;
		return result;
	}

	/// Stringify verb																			
	Verb::operator Debug() const {
		Code result;
		result += GetVerb();
		result += GetCharge();
		result += Code::OpenScope;
		if (mSource.IsValid()) {
			result += pcSerialize<Debug>(mSource);
			result += Code::Context;
		}
		if (mArgument.IsValid())
			result += pcSerialize<Debug>(mArgument);
		if (mOutput.IsValid()) {
			result += Code::As;
			result += pcSerialize<Debug>(mOutput);
		}
		result += Code::CloseScope;
		return result;
	}

} // namespace Langulus::Flow


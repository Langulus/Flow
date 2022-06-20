#include "Verb.hpp"
#include "GASM.hpp"
#include "IncludeLogger.hpp"

#define DELEGATION_VERBOSE(a) pcLogSelfVerbose << a
#define HASHING_VERBOSE(a) //pcLogSelfVerbose << a

namespace PCFW::Flow
{

	/// Manual constructor with verb meta 													
	///	@param call - the verb ID															
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	Verb::Verb(VMeta call, const Any& s, const Any& a, const Any& o)
		: mVerb {call}
		, mSource {s}
		, mArgument {a}
		, mOutput {o} { }

	/// Manual constructor with verb id 													
	///	@param call - the verb ID															
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	Verb::Verb(const VerbID& call, const Any& s, const Any& a, const Any& o)
		: mVerb {call.GetMeta()}
		, mSource {s}
		, mArgument {a}
		, mOutput {o} { }

	/// Manual constructor with modified verb id 										
	///	@param call - the modified verb ID												
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	Verb::Verb(const ChargedVerbID& call, const Any& s, const Any& a, const Any& o)
		: mVerb {call}
		, mSource {s}
		, mArgument {a}
		, mOutput {o} { }

	/// Manual constructor with verb token 												
	///	@param call - the verb token														
	///	@param s - the source																
	///	@param a - the argument																
	///	@param o - the output																
	Verb::Verb(const Text& call, const Any& s, const Any& a, const Any& o)
		: Verb(PCMEMORY.GetMetaVerb(call), s, a, o) {
		if (mVerb.mID->mStaticDescriptor.mTokenReverse == LiteralText{ call })
			mVerb.mCharge.mMass *= real(-1);
	}

	/// Hash the verb																				
	///	@return the hash of the content													
	Hash Verb::GetHash() const {
		HASHING_VERBOSE("mVerb hash: " << mVerb.GetHash());
		HASHING_VERBOSE("mSource hash: " << mSource.GetHash() << "; type: " << mSource.GetToken());
		HASHING_VERBOSE("mArgument hash: " << mArgument.GetHash() << "; type: " << mArgument.GetToken());
		HASHING_VERBOSE("mOutput hash: " << mOutput.GetHash() << "; type: " << mOutput.GetToken());
		return mVerb.GetHash() | mSource.GetHash() | mArgument.GetHash() | mOutput.GetHash();
	}

	/// Compare verbs for equality															
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (const Verb& rhs) const {
		return mVerb == rhs.mVerb
			&& mSource == rhs.mSource
			&& mArgument == rhs.mArgument
			&& mOutput == rhs.mOutput;
	}

	/// Compare verbs for inequality															
	///	@param rhs - the verb to compare against										
	///	@return true if verbs don't match												
	inline bool Verb::operator != (const Verb& rhs) const {
		return !(*this == rhs);
	}

	/// Compare verb types for equality														
	///	@param rhs - the verb to compare against										
	///	@return true if verbs match														
	inline bool Verb::operator == (const VerbID& rhs) const noexcept {
		return Is(rhs); 
	}

	/// Compare verb types for inequality													
	///	@param rhs - the verb to compare against										
	///	@return true if verbs don't match												
	inline bool Verb::operator != (const VerbID& rhs) const noexcept {
		return !(*this == rhs); 
	}

	/// Partial copy (doesn't copy source, argument, and output)					
	///	@param other - the verb to use as base											
	///	@return the partially copied verb												
	Verb Verb::PartialCopy() const noexcept {
		return { mVerb };
	}

	/// Clone the verb																			
	///	@return the cloned verb																
	Verb Verb::Clone() const {
		Verb clone { mVerb };
		clone.mSource = mSource.Clone();
		clone.mArgument = mArgument.Clone();
		clone.mOutput = mOutput.Clone();
		clone.mSuccesses = mSuccesses;
		return clone;
	}

	/// Reset all verb members and energy													
	void Verb::Reset() {
		mVerb = {};
		mSource.Reset();
		mArgument.Reset();
		mOutput.Reset();
		mSuccesses = 0;
	};

	/// Check if verb id matches																
	///	@param id - the verb id to check													
	///	@return true if verb id matches													
	bool Verb::Is(const VerbID& id) const noexcept {
		return !mVerb.mID ? !id : mVerb.mID->GetID() == id;
	}

	/// Check if a verb is valid for the given priority								
	///	@param priority - the priority to check										
	///	@return true if this verb's priority matches the provided one			
	bool Verb::Validate(const Index& priority) const noexcept {
		return int(mVerb.mCharge.mPriority) == priority.mIndex || priority == uiAll;
	}
	
	/// Change the verb's circuitry															
	///	@param toggle - enable or disable short-circuit								
	///	@return a reference to this verb for chaining								
	Verb& Verb::ShortCircuit(bool toggle) noexcept {
		mVerb.mShortCircuited = toggle;
		return *this;
	}

	/// Get the verb token																		
	///	@return the token as a literal													
	LiteralText Verb::GetToken() const {
		if (!mVerb.mID)
			return VerbID::DefaultToken;

		if (mVerb.mCharge.mMass < 0)
			return mVerb.mID->mStaticDescriptor.mTokenReverse;
		else
			return mVerb.mID->mStaticDescriptor.mToken;
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

	/// Serialize verb to GASM																	
	Verb::operator GASM() const {
		GASM result;
		result += GetChargedID();
		result += GASM::OpenScope;
		if (mSource.IsValid()) {
			result += pcSerialize<GASM>(mSource);
			result += GASM::Context;
		}
		if (mArgument.IsValid())
			result += pcSerialize<GASM>(mArgument);
		if (mOutput.IsValid()) {
			result += GASM::As;
			result += pcSerialize<GASM>(mOutput);
		}
		result += GASM::CloseScope;
		return result;
	}

	/// Stringify verb																			
	Verb::operator Debug() const {
		GASM result;
		result += GetChargedID();
		result += GASM::OpenScope;
		if (mSource.IsValid()) {
			result += pcSerialize<Debug>(mSource);
			result += GASM::Context;
		}
		if (mArgument.IsValid())
			result += pcSerialize<Debug>(mArgument);
		if (mOutput.IsValid()) {
			result += GASM::As;
			result += pcSerialize<Debug>(mOutput);
		}
		result += GASM::CloseScope;
		return result;
	}


} // namespace PCFW::PCGASM


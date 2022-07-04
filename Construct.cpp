#include "Construct.hpp"
#include "Serial.hpp"

#define VERBOSE_CONSTRUCT(a) //pcLogFuncVerbose << a

namespace Langulus::Flow
{

	/// Construct from a header																
	///	@param type - the type of the content											
	Construct::Construct(DMeta type)
		: mType {type} {}

	/// Construct from a header and movable arguments									
	///	@param type - the type of the content											
	///	@param arguments - the argument container to move							
	Construct::Construct(DMeta type, Any&& arguments, const Charge& charge)
		: Charge {charge}
		, mType {type}
		, mArguments {Forward<Any>(arguments)} { }

	/// Construct from a header and shallow-copied arguments							
	///	@param type - the type of the content											
	///	@param arguments - the argument container to copy							
	Construct::Construct(DMeta type, const Any& arguments, const Charge& charge)
		: Charge {charge}
		, mType {type}
		, mArguments {arguments} { }

	/// Hash the descriptor																		
	///	@return the hash of the content													
	Hash Construct::GetHash() const {
		if (mHash.mHash)
			return mHash;

		const_cast<Hash&>(mHash) = HashData(mType->mHash, mArguments.GetHash());
		return mHash;
	}

	/// Clears arguments and charge															
	void Construct::Clear() {
		Charge::Reset();
		mArguments.Reset();
		mHash = {};
	}

	/// Clone construct																			
	///	@param override - whether or not to change header of the cloned		
	///	@return a construct with cloned arguments										
	Construct Construct::Clone(DMeta overrride) const {
		Construct clone {overrride ? overrride : mType};
		static_cast<Charge&>(clone) = static_cast<const Charge&>(*this);
		clone.mArguments = mArguments.Clone();
		if (!overrride)
			clone.mHash = GetHash();
		return clone;
	}

	/// Compare descriptors																		
	///	@param rhs - descriptor to compare with										
	///	@return true if both constructs are the same									
	bool Construct::operator == (const Construct& rhs) const noexcept {
		return GetHash() == rhs.GetHash()
			&& mType == rhs.mType
			&& mArguments == rhs.mArguments;
	}

	/// Attempt to create construct statically if possible							
	/// If not possible, simply propagate the construct								
	///	@param output - [out] results go here											
	void Construct::StaticCreation(Any& output) const {
		auto meta = GetType();
		if (meta->mProducer) {
			// If the construct requires a producer, just propagate it		
			// without changing anything - static creation is impossibru	
			output << *this;
			return;
		}

		// If reached, data doesn't rely on a producer							
		// Make sure we're creating something concrete							
		auto arguments = const_cast<Construct*>(this)->GetAll();
		meta = meta->mConcrete;
		if (arguments.GetCountElementsDeep() == 1) {
			// Convert single argument to requested type							
			// If a direct copy is available it will be utilized				
			auto interpreter = Verbs::Interpret({}, meta);
			if (Verb::DispatchDeep(arguments, interpreter) && !interpreter.GetOutput().IsEmpty()) {
				// Success																	
				VERBOSE_CONSTRUCT("Constructed from DataID: "
					<< Logger::Cyan << interpreter.GetOutput());
				output << Move(interpreter.GetOutput());
				return;
			}
		}
		
		// Either Interpret verb didn't do the trick, or multiple items	
		// were provided, so we need to inspect members, and satisfy them	
		// one by one																		
		const auto concreteConstruct = Construct::From(meta, arguments);
		auto creator = Verbs::Create({}, &concreteConstruct);
		if (!Verb::DispatchEmpty(creator) || creator.GetOutput().IsEmpty()) {
			Throw<Except::Construct>(Logger::Error()
				<< "Can't construct " << meta->mToken
				<< " from " << concreteConstruct);
		}

		VERBOSE_CONSTRUCT("Constructed from initializer-list: "
			<< Logger::Cyan << creator.GetOutput());
		output << Move(creator.GetOutput());
	}

	/// Check if constructor header can be interpreted as another type			
	///	@param type - the type check if current type interprets to				
	///	@return true if able to interpret current header to 'type'				
	bool Construct::InterpretsAs(DMeta type) const {
		return !type || (mType == type || mType->CastsTo(type));
	}

	/// Check if constructor header is exactly another type							
	///	@param type - the type to check for (must be a dense type)				
	///	@return true if current header is 'type'										
	bool Construct::Is(DMeta type) const {
		return !type || (mType && mType->Is(type));
	}

	/// Set a tagged argument inside constructor											
	///	@param trait - trait to set														
	///	@param index - the index we're interested with if repeated				
	///	@return a reference to this construct for chaining							
	Construct& Construct::Set(const Trait& trait, const Offset& index) {
		bool done = false;
		Count counter = 0;
		mArguments.ForEachDeep([&](Block& group) {
			group.ForEach([&](Trait& t) {
				if (t.GetTrait() != trait.GetTrait())
					return true;

				if (counter == index) {
					t = trait;
					mHash = {};
					done = true;
					return false;
				}
				
				++counter;
				return true;
			});

			return !done;
		});

		if (!done)
			*this << Any(trait);
		return *this;
	}

	/// Get a tagged argument inside constructor											
	///	@param meta - trait to search for												
	///	@param index - the index we're interested in, if repeated				
	///	@return selected trait or nullptr if none was found						
	const Trait* Construct::Get(TMeta meta, const Offset& index) const {
		const Trait* found = nullptr;
		Count counter = 0;
		mArguments.ForEachDeep([&](const Block& group) {
			group.ForEach([&](const Trait& t) {
				if (t.GetTrait() != meta)
					return true;

				if (counter == index) {
					found = &t;
					return false;
				}

				++counter;
				return true;
			});

			return !found;
		});

		return found;
	}

	/// Serialize a construct to code														
	Construct::operator Code() const {
		Code result;
		result += mType->mToken;
		if (!Charge::IsDefault() || !mArguments.IsEmpty()) {
			result += Verbs::Interpret::To<Code>(GetCharge());
			result += Code::OpenScope;
			result += Verbs::Interpret::To<Code>(mArguments);
			result += Code::CloseScope;
		}

		return result;
	}

	/// Stringify a construct for logging													
	Construct::operator Debug() const {
		Code result;
		result += mType->mToken;
		if (!Charge::IsDefault() || !mArguments.IsEmpty()) {
			result += Verbs::Interpret::To<Debug>(GetCharge());
			result += Code::OpenScope;
			result += Verbs::Interpret::To<Debug>(mArguments);
			result += Code::CloseScope;
		}

		return result;
	}

} // namespace Langulus::Flow

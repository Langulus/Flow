#include "Construct.hpp"
#include "Serial.hpp"

#define VERBOSE_CONSTRUCT(a) //pcLogFuncVerbose << a

namespace Langulus::Flow
{

	/// Construct from a header																
	///	@param header - the type of the content										
	Construct::Construct(DataID header)
		: mHeader {header.GetMeta()} {}

	/// Construct from a header																
	///	@param header - the type of the content										
	Construct::Construct(DMeta header)
		: mHeader {header} {}

	/// Construct from a header token														
	///	@param header - the type token of the content								
	Construct::Construct(const Text& header)
		: mHeader {PCMEMORY.GetMetaData(header)} {}

	/// Construct from a header and movable arguments									
	///	@param header - the type of the content										
	///	@param arguments - the argument container to move							
	Construct::Construct(DataID header, Any&& arguments)
		: mHeader {header.GetMeta()}
		, mArguments {Forward<Any>(arguments)} { }

	/// Construct from a header and movable arguments									
	///	@param header - the type of the content										
	///	@param arguments - the argument container to move							
	Construct::Construct(DMeta header, Any&& arguments)
		: mHeader {header}
		, mArguments {Forward<Any>(arguments)} { }

	/// Construct from a header and movable arguments									
	///	@param header - the type token of the content								
	///	@param arguments - the argument container to move							
	Construct::Construct(const Text& header, Any&& arguments)
		: mHeader {PCMEMORY.GetMetaData(header)}
		, mArguments {Forward<Any>(arguments)} { }

	/// Construct from a header and shallow-copied arguments							
	///	@param header - the type of the content										
	///	@param arguments - the argument container to copy							
	Construct::Construct(DataID header, const Any& arguments)
		: mHeader {header.GetMeta()}
		, mArguments {arguments} { }

	/// Construct from a header and shallow-copied arguments							
	///	@param header - the type of the content										
	///	@param arguments - the argument container to copy							
	Construct::Construct(DMeta header, const Any& arguments)
		: mHeader {header}
		, mArguments {arguments} { }

	/// Construct from a header and shallow-copied arguments							
	///	@param header - the type token of the content								
	///	@param arguments - the argument container to copy							
	Construct::Construct(const Text& header, const Any& arguments)
		: mHeader {PCMEMORY.GetMetaData(header)}
		, mArguments {arguments} { }

	/// Hash the descriptor																		
	///	@return the hash of the content													
	Hash Construct::GetHash() const {
		if (mHashed)
			return mHash;
		return Hashed::SetHash(mHeader->GetID().GetHash() | mArguments.GetHash());
	}

	/// Clears arguments and charge															
	void Construct::Clear() {
		mCharge = {};
		mArguments.Reset();
		ResetHash();
	}

	/// Clone construct																			
	///	@param override - whether or not to change header of the cloned		
	///	@return a construct with cloned arguments										
	Construct Construct::Clone(DMeta overrride) const {
		Construct clone {overrride ? overrride : mHeader};
		clone.mCharge = mCharge;
		clone.mArguments = mArguments.Clone();
		if (!overrride)
			clone.SetHash(GetHash());
		return clone;
	}

	/// Compare descriptors																		
	///	@param other - descriptor to compare with										
	///	@return true if both content descriptors are the same						
	bool Construct::operator == (const Construct& rhs) const noexcept {
		return CompareHash(rhs)
			&& mHeader == rhs.mHeader && mArguments == rhs.mArguments;
	}

	/// Attempt to create construct statically if possible							
	/// If not possible, simply propagate the construct								
	///	@param output - [out] results go here											
	void Construct::StaticCreation(Any& output) const {
		auto meta = GetMeta();
		if (meta->GetProducerMeta()) {
			// If the construct requires a producer, just propagate it		
			// without changing anything - static creation is impossibru	
			output << *this;
			return;
		}

		// If reached, data doesn't rely on a producer							
		// Make sure we're creating something concrete							
		auto arguments = const_cast<Construct*>(this)->GetAll();
		meta = meta->GetConcreteMeta();
		if (arguments.GetCountElementsDeep() == 1) {
			// Convert single argument to requested type							
			// If a direct copy is available it will be utilized				
			auto interpreter = Verb::From<Verbs::Interpret>({}, meta);
			if (Verb::DispatchDeep(arguments, interpreter)
				&& !interpreter.GetOutput().IsEmpty()) {
				// Success																	
				VERBOSE_CONSTRUCT("Constructed from DataID: "
					<< ccCyan << interpreter.GetOutput());
				output << pcMove(interpreter.GetOutput());
				return;
			}
		}
		
		// Either Interpret verb didn't do the trick, or multiple items	
		// were provided, so we need to inspect members, and satisfy them	
		// one by one																		
		const auto concreteConstruct = Construct::From(meta, arguments);
		auto creator = Verb::From<Verbs::Create>({}, &concreteConstruct);
		if (!Verb::DispatchEmpty(creator) || creator.GetOutput().IsEmpty()) {
			throw Except::BadConstruction(pcLogFuncError
				<< "Can't construct " << meta->GetToken()
				<< " from " << concreteConstruct);
		}

		VERBOSE_CONSTRUCT("Constructed from initializer-list: "
			<< ccCyan << creator.GetOutput());
		output << pcMove(creator.GetOutput());
	}

	/// Check if constructor header can be interpreted as another type			
	///	@param type - the type check if current type interprets to				
	///	@return true if able to interpret current header to 'type'				
	bool Construct::InterpretsAs(DMeta type) const {
		return !type || (mHeader == type || mHeader->InterpretsAs(type));
	}

	/// Check if constructor header is exactly another type							
	///	@param type - the type to check for (must be a dense type)				
	///	@return true if current header is 'type'										
	bool Construct::Is(DataID type) const {
		return !type || (mHeader && mHeader->Is(type));
	}

	/// Set a tagged argument inside constructor											
	///	@param trait - trait to set														
	///	@param index - the index we're interested with if repeated				
	///	@return a reference to this construct for chaining							
	Construct& Construct::Set(const Trait& trait, const pcptr& index) {
		bool done = false;
		pcptr counter = 0;
		mArguments.ForEachDeep([&](Block& group) {
			group.ForEach([&](Trait& t) {
				if (t.GetTraitMeta() != trait.GetTraitMeta())
					return true;

				if (counter == index) {
					t = trait;
					ResetHash();
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
	const Trait* Construct::Get(TMeta meta, const pcptr& index) const {
		const Trait* found = nullptr;
		pcptr counter = 0;
		mArguments.ForEachDeep([&](const Block& group) {
			group.ForEach([&](const Trait& t) {
				if (t.GetTraitMeta() != meta)
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

	/// Serialize a construct to Code														
	Construct::operator Code() const {
		Code result;
		result += mHeader->GetToken();
		if (!mCharge.IsDefault() || !mArguments.IsEmpty()) {
			result += mCharge;
			result += Code::OpenScope;
			result += pcSerialize<Code>(mArguments);
			result += Code::CloseScope;
		}
		return result;
	}

	/// Stringify a construct																	
	Construct::operator Debug() const {
		Code result;
		result += GetMeta()->GetToken();
		if (!mCharge.IsDefault() || !mArguments.IsEmpty()) {
			result += mCharge;
			result += Code::OpenScope;
			result += pcSerialize<Debug>(mArguments);
			result += Code::CloseScope;
		}
		return result;
	}

} // namespace Langulus::Flow

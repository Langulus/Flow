#pragma once
#include "Construct.hpp"

namespace Langulus::Flow
{

	/// Create content descriptor from a type and include a constructor			
	///	@param type - the type we're constructing										
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<CT::Data DATA>
	Construct Construct::From(DMeta type, DATA&& arguments) {
		return Construct(type) << Forward<DATA>(arguments);
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param type - the type we're constructing										
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<CT::Data DATA>
	Construct Construct::From(DMeta type, const DATA& arguments) {
		return Construct(type) << arguments;
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<CT::Data T, CT::Data DATA>
	Construct Construct::From(DATA&& arguments) {
		return Construct::From<DATA>(MetaData::Of<T>(), Forward<DATA>(arguments));
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<CT::Data T, CT::Data DATA>
	Construct Construct::From(const DATA& arguments) {
		return Construct::From<DATA>(MetaData::Of<T>(), arguments);
	}

	/// Create content descriptor from a type and include a constructor			
	///	@param arguments - the constructor arguments									
	///	@return the request																	
	template<CT::Data T>
	Construct Construct::From() {
		return Construct(MetaData::Of<T>());
	}

	/// Check if contained data can be interpreted as a given type					
	/// Interpreting means reading compatiblity											
	template<CT::Data T>
	bool Construct::InterpretsAs() const {
		return InterpretsAs(MetaData::Of<T>());
	}

	/// Check if contained data fully matches a given type							
	template<CT::Data T>
	bool Construct::Is() const {
		return Is(MetaData::Of<T>());
	}

	inline const Any& Construct::GetAll() const noexcept {
		return static_cast<const Any&>(*this);
	}

	inline Any& Construct::GetAll() noexcept {
		return static_cast<Any&>(*this);
	}

	inline const Charge& Construct::GetCharge() const noexcept {
		return static_cast<const Charge&>(*this);
	}

	inline DMeta Construct::GetType() const noexcept {
		return mType;
	}

	template<CT::Data T>
	Construct Construct::CloneAs() const {
		return Clone(MetaData::Of<T>());
	}

	/// Copy items to the construct															
	///	@param whatever - the thing you wish to push									
	template<CT::Data T>
	Construct& Construct::operator << (const T& whatever) {
		if (Any::SmartPush<Index::Back, true, true, T, Any>(whatever))
			mHash = {};
		return *this;
	}

	/// Merge items to the construct's arguments											
	///	@param whatever - the thing you wish to push									
	template<CT::Data T>
	Construct& Construct::operator <<= (const T& whatever) {
		if constexpr (CT::Same<T, Trait>)
			return Set(DenseCast(whatever));
		else {
			if (!FindDeep(whatever))
				*this << whatever;
			return *this;
		}
	}

	/// Get traits from constructor															
	///	@return pointer to found traits or nullptr if none found					
	template<CT::Trait T>
	const Trait* Construct::Get(const Offset& index) const {
		return Get(MetaTrait::Of<T>(), index);
	}

} // namespace Langulus::Flow

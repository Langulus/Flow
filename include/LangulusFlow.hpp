#pragma once
#include "../source/Code.hpp"
#include "../source/Resolvable.hpp"
#include "../source/Temporal.hpp"

#include "../source/verbs/Create.inl"
#include "../source/verbs/Conjunct.inl"
#include "../source/verbs/Associate.inl"
#include "../source/verbs/Interpret.inl"
#include "../source/verbs/Select.inl"
#include "../source/verbs/Add.inl"
#include "../source/verbs/Multiply.inl"
#include "../source/verbs/Exponent.inl"
#include "../source/verbs/Catenate.inl"
#include "../source/verbs/Do.inl"

#define LANGULUS_MODULE_FLOW()

namespace Langulus
{

	/// Get the meta of some stuff, just for convenience								
	///	@tparam T - type to get meta definition of									
	///	@return the meta definition of the provided stuff							
	template<class T>
	NOD() auto MetaOf() {
		if constexpr (CT::Trait<T>)
			return RTTI::MetaTrait::Of<T>();
		else if constexpr (CT::Verb<T>)
			return RTTI::MetaVerb::Of<T>();
		else
			return RTTI::MetaData::Of<T>();
	}

} // namespace Langulus
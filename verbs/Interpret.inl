#pragma once
#include "../Verb.hpp"

#define VERBOSE_CONVERSION(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Statically optimized interpret verb												
	///	@param from - the element to convert											
	///	@return the converted element														
	template<class TO, class FROM>
	TO Interpret::To(const FROM& from) {
		static_assert(CT::Convertible<FROM, TO>, "Type is not convertible");
		return static_cast<TO>(from);
	}

} // namespace Langulus::Verbs
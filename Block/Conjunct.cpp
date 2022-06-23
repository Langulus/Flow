#include "../Code.hpp"
#include "IncludeLogger.hpp"

#define PC_SELECT_VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Flow
{

	/// Default conjunction, catenating arguments to the context					
	///	@param verb - the stuff to conjunct												
	void Verb::DefaultConjunct(Block& context, Verb& verb) {
		if (!verb.GetArgument().IsEmpty()) {
			Any conjunct;
			conjunct << Any(context);
			conjunct << verb.GetArgument();
			verb << pcMove(conjunct);
		}
	}

} // namespace Langulus::Flow

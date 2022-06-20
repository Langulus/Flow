#include "../GASM.hpp"
#include "IncludeLogger.hpp"

#define PC_SELECT_VERBOSE(a) //pcLogFuncVerbose << a

namespace PCFW::Flow
{

	/// Default disjunction, or-catenating arguments to the context				
	///	@param verb - the stuff to disjunct												
	void Verb::DefaultDisjunct(Block& context, Verb& verb) {
		if (!verb.GetArgument().IsEmpty()) {
			Any disjunct;
			disjunct.MakeOr();
			disjunct << Any(context);
			disjunct << verb.GetArgument();
			verb << pcMove(disjunct);
		}
	}

} // namespace PCFW::PCGASM

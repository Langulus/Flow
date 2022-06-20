#include "../GASM.hpp"
#include "IncludeLogger.hpp"

#define PC_SELECT_VERBOSE(a) //pcLogFuncVerbose << a

namespace PCFW::Flow
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

} // namespace PCFW::PCGASM

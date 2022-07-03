#include "../Code.hpp"

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
			verb << Move(conjunct);
		}
	}

	/// Default disjunction, or-catenating arguments to the context				
	///	@param verb - the stuff to disjunct												
	void Verb::DefaultDisjunct(Block& context, Verb& verb) {
		if (!verb.GetArgument().IsEmpty()) {
			Any disjunct;
			disjunct.MakeOr();
			disjunct << Any(context);
			disjunct << verb.GetArgument();
			verb << Move(disjunct);
		}
	}

} // namespace Langulus::Flow

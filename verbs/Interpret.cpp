#include "../Code.hpp"

#define VERBOSE_INTERPRET(a) //Logger::Verbose() << a

namespace Langulus::Flow
{

	/*template<Number FROM, Number TO>
	void TDefaultNumberConverter2(Verb& verb, const FROM& from) {
		//TODO do some realtime conversion checks here
		verb << static_cast<TO>(from);
	}

	template<Number FROM>
	void TDefaultNumberConverter(Verb& verb, const FROM& from, DMeta to) {
		if (to->Is<pcu8>())
			TDefaultNumberConverter2<FROM, pcu8>(verb, from);
		else if (to->Is<pcu16>())
			TDefaultNumberConverter2<FROM, pcu16>(verb, from);
		else if (to->Is<pcu32>())
			TDefaultNumberConverter2<FROM, pcu32>(verb, from);
		else if (to->Is<pcu64>())
			TDefaultNumberConverter2<FROM, pcu64>(verb, from);
		else if (to->Is<pci8>())
			TDefaultNumberConverter2<FROM, pci8>(verb, from);
		else if (to->Is<pci16>())
			TDefaultNumberConverter2<FROM, pci16>(verb, from);
		else if (to->Is<pci32>())
			TDefaultNumberConverter2<FROM, pci32>(verb, from);
		else if (to->Is<pci64>())
			TDefaultNumberConverter2<FROM, pci64>(verb, from);
		else if (to->Is<pcr32>())
			TDefaultNumberConverter2<FROM, pcr32>(verb, from);
		else if (to->Is<pcr64>())
			TDefaultNumberConverter2<FROM, pcr64>(verb, from);
	}

	void DefaultNumberConverter(Verb& verb, const Block& from, DMeta to) {
		if (from.Is<pcu8>())
			TDefaultNumberConverter(verb, from.Get<pcu8>(), to);
		else if (from.Is<pcu16>())
			TDefaultNumberConverter(verb, from.Get<pcu16>(), to);
		else if (from.Is<pcu32>())
			TDefaultNumberConverter(verb, from.Get<pcu32>(), to);
		else if (from.Is<pcu64>())
			TDefaultNumberConverter(verb, from.Get<pcu64>(), to);
		else if (from.Is<pci8>())
			TDefaultNumberConverter(verb, from.Get<pci8>(), to);
		else if (from.Is<pci16>())
			TDefaultNumberConverter(verb, from.Get<pci16>(), to);
		else if (from.Is<pci32>())
			TDefaultNumberConverter(verb, from.Get<pci32>(), to);
		else if (from.Is<pci64>())
			TDefaultNumberConverter(verb, from.Get<pci64>(), to);
		else if (from.Is<pcr32>())
			TDefaultNumberConverter(verb, from.Get<pcr32>(), to);
		else if (from.Is<pcr64>())
			TDefaultNumberConverter(verb, from.Get<pcr64>(), to);
	}*/

	/// Default interpretation																	
	/// Checks for trivial conversions, such as conversion to the same type,	
	/// or a derived type to a base. For all the rest, a converter must be		
	/// reflected (and implemented, if not a constructor or cast-operator)		
	///	@param context - the block to execute in										
	///	@param verb - interpretation verb												
	void Verb::DefaultInterpret(Block& context, Verb& verb) {
		SAFETY(if (context.IsDeep()) {
			Throw<Except::Flow>(Logger::Error() <<
				"Default-interpreting a deep context is no allowed"
				" - the flow handle should've handled that")\
		});

		// For each type inside verb argument										
		verb.GetArgument().ForEachDeep([&](DMeta type) {
			if (context.Is(type) || (type->mIsAbstract && context.CastsToMeta(type))) {
				// Types match, nothing to really interpret, just forward	
				// and rely on pointer arithmetics eventually					
				VERBOSE_INTERPRET("Implicitly converted " << context << " from "
					<< context.GetToken() << " to " << type << " (same type or abstract base)");
				verb << Any {context};
			}
			else if (context.CastsTo<A::Number>(1) && !context.IsAbstract() && type->CastsTo<A::Number>(1)) {
				// We're casting one built-in number to another					
				DefaultNumberConverter(verb, context, type);
			}
			else {
				// Avoid serializing here, just literals allowed				
				// Otherwise stack explosions will commence						
				VERBOSE_INTERPRET(Logger::Red << "Can't convert "
					<< context.GetToken() << "(s) to " << type->mToken
					<< " - no conversion routine inferred or reflected");
			}
		});
	}

} // namespace Langulus::Flow

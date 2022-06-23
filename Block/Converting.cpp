#include "Converting.hpp"

namespace Langulus::Flow
{

	template<Number FROM, Number TO>
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
	}

	/// Default interpretation																	
	/// Checks for trivial conversions, such as conversion to the same type,	
	/// or a derived type to a base. For all the rest, a converter must be		
	/// reflected (and implemented, if not a constructor or cast-operator)		
	///	@param context - the block to execute in										
	///	@param verb - interpretation verb												
	void Verb::DefaultInterpret(Block& context, Verb& verb) {
		const auto doer = [&](DMeta type) {
			if (context.Is(type->GetID())) {
				// Types match, nothing to really interpret, just forwrad	
				PC_VERBOSE_CONVERSION("Implicitly converted " << context << " from " 
					<< context.GetToken() << " to " << type << " (same type)");
				verb << Any {context};
			}
			else if (context.InterpretsAs(type) && type->IsAbstract()) {
				// We're interpreting as an abstract base of some sort		
				// - just return context without any interpretation			
				PC_VERBOSE_CONVERSION("Implicitly converted " << context << " from " 
					<< context.GetToken() << " to " << type << " (abstract base)");
				verb << Any {context};
			}
			else if (context.InterpretsAs<ANumber>(1) && !context.IsAbstract() && type->InterpretsAs<ANumber>(1)) {
				// We're interpreting one built-in number to another			
				DefaultNumberConverter(verb, context, type);
			}
			else {
				// Avoid serializing here, just literals allowed				
				// Otherwise stack explosions will commence						
				PC_VERBOSE_CONVERSION(ccRed << "Can't convert " 
					<< context.GetToken() << "(s) to " << type->GetToken()
					<< " - no conversion routine inferred or reflected");
			}
		};

		// For each DataID or MetaData inside verb argument					
		verb.GetArgument().ForEachDeep([&doer](const Block& group) {
			EitherDoThis
				group.ForEach([&doer](const DataID& t) {
					doer(t.GetMeta());
				})
			OrThis
				group.ForEach([&doer](const MetaData* t) {
					doer(t);
				});
		});
	}

} // namespace Langulus::Flow

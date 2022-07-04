#include "../Code.hpp"

#define PC_SELECT_VERBOSE(a) //Logger::Verbose() << a

namespace Langulus::Flow
{

	/// Select members by providing either meta data or meta trait					
	///	@tparam META - type of filter we're using										
	///	@param context - the thing we're searching in								
	///	@param selectedTraits - [out] found traits go here							
	///	@param resultingTrait - the type of trait to push to selectedTraits	
	///	@param meta - the filter we'll be using										
	///	@param indices - optional indices (i.e. Nth trait of a kind				
	///	@return true if at least one trait has been pushed to selectedTraits	
	template<class META>
	bool PerIndex(Block& context, TAny<Trait>& selectedTraits, TMeta resultingTrait, META meta, const TAny<Index>& indices) {
		bool done = false;
		if (indices.IsEmpty()) {
			auto variable = context.GetMember(meta);
			if (variable.IsAllocated()) {
				selectedTraits << Trait(resultingTrait, variable);
				done = true;
			}
		}
		else for (auto& idx : indices) {
			auto variable = context.GetMember(meta, idx.GetOffset());
			if (variable.IsAllocated()) {
				selectedTraits << Trait(resultingTrait, variable);
				done = true;
			}
		}

		return done;
	}

	/// Select ability or member by a meta													
	///	@param indices - optional index for trait/verb (Nth trait/verb)		
	///	@param id - type of data we'll be selecting									
	///	@param context - the thing we're searching in								
	///	@param selectedTraits - [out] found traits go here							
	///	@param selectedVerbs - [out] found verb go here								
	///	@return if at least trait/verb has been pushed to outputs				
	bool SelectByMeta(const TAny<Index>& indices, DMeta id, Block& context, TAny<Trait>& selectedTraits, TAny<VMeta>& selectedVerbs) {
		const auto type = context.GetType();
		if (id->Is<VMeta>()) {
			if (indices.IsEmpty() || indices == Index::All) { //TODO make sure the == operator is optimal
				// Retrieve each verb inside rhs for this block					
				for (auto& ability : type->mAbilities)
					selectedVerbs << ability.second.mVerb;
			}
			else for (auto& idx : indices) {
				// Retrieve specified abilities by index							
				selectedVerbs << type->GetAbility(idx.GetOffset()).mVerb;
			}

			return true;
		}

		// Select data IDs																
		return PerIndex(context, selectedTraits, nullptr, id, indices);
	};

	/// Default selection - retrieves reflected traits and/or abilities			
	///	@param context - the place to search for selection							
	///	@param verb - the stuff to retrieve												
	void Verb::DefaultSelect(Block& context, Verb& verb) {
		if (context.IsEmpty())
			return;

		TAny<Index> indices;
		verb.GetArgument().Gather(indices);
		bool containsOnlyIndices = !indices.IsEmpty();

		TAny<Trait> selectedTraits;
		TAny<VMeta> selectedVerbs;

		// Scan verb argument for anything but indices							
		verb.GetArgument().ForEachDeep([&](const Block& group) {
			// Skip indices - they were gathered before the loop				
			if (group.Is<Index>())
				return;

			EitherDoThis
				group.ForEach([&](const Construct& construct) {
					containsOnlyIndices = false;
					auto nested = verb.PartialCopy().SetArgument(construct.GetAll());
					Verb::DefaultSelect(context, nested);
					verb << nested.GetOutput();
				})
			OrThis
				group.ForEach([&](const Trait& trait) {
					containsOnlyIndices = false;
					auto tmeta = trait.GetTrait();
					if (tmeta)
						PerIndex(context, selectedTraits, tmeta, tmeta, indices);
					else
						PerIndex(context, selectedTraits, tmeta, trait.GetType(), indices);
				})
			OrThis
				group.ForEach([&](TMeta tmeta) {
					containsOnlyIndices = false;
					PerIndex(context, selectedTraits, tmeta, tmeta, indices);
				})
			OrThis
				group.ForEach([&](DMeta dmeta) {
					containsOnlyIndices = false;
					SelectByMeta(indices, dmeta, context, selectedTraits, selectedVerbs);
				});
		});

		if (containsOnlyIndices) {
			// Try selecting via indices only										
			// This is allowed only if no metas were found in the argument	
			PerIndex(context, selectedTraits, nullptr, nullptr, indices);
		}

		// Output results if any, satisfying the verb							
		verb << selectedTraits;// .Decay(); //TODO investigate this issue and if an issue generalize it by implementing it in verb::operator <<
		verb << selectedVerbs;// .Decay();
	}

} // namespace Langulus::Flow

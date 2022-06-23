#include "../Code.hpp"
#include "IncludeLogger.hpp"

#define PC_SELECT_VERBOSE(a) //pcLogFuncVerbose << a

namespace Langulus::Flow
{

	/// @brief 
	/// @tparam META 
	/// @param context 
	/// @param selectedTraits 
	/// @param resultingTrait 
	/// @param meta 
	/// @param indices 
	/// @return 
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
			if (idx.IsSpecial())
				throw Except::BadAccess(pcLogFuncError << "Can't select with index " << idx);

			auto variable = context.GetMember(meta, static_cast<pcptr>(idx));
			if (variable.IsAllocated()) {
				selectedTraits << Trait(resultingTrait, variable);
				done = true;
			}
		}

		return done;
	}

	/// Select ability or member by a meta													
	/// @param indices 
	/// @param id 
	/// @param context 
	/// @param selectedTraits 
	/// @param selectedVerbs 
	/// @return 
	bool SelectByMeta(const TAny<Index>& indices, DMeta id, Block& context, TAny<Trait>& selectedTraits, TAny<VerbID>& selectedVerbs) {
		if (id->Is<VerbID>()) {
			if (indices.IsEmpty() || (indices.GetCount() == 1 && indices[0] == uiAll)) {
				// Retrieve each verb inside rhs for this block					
				for (auto& ability : context.GetMeta()->GetAbilityList())
					selectedVerbs << ability.mStaticAbility.mVerb;
			}
			else for (auto& idx : indices) {
				// Retrieve specified abilities										
				if (idx.IsSpecial() || idx >= context.GetMeta()->GetAbilityList().size()) {
					pcLogWarning << "Skipping special index in default selection: " << idx;
					continue;
				}

				selectedVerbs << context.GetMeta()->GetAbility(static_cast<pcptr>(idx)).mStaticAbility.mVerb;
			}

			return true;
		}

		// Select data IDs																
		return PerIndex(context, selectedTraits, nullptr, id, indices);
	};

	/// Default uvSelect - retrieves static traits and/or abilities				
	///	@param verb - the stuff to retrieve												
	void Verb::DefaultSelect(Block& context, Verb& verb) {
		if (context.IsEmpty())
			return;

		TAny<Index> indices;
		verb.GetArgument().Gather(indices);
		bool containsOnlyIndices = !indices.IsEmpty();

		TAny<Trait> selectedTraits;
		TAny<VerbID> selectedVerbs;

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
					auto tmeta = trait.GetTraitMeta();
					if (tmeta)
						PerIndex(context, selectedTraits, tmeta, tmeta, indices);
					else
						PerIndex(context, selectedTraits, tmeta, trait.GetMeta(), indices);
				})
			OrThis
				group.ForEach([&](const TraitID& trait) {
					containsOnlyIndices = false;
					auto tmeta = trait.GetMeta();
					PerIndex(context, selectedTraits, tmeta, tmeta, indices);
				})
			OrThis
				group.ForEach([&](const MetaTrait* tmeta) {
					containsOnlyIndices = false;
					PerIndex(context, selectedTraits, tmeta, tmeta, indices);
				})
			OrThis
				group.ForEach([&](const DataID& id) {
					containsOnlyIndices = false;
					SelectByMeta(indices, id.GetMeta(), context, selectedTraits, selectedVerbs);
				})
			OrThis
				group.ForEach([&](const MetaData* id) {
					containsOnlyIndices = false;
					SelectByMeta(indices, id, context, selectedTraits, selectedVerbs);
				});
		});

		if (containsOnlyIndices) {
			// Try selecting via indices only										
			// This is allowed only if no metas were found in the argument	
			PerIndex(context, selectedTraits, nullptr, nullptr, indices);
		}

		// Output results if any, satisfying the verb							
		verb << selectedTraits.Decay();
		verb << selectedVerbs.Decay();
	}

} // namespace Langulus::Flow

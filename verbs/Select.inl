#pragma once
#include "../Code.hpp"

#define VERBOSE_SELECT(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Select/deselect verb construction													
	///	@param s - where are we selecting?												
	///	@param a - what are we searching for?											
	///	@param o - result mask (optional)												
	///	@param c - the charge of the selection											
	///	@param sc - is the selection short-circuited									
	inline Select::Select(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Select>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Select::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Select(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Select(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Select::Of() noexcept {
		if constexpr (!Select::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Select(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Select(verb, args...);
			};
		}
	}

	/// Execute the selection verb in a specific context								
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Select::ExecuteIn(T& context, Verb& verb) {
		if constexpr (Select::AvailableFor<T>()) {
			context.Select(verb);
			return verb.IsDone();
		}
		else return false;
	}

	/// Execute the default verb in a mtuable context									
	/// Returns mutable results																
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Select::ExecuteDefault(Block& context, Verb& verb) {
		TAny<Index> indices;
		verb.GetArgument().Gather(indices);
		bool containsOnlyIndices = !indices.IsEmpty();

		TAny<Trait> selectedTraits;
		TAny<const RTTI::Ability*> selectedAbilities;

		// Scan verb argument for anything but indices							
		verb.GetArgument().ForEachDeep([&](const Block& group) {
			// Skip indices - they were gathered before the loop				
			if (group.Is<Index>())
				return;

			group.ForEach(
				[&](const Construct& construct) {
					containsOnlyIndices = false;
					auto nested = verb.PartialCopy().SetArgument(construct.GetAll());
					ExecuteDefault(context, nested);
					verb << nested.GetOutput();
				},
				[&](const Trait& trait) {
					containsOnlyIndices = false;
					auto tmeta = trait.GetTrait();
					if (tmeta)
						PerIndex(context, selectedTraits, tmeta, tmeta, indices);
					else
						PerIndex(context, selectedTraits, tmeta, trait.GetType(), indices);
				},
				[&](TMeta tmeta) {
					containsOnlyIndices = false;
					PerIndex(context, selectedTraits, tmeta, tmeta, indices);
				},
				[&](DMeta dmeta) {
					containsOnlyIndices = false;
					SelectByMeta(indices, dmeta, context, selectedTraits, selectedAbilities);
				}
			);
		});

		if (containsOnlyIndices) {
			// Try selecting via indices only										
			// This is allowed only if no metas were found in the argument	
			PerIndex(context, selectedTraits, nullptr, nullptr, indices);
		}

		// Output results if any, satisfying the verb							
		verb << selectedTraits;// .Decay(); //TODO investigate this issue and if an issue generalize it by implementing it in verb::operator <<
		verb << selectedAbilities;// .Decay();
	}

	/// Select members by providing either meta data or meta trait					
	///	@tparam META - type of filter we're using										
	///	@param context - the thing we're searching in								
	///	@param selectedTraits - [out] found traits go here							
	///	@param resultingTrait - the type of trait to push to selectedTraits	
	///	@param meta - the filter we'll be using										
	///	@param indices - optional indices (i.e. Nth trait of a kind				
	///	@return true if at least one trait has been pushed to selectedTraits	
	template<class META>
	bool Select::PerIndex(Block& context, TAny<Trait>& selectedTraits, TMeta resultingTrait, META meta, const TAny<Index>& indices) {
		bool done = false;
		if (indices.IsEmpty()) {
			// Meta is the only filter													
			auto variable = context.GetMember(meta);
			if (variable.IsAllocated()) {
				selectedTraits << Trait {resultingTrait, variable};
				done = true;
			}
		}
		else for (auto& idx : indices) {
			// Search for each meta-index pair										
			auto variable = context.GetMember(meta, idx.GetOffset());
			if (variable.IsAllocated()) {
				selectedTraits << Trait {resultingTrait, variable};
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
	bool Select::SelectByMeta(const TAny<Index>& indices, DMeta id, Block& context, TAny<Trait>& selectedTraits, TAny<const RTTI::Ability*>& selectedVerbs) {
		const auto type = context.GetType();
		if (id->Is<VMeta>()) {
			if (indices.IsEmpty() || indices == Index::All) { //TODO make sure the == operator is optimal
				// Retrieve each ability corresponding to verbs in rhs		
				for (auto& ability : type->mAbilities)
					selectedVerbs << &ability.second;
			}
			else for (auto& idx : indices) {
				// Retrieve specified abilities by index							
				Count counter {};
				for (auto& ability : type->mAbilities) {
					if (counter == idx.GetOffset()) {
						selectedVerbs << &ability.second;
						break;
					}
					++counter;
				}
			}

			return true;
		}

		// Select data IDs																
		return PerIndex(context, selectedTraits, nullptr, id, indices);
	};

} // namespace Langulus::Verbs

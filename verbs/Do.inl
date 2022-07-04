#pragma once
#include "../Verb.hpp"

namespace Langulus::Flow
{

	/// Call the default built-in memory abilities										
	/// This should be called only in memory blocks that are flat and contain	
	/// only a single element																	
	///	@param context - the context in which scope will be executed			
	///	@param verb - the verb to send over												
	///	@return true on success																
	/*template<CT::Verb V>
	bool Verb::DefaultDo(Block& context, V& verb) {
		SAFETY(if (context.GetCount() > 1 || context.IsDeep()) {
			Throw<Except::Flow>(Logger::Error()
				<< "Should operate on flat single/empty instances only, but "
				<< context.GetCount() << " instead (or just deep)")
		});

		if constexpr (CT:Same<V, Verbs::Interpret>)
			DefaultInterpret(context, verb);

		switch (verb.GetSwitch()) {
		case Verbs::Interpret::Switch:
			DefaultInterpret(context, verb);
			break;
		case Verbs::Associate::Switch:
			DefaultAssociate(context, verb);
			break;
		case Verbs::Select::Switch:
			DefaultSelect(context, verb);
			break;
		case Verbs::Create::Switch:
			DefaultCreate(context, verb);
			break;
		case Verbs::Scope::Switch:
			DefaultScope(context, verb);
			break;
		case Verbs::Conjunct::Switch:
			DefaultConjunct(context, verb);
			break;
		case Verbs::Disjunct::Switch:
			DefaultDisjunct(context, verb);
			break;
		}

		return verb.IsDone();
	}*/

	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V>
	Count Execute(T& context, V& verb);

	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V, CT::Data... BASES>
	Count ExecuteInBases(T& context, V& verb, TTypeList<BASES...> bases) {
		return (Execute<DISPATCH, DEFAULT, FALLBACK>(static_cast<BASES&>(context), verb) || ...);
	}

	/// Invoke a static verb on a static type												
	///	@tparam RESOLVE - perform runtime type-checking and execute in the	
	///							most concrete result											
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@return the number of successful executions									
	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V>
	Count Execute(T& context, V& verb) {
		static_assert(!CT::Deep<T>,  "T must be flat");
		static_assert( CT::Dense<T>, "T must be dense");

		// Always reset verb progress prior to execution						
		verb.Undo();

		if constexpr (!FALLBACK && DISPATCH && CT::Dispatcher<T>) {
			// Custom reflected dispatcher is available							
			// It's your responsibility to implement it adequately			
			// Keep in mind, that once you declare a custom Do for your		
			// type, you no longer rely on reflected bases' verbs or			
			// default verbs. You must invoke those by yourself in your		
			// dispatcher - the custom dispatcher provides full control		
			context.Do(verb);
		}
		else if constexpr (V::template AvailableFor<T>()) {
			// Execute verb inside the context directly							
			V::ExecuteIn<FALLBACK>(context, verb);

			// If that fails, attempt in all reflected bases					
			if constexpr (requires { typename T::CTTI_Bases; }) {
				if (!verb.IsDone()) {
					// Context has no abilities, or they failed, so try with	
					// all bases' abilities												
					ExecuteInBases<true, false, FALLBACK>(context, verb, typename T::CTTI_Bases {});
				}
			}

			// If that fails, attempt executing the default verb				
			if constexpr (DEFAULT && !FALLBACK) {
				if (!verb.IsDone()) {
					// Verb wasn't executed neither in current element, nor	
					// in	any of its bases, so we resort to the default		
					// abilities															
					Execute<false, false, true>(context, verb);
				}
			}
		}

		return verb.mSuccesses;
	}

} // namespace Langulus::Flow


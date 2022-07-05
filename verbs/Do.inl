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
		//static_assert(!CT::Deep<T>,  "T must be flat");
		//static_assert( CT::Dense<T>, "T must be dense");

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

		return verb.GetSuccesses();
	}

	/// Invoke a verb on a flat context of as much elements as you want			
	/// If an element is not able to execute verb, attempt calling the default	
	/// This should be called only in memory blocks that are flat					
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@return the number of successful executions									
	template<bool RESOLVE, bool DISPATCH, bool DEFAULT, CT::Data T, CT::Verb V>
	Count DispatchFlat(T& context, V& verb) {
		//static_assert(!CT::Deep<T>, "T must be flat");

		if (context.IsEmpty()) {
			if constexpr (DEFAULT)
				Execute<DISPATCH, true, true>(context, verb);
			return 0;
		}

		auto output = Any::FromState(context);

		// Iterate elements in the current context								
		Count successCount {};
		for (Count i = 0; i < context.GetCount(); ++i) {
			// Reset verb to initial state											
			if constexpr (RESOLVE) {
				auto resolved = context.GetElementResolved(i);
				Execute<DISPATCH, DEFAULT, false>(resolved, verb);
			}
			else {
				auto resolved = context.GetElementDense(i);
				Execute<DISPATCH, DEFAULT, false>(resolved, verb);
			}

			if (verb.IsShortCircuited()) {
				// Short-circuit if enabled for verb								
				if (verb.IsDone() == context.IsOr()) {
					// Time to early-exit												
					// Will fail on first AND-failure								
					// Will succeed on first OR-success								
					if (context.IsOr()) {
						// OR-Success														
						// Will carry its own output, no need to use cache		
						verb.Done(1);
					}
					else {
						// AND-Failure														
						// All outputs are discarded									
						verb.Undo();
					}

					return verb.GetSuccesses();
				}
			}
			
			if (verb.IsDone() && !verb.GetOutput().IsEmpty()) {
				// Cache output, conserving the context hierarchy				
				output << Move(verb.GetOutput());
			}

			if (verb.IsDone())
				++successCount;
		}
		
		if (context.IsOr())
			return verb.CompleteDispatch<true>(successCount, Abandon(output));
		return verb.CompleteDispatch<false>(successCount, Abandon(output));
	}

	/// Invoke a verb on a container, that is either deep or flat, either		
	/// AND, or OR. The verb will be executed for each flat element inside		
	/// this block. If a failure occurs inside a scope, that scope will be		
	/// considered failed, unless it's an OR scope - OR scopes stop execution	
	/// right after the first success and fail only	if all branches fail			
	///	@param context - the context in which scope will be executed			
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	template<bool RESOLVE, bool DISPATCH, bool DEFAULT, CT::Data T, CT::Verb V>
	Count DispatchDeep(T& context, V& verb) {
		if (context.IsDeep() || context.Is<Trait>()) {
			// Nest if context is deep, or a trait									
			// Traits are considered deep only when executing in them		
			// There is no escape from this scope									
			Count successCount {};
			auto output = Any::FromState(context);
			for (Count i = 0; i < context.GetCount(); ++i) {
				const auto hits = DispatchDeep<RESOLVE, DISPATCH, DEFAULT>(context.As<Block>(i), verb);
				successCount += hits;

				if (verb.IsShortCircuited()) {
					// Short-circuit if enabled for verb							
					if (context.IsOr() == (successCount > 0)) {
						// It is time for an early return							
						// Will fail on first AND-failure							
						// Will succeed on first OR-success							
						if (context.IsOr()) {
							// OR-Success													
							// Will carry its own output								
							verb.Done(successCount);
						}
						else {
							// AND-Failure													
							// All outputs are discarded								
							verb.Undo();
						}

						return verb.GetSuccesses();
					}
				}

				// Cache each output, conserving the context hierarchy		
				if (hits && !verb.GetOutput().IsEmpty())
					output << Move(verb.GetOutput());
			}

			if (context.IsOr())
				return verb.CompleteDispatch<true>(successCount, Abandon(output));
			return verb.CompleteDispatch<false>(successCount, Abandon(output));
		}

		// If reached, then block is flat											
		// Execute implemented verbs if available, or fallback to			
		// default verbs, eventually													
		return DispatchFlat<RESOLVE, DISPATCH, DEFAULT>(context, verb);
	}

	/// Invoke a verb on an empty context													
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	template<CT::Verb V>
	Count DispatchEmpty(V& verb) {
		return V::ExecuteStateless(verb);
	}

} // namespace Langulus::Flow


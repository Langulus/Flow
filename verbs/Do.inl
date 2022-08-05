#pragma once
#include "../Verb.hpp"

namespace Langulus::Verbs
{

	/// Default Do/Undo verb construction													
	inline Do::Do()
		: Verb {RTTI::MetaVerb::Of<Do>()} {}

	/// Do/Undo verb construction via shallow-copy										
	///	@param a - what to execute															
	///	@param c - the charge of the do/undo											
	///	@param sc - is the do/undo short-circuited									
	template<CT::Data T>
	Do::Do(const T& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Do>(), a, c, sc} {}

	/// Do/Undo verb construction via move													
	///	@param a - what to execute															
	///	@param c - the charge of the do/undo											
	///	@param sc - is the do/undo short-circuited									
	template<CT::Data T>
	Do::Do(T&& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Do>(), Forward<T>(a), c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Do::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Do(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Do(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Do::Of() noexcept {
		if constexpr (!Do::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Do(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Do(verb, args...);
			};
		}
	}

	/// Execute the do/undo verb in a specific context									
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Do::ExecuteIn(T& context, Verb& verb) {
		static_assert(Do::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Do(verb);
		return verb.IsDone();
	}

	/// Default do/undo in an immutable context											
	///	@param context - the block to execute in										
	///	@param verb - do/undo verb															
	inline bool Do::ExecuteDefault(const Block& context, Verb& verb) {
		//TODO
		return true;
	}

	/// Default do/undo in a mutable context												
	///	@param context - the block to execute in										
	///	@param verb - do/undo verb															
	inline bool Do::ExecuteDefault(Block& context, Verb& verb) {
		//TODO
		return true;
	}

	/// Stateless execution																		
	///	@param verb - the do/undo verb													
	///	@return true if verb was satisfied												
	inline bool Do::ExecuteStateless(Verb& verb) {
		if (verb.GetArgument().IsEmpty())
			return false;

		//TODO execute
		return true;
	}

} // namespace Langulus::Verbs


namespace Langulus::Flow
{

	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V>
	Count Execute(T& context, V& verb);

	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V, CT::Data... BASES>
	Count ExecuteInBases(T& context, V& verb, TTypeList<BASES...> bases) {
		if constexpr (CT::Constant<T>)
			return (Execute<DISPATCH, DEFAULT, FALLBACK>(static_cast<const BASES&>(context), verb) || ...);
		else 
			return (Execute<DISPATCH, DEFAULT, FALLBACK>(static_cast<BASES&>(context), verb) || ...);
	}

	/// Invoke a single verb on a single context											
	///	@attention assumes that if T is deep, then it contains exactly one	
	///		element																				
	///	@tparam DISPATCH - whether or not to use context's dispatcher, if		
	///		any is statically available or reflected									
	///	@tparam DEFAULT - whether or not to attempt default verb execution	
	///		if such is statically available or reflected this is done only		
	///		if direct or dispatched execution fails									
	///	@tparam FALLBACK - for internal use by the function - used to nest	
	///		with default functionality, if DEFAULT is enabled						
	///	@param context - the context in which to execute in						
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	template<bool DISPATCH, bool DEFAULT, bool FALLBACK, CT::Data T, CT::Verb V>
	Count Execute(T& context, V& verb) {
		// Always reset verb progress prior to execution						
		verb.Undo();

		if constexpr (!FALLBACK && DISPATCH && CT::Dispatcher<T>) {
			// Custom reflected dispatcher is available							
			// It's your responsibility to implement it adequately			
			// Keep in mind, that once you declare a custom Do for your		
			// type, you no longer rely on reflected bases' verbs or			
			// default verbs. You must invoke those by yourself in your		
			// dispatcher - the custom dispatcher provides full control		
			DenseCast(context).Do(verb);
		}
		else {
			// Execute verb inside the context directly							
			if constexpr (FALLBACK)
				Verb::GenericExecuteDefault(context, verb);
			else
				Verb::GenericExecuteIn(context, verb);

			// If that fails, attempt in all reflected bases					
			if constexpr (requires { typename T::CTTI_Bases; }) {
				if (!verb.IsDone()) {
					// Context has no abilities, or they failed, so try		
					// with all bases' abilities										
					ExecuteInBases<true, false, FALLBACK>(context, verb, typename T::CTTI_Bases {});
				}
			}

			// If that fails, attempt executing the default verb				
			if constexpr (DEFAULT && !FALLBACK) {
				if (!verb.IsDone()) {
					// Verb wasn't executed neither in current element,		
					// nor in any of its bases, so we resort to the				
					// default abilities													
					Execute<false, false, true>(context, verb);
				}
			}
		}

		return verb.GetSuccesses();
	}

	/// Invoke a verb on a flat context of as much elements as you want			
	/// If an element is not able to execute verb, attempt calling the default	
	/// This should be called only in memory blocks that are flat					
	///	@tparam RESOLVE - whether or not to perform runtime resolve of the	
	///							contexts, getting the most concrete type				
	///	@tparam DISPATCH - whether or not to use custom dispatcher for			
	///							 contexts, if any												
	///	@tparam DEFAULT - whether or not to wllo default/stateless verb		
	///							execution, if all else fails								
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@return the number of successful executions									
	template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true, CT::Deep T, CT::Verb V>
	Count DispatchFlat(T& context, V& verb) {
		if (context.IsEmpty()) {
			// Context is empty, and execution happens only if DEFAULT		
			// verbs are allowed, as a stateless verb execution				
			if constexpr (DEFAULT)
				return Verb::GenericExecuteStateless(verb);
			return 0;
		}

		// Copy the context's state into the output container					
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
				output.SmartPush(Move(verb.GetOutput()));
			}

			if (verb.IsDone())
				++successCount;
		}
		
		if (context.IsOr())
			return verb.template CompleteDispatch<true>(successCount, Abandon(output));
		return verb.template CompleteDispatch<false>(successCount, Abandon(output));
	}

	/// Invoke a verb on a container, that is either deep or flat, either		
	/// AND, or OR. The verb will be executed for each flat element inside		
	/// this block. If a failure occurs inside a scope, that scope will be		
	/// considered failed, unless it's an OR scope - OR scopes stop execution	
	/// right after the first success and fail only	if all branches fail			
	///	@tparam RESOLVE - whether or not to perform runtime resolve of the	
	///							contexts, getting the most concrete type				
	///	@tparam DISPATCH - whether or not to use custom dispatcher for			
	///							 contexts, if any												
	///	@tparam DEFAULT - whether or not to wllo default/stateless verb		
	///							execution, if all else fails								
	///	@param context - the context in which scope will be dispatched to		
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	template<bool RESOLVE = true, bool DISPATCH = true, bool DEFAULT = true, CT::Deep T, CT::Verb V>
	Count DispatchDeep(T& context, V& verb) {
		if (context.IsEmpty()) {
			// Context is empty, and execution happens only if DEFAULT		
			// verbs are allowed, as a stateless verb execution				
			if constexpr (DEFAULT)
				return Verb::GenericExecuteStateless(verb);
			return 0;
		}

		if (context.IsDeep() || context.template Is<Trait>()) {
			// Nest if context is deep, or a trait									
			// Traits are considered deep only when executing in them		
			// There is no escape from this scope									
			Count successCount {};
			auto output = Any::FromState(context);
			for (Count i = 0; i < context.GetCount(); ++i) {
				const auto hits = DispatchDeep<RESOLVE, DISPATCH, DEFAULT>(context.template As<Block>(i), verb);
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
				return verb.template CompleteDispatch<true>(successCount, Abandon(output));
			return verb.template CompleteDispatch<false>(successCount, Abandon(output));
		}

		// If reached, then block is flat											
		// Execute implemented verbs if available, or fallback to			
		// default verbs, eventually													
		return DispatchFlat<RESOLVE, DISPATCH, DEFAULT>(context, verb);
	}

} // namespace Langulus::Flow


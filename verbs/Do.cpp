#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_DISPATCH(a)	// Logger::Verbose() << a

namespace Langulus::Flow
{

	/// Invoke a verb on an empty context													
	/// Only default verbs will be called													
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	/*Count Verb::DispatchEmpty(Verb& verb) {
		Any emptyContext;
		return Verb::DefaultDo(emptyContext, verb);
	}*/

	/// Invoke a verb on a container, that is either deep or flat, either		
	/// AND, or OR. The verb will be executed for each flat element inside		
	/// this block. If a failure occurs inside a scope, that scope will be		
	/// considered failed, unless it's an OR scope - OR scopes stop execution	
	/// right after the first success and fail only	if all branches fail			
	///	@param context - the context in which scope will be executed			
	///	@param verb - the verb to execute												
	///	@param resolveElements - whether or not the most concrete type of		
	///			 an element will be used - useful to resolve sparse elements	
	///	@return the number of successful executions									
	/*Count Verb::DispatchDeep(Block& context, Verb& verb, bool resolveElements, bool allowCustomDispatch, bool allowDefaultVerbs) {
		if (context.IsDeep() || context.Is<Trait>()) {
			// Nest if context is deep, or a trait									
			// Traits are considered deep only when executing in them		
			// There is no escape from this scope									
			Count successCount {};
			auto output = Any::FromState(context);
			for (Count i = 0; i < context.GetCount(); ++i) {
				const auto hits = DispatchDeep(context.As<Block>(i), verb, resolveElements, allowCustomDispatch, allowDefaultVerbs);
				successCount += hits;
				if (verb.mShortCircuited) {
					// Short-circuit if enabled for verb							
					if (context.IsOr() == (successCount > 0)) {
						// It is time for an early return							
						// Will fail on first AND-failure							
						// Will succeed on first OR-success							
						if (context.IsOr()) {
							// OR-Success													
							// Will carry its own output								
							verb.mSuccesses = successCount;
						}
						else {
							// AND-Failure													
							// All outputs are discarded								
							verb.mOutput.Reset();
							verb.mSuccesses = 0;
						}

						return verb.mSuccesses;
					}
				}

				// Cache each output, conserving the context hierarchy		
				if (hits && !verb.mOutput.IsEmpty())
					output << Move(verb.mOutput);
			}

			if (verb.mShortCircuited) {
				// If reached, this will result in failure if OR, or			
				// success if AND, as long as the verb is short-circuited	
				verb.mSuccesses = context.IsOr() ? 0 : successCount;
			}
			else {
				// If verb is not short-circuited, then a single success		
				// is enough																
				verb.mSuccesses = successCount;
			}

			// Set the output																
			if (verb.mSuccesses) {
				output.Optimize();
				verb.mOutput = Move(output);
			}
			else verb.mOutput.Reset();

			// Done																			
			return verb.mSuccesses;
		}

		// If reached, then block is flat											
		// Execute implemented verbs if available, or fallback to			
		// default verbs																	
		return DispatchFlat(context, verb, resolveElements, allowCustomDispatch, allowDefaultVerbs);
	}*/

	/// Invoke a verb on a flat context of as much elements as you want			
	/// If an element is not able to execute verb, attempt calling default verb
	/// This should be called only in memory blocks that are flat					
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@param resolve - whether or not the most concrete type of				
	///			 an element will be used - useful to resolve elements				
	///	@return the number of successful executions									
	/*Count Verb::DispatchFlat(Block& context, Verb& verb, bool resolve, bool allowCustomDispatch, bool allowDefaultVerbs) {
		SAFETY(if (resolve && context.IsDeep()) Throw<Except::Flow>());

		if (context.IsEmpty()) {
			if (allowDefaultVerbs)
				return Verb::DefaultDo(context, verb);
			return 0;
		}

		// Iterate elements in the current context								
		Count successCount {};
		auto output = Any::FromState(context);
		for (Count i = 0; i < context.GetCount(); ++i) {
			// Reset verb to initial state											
			auto resolved = resolve
				? context.GetElementResolved(i) 
				: context.GetElementDense(i);

			verb.Undo();

			VERBOSE_DISPATCH("Dispatching " << verb.GetToken()
				<< " to element " << (i + 1) << " (" << resolved.GetToken() << ") ");

			auto customDispatcher = resolved.GetType()->GetDispatcher();
			if (allowCustomDispatch && customDispatcher) {
				// Resolved element has a custom dispatcher						
				// It's your responsibility to implement it adequately		
				// Keep in mind, that once you declare a custom Do for your	
				// type, you no longer rely on reflected bases' verbs or		
				// default verbs. You must invoke those by yourself in your	
				// dispatcher - custom dispatcher provides full control		
				customDispatcher(resolved.GetRaw(), verb);
			}
			else {
				// Scan the reflected abilities										
				auto& abilities = resolved.GetType()->mAbilities;
				auto found = abilities.find(verb.GetVerb());
				if (found != abilities.end()) {
					found->second.mFunction(resolved.GetRaw(), verb);
					if (verb.IsDone())
						break;
				}

				if (!verb.IsDone()) {
					// Context has no abilities, or they failed, so try with	
					// all bases' abilities												
					for (auto& base : resolved.GetType()->mBases) {
						if (base.mType->mIsDeep || base.mCount > 1)
							continue; // TODO handle batchable

						VERBOSE_DISPATCH(Logger::Cyan
							<< " (attempting execution in context base "
							<< base.mType << ") ");
						verb.Undo();

						auto baseBlock = resolved.GetBaseMemory(base.mType, base);
						Verb::DispatchFlat(baseBlock, verb, false, true, false);
						if (verb.IsDone())
							break;
					}
				}

				if (!verb.IsDone() && allowDefaultVerbs) {
					// Verb wasn't executed neither in current element, nor	
					// in	any of its bases, so we resort to the default		
					// abilities															
					if (Verb::DefaultDo(resolved, verb))
						verb.Done();

					if (!verb.IsDone()) {
						// Default ability didn't do anything, so we try all	
						// default abilities in all bases							
						for (auto& base : resolved.GetType()->mBases) {
							if (base.mType->mIsDeep || base.mCount > 1)
								continue; // TODO handle batchable

							VERBOSE_DISPATCH(Logger::Cyan 
								<< " (attempting DEFAULT execution in context base "
								<< base.mType << ") ");
							verb.Undo();

							auto baseBlock = resolved.GetBaseMemory(base.mType, base);
							if (Verb::DefaultDo(baseBlock, verb)) {
								verb.Done();
								break;
							}
						}
					}
				}
			}

			if (verb.mShortCircuited) {
				// Short-circuit if enabled for verb								
				if (verb.IsDone() == context.IsOr()) {
					// Time to early-exit												
					// Will fail on first AND-failure								
					// Will succeed on first OR-success								
					if (context.IsOr()) {
						// OR-Success														
						// Will carry its own output, no need to use cache		
						verb.mSuccesses = 1;
					}
					else {
						// AND-Failure														
						// All outputs are discarded									
						verb.mSuccesses = 0;
						verb.mOutput.Reset();
					}

					return verb.mSuccesses;
				}
			}
			
			if (verb.IsDone() && !verb.mOutput.IsEmpty()) {
				// Cache output, conserving the context hierarchy				
				output << Move(verb.mOutput);
			}

			if (verb.IsDone())
				++successCount;
		}
		
		if (verb.mShortCircuited) {
			// If reached, this will result in failure in OR-context, or	
			// success if AND, as long as the verb is short-circuited		
			verb.mSuccesses = context.IsOr() ? 0 : successCount;
		}
		else {
			// If verb is not short-circuited, then a single success			
			// is enough																	
			verb.mSuccesses = successCount;
		}

		// Set output																		
		if (verb.mSuccesses) {
			output.Optimize();
			verb.mOutput = Move(output);
		}
		else verb.mOutput.Reset();

		// Restore the backed-up source												
		return verb.mSuccesses;
	}*/



} // namespace Langulus::Flow


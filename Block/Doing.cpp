#include "../Code.hpp"
#include "IncludeLogger.hpp"

#define VERBOSE_DISPATCH(a) //pcLogVerbose << a
#define VERBOSE_FLOW(a) //pcLogVerbose << a
#define VERBOSE_FLOW_TAB(a) //ScopedTab tab; pcLogVerbose << a << tab
#define FLOW_ERRORS(a) //pcLogError << a

namespace Langulus::Flow
{

	/// Call the default built-in memory abilities										
	/// This should be called only in memory blocks that are flat and contain	
	/// only a single element																	
	///	@param context - the context in which scope will be executed			
	///	@param verb - the verb to send over												
	///	@return true on success																
	bool Verb::DefaultDo(Block& context, Verb& verb) {
		SAFETY(if (context.GetCount() > 1)
			throw Except::BadOperation(pcLogFuncError
				<< "Should operate on single/empty instances only, but "
				<< context.GetCount() << " instead"));

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
	}

	/// Invoke a verb on an empty context													
	/// Only default verbs will be called													
	///	@param verb - the verb to execute												
	///	@return the number of successful executions									
	pcptr Verb::DispatchEmpty(Verb& verb) {
		Any emptyContext;
		return Verb::DefaultDo(emptyContext, verb);
	}

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
	pcptr Verb::DispatchDeep(Block& context, Verb& verb, bool resolveElements, bool allowCustomDispatch, bool allowDefaultVerbs) {
		if (context.IsDeep() || context.Is<Trait>()) {
			// Nest if context is deep, or a trait									
			// Traits are considered deep only when executing in them		
			// There is no escape from this scope									
			pcptr successCount = 0;
			auto output = Any::FromStateOf(context);
			for (pcptr i = 0; i < context.GetCount(); ++i) {
				const auto hits = DispatchDeep(context.As<Block>(i), verb, resolveElements, allowCustomDispatch, allowDefaultVerbs);
				successCount += hits;
				if (verb.mVerb.mShortCircuited) {
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
					output << pcMove(verb.mOutput);
			}

			if (verb.mVerb.mShortCircuited) {
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
				verb.mOutput = pcMove(output);
			}
			else verb.mOutput.Reset();

			// Done																			
			return verb.mSuccesses;
		}

		// If reached, then block is flat											
		// Execute implemented verbs if available, or fallback to			
		// default verbs																	
		return DispatchFlat(context, verb, resolveElements, allowCustomDispatch, allowDefaultVerbs);
	}

	/// Invoke a verb on a flat context of as much elements as you want			
	/// If an element is not able to execute verb, attempt calling default verb
	/// This should be called only in memory blocks that are flat					
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@param resolve - whether or not the most concrete type of				
	///			 an element will be used - useful to resolve elements				
	///	@return the number of successful executions									
	pcptr Verb::DispatchFlat(Block& context, Verb& verb, bool resolve, bool allowCustomDispatch, bool allowDefaultVerbs) {
		SAFETY(if (resolve && context.IsDeep())
			throw Except::BadOperation());

		if (context.IsEmpty()) {
			if (allowDefaultVerbs) {
				// Only default verbs can be called on empty contexts			
				return Verb::DefaultDo(context, verb);
			}
			else return 0;
		}

		// Iterate elements in the current context								
		pcptr successCount = 0;
		auto output = Any::FromStateOf(context);
		for (pcptr i = 0; i < context.GetCount(); ++i) {
			// Reset verb to initial state											
			auto resolved = resolve
				? context.GetElementResolved(i) 
				: context.GetElementDense(i);

			verb.Undo();

			VERBOSE_DISPATCH("Dispatching " << verb.GetToken()
				<< " to element " << (i + 1) << " (" << resolved.GetToken() << ") ");

			auto customDispatcher = resolved.GetMeta()->GetDispatcher();
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
				for (auto& ability : resolved.GetMeta()->GetAbilityList()) {
					if (ability.mStaticAbility.mVerb != verb.GetID())
						continue;

					ability.mStaticAbility.mFunction(resolved.GetRaw(), verb);
					if (verb.IsDone())
						break;
				}

				if (!verb.IsDone()) {
					// Context has no abilities, or they failed, so try with	
					// all bases' abilities												
					for (auto& base : resolved.GetMeta()->GetBaseList()) {
						if (base.mBase->IsDeep() || base.mStaticBase.mCount > 1)
							continue; // TODO handle batchable

						VERBOSE_DISPATCH(ccCyan << " (attempting execution in context base "
							<< base.mBase << ") ");
						verb.Undo();
						auto baseBlock = resolved.GetBaseMemory(base.mBase, base);
						Verb::DispatchFlat(baseBlock, verb, false, true, false);
						if (verb.IsDone())
							break;
					}
				}

				if (!verb.IsDone() && allowDefaultVerbs) {
					// Verb wasn't executed neither in current element, nor in	
					// any of its bases, so we resort to the default abilities	
					if (Verb::DefaultDo(resolved, verb))
						verb.Done();

					if (!verb.IsDone()) {
						// Default ability didn't do anything, so we try all		
						// default abilities in all bases								
						for (auto& base : resolved.GetMeta()->GetBaseList()) {
							if (base.mBase->IsDeep() || base.mStaticBase.mCount > 1)
								continue; // TODO handle batchable

							VERBOSE_DISPATCH(ccCyan << " (attempting DEFAULT execution in context base "
								<< base.mBase << ") ");
							verb.Undo();
							auto baseBlock = resolved.GetBaseMemory(base.mBase, base);
							if (Verb::DefaultDo(baseBlock, verb)) {
								verb.Done();
								break;
							}
						}
					}
				}
			}

			if (verb.mVerb.mShortCircuited) {
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
				output << pcMove(verb.mOutput);
			}

			if (verb.IsDone())
				++successCount;
		}
		
		if (verb.mVerb.mShortCircuited) {
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
			verb.mOutput = pcMove(output);
		}
		else verb.mOutput.Reset();

		// Restore the backed-up source												
		return verb.mSuccesses;
	}

	/// Nested AND/OR scope execution with output										
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@return true of everything is alright											
	bool Verb::ExecuteScope(Any& context, const Any& scope, Any& output) {
		bool skipVerbs = false;
		return ExecuteScope(context, scope, output, skipVerbs);
	}

	/// Nested AND/OR scope execution with output										
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [out] whether to skip verbs after OR success		
	///	@return true of everything is alright											
	bool Verb::ExecuteScope(Any& context, const Any& scope, Any& output, bool& skipVerbs) {
		// Execute either an AND, or an OR scope									
		bool executed = true;
		auto results = Any::FromStateOf(scope);
		if (!scope.IsEmpty()) {
			VERBOSE_FLOW_TAB("Executing scope: " << scope);
			if (scope.IsOr() && scope.GetCount() > 1)
				executed = Verb::ExecuteScopeOR(context, scope, results, skipVerbs);
			else
				executed = Verb::ExecuteScopeAND(context, scope, results, skipVerbs);
		}

		// Propagate the results														
		if (executed && !results.IsEmpty()) {
			results.Optimize();
			output = pcMove(results);
		}

		return executed;
	}

	/// Nested AND/OR scope execution (discarding outputs)							
	/// TODO optimize for unneeded outputs													
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@return true of everything is alright											
	bool Verb::ExecuteScope(Any& context, const Any& scope) {
		Any output;
		return Verb::ExecuteScope(context, scope, output);
	}

	/// Nested AND scope execution															
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [out] whether to skip verbs (after an OR success)	
	///	@param scopeOverwritesContext - [out] whether to overwrite context	
	///	@return true of everything is alright											
	bool Verb::ExecuteScopeAND(Any& context, const Any& scope, Any& output, bool& skipVerbs) {
		if (scope.IsDeep()) {
			// Nest if deep																
			for (uint i = 0; i < scope.GetCount(); ++i) {
				Any localOutput;
				if (!Verb::ExecuteScope(
					context, scope.As<Any>(i), localOutput, skipVerbs)
				) {
					VERBOSE_FLOW(ccRed << "Deep-AND failed: " << scope);
					return false;
				}

				output << pcMove(localOutput);
			}
		}
		else if (scope.Is<Trait>()) {
			// Nest if traits, but retain each trait 								
			for (uint i = 0; i < scope.GetCount(); ++i) {
				auto& trait = scope.Get<Trait>(i);
				Any localOutput;
				if (!Verb::ExecuteScope(
					context, trait, localOutput, skipVerbs)
				) {
					VERBOSE_FLOW(ccRed << "Trait-AND failed: " << scope);
					return false;
				}

				output << Trait { trait.GetTraitMeta(), pcMove(localOutput) };
			}
		}
		else if (scope.Is<Construct>()) {
			// Nest if constructs, but retain each construct					
			for (uint i = 0; i < scope.GetCount(); ++i) {
				auto& construct = scope.Get<Construct>(i);
				Any localOutput;
				if (!Verb::ExecuteScope(
					context, construct.GetAll(), localOutput, skipVerbs)
				) {
					VERBOSE_FLOW(ccRed << "Construct-AND failed: " << scope);
					return false;
				}

				Construct newc { construct.GetMeta(), pcMove(localOutput) };
				newc.mCharge = construct.mCharge;

				try {
					// Attempt constructing the construct here if possible	
					newc.StaticCreation(localOutput);
					output << pcMove(localOutput);
				}
				catch (const Except::BadConstruction&) {
					// Construction failed, so just propagate construct		
					output << pcMove(newc);
				}
			}
		}
		else if (scope.Is<Verb>()) {
			if (skipVerbs)
				return false;

			// Scary cast, but should be alright									
			SAFETY(if (scope.IsSparse()) throw Except::BadAccess();)
			auto& asVerbs = pcReinterpret<Script>(scope);
			for (auto& constVerb : asVerbs) {
				// Shallow-copy the verb to make it modifiable					
				Verb verb {
					constVerb.GetChargedID(),
					constVerb.mSource,
					constVerb.mArgument
				};

				// Check if verb outputs to context									
				const bool verbOverwritesContext = 
					constVerb.OutputsTo<Traits::Context>();

				// Execute the verb														
				if (!Verb::ExecuteVerb(context, verb)) {
					VERBOSE_FLOW(ccRed << "Verb-AND failed: " << scope);
					return false;
				}

				if (!verb.mOutput.IsEmpty()) {
					if (verbOverwritesContext) {
						// Substitute local environment if required				
						context = verb.mOutput;
						output = pcMove(verb.mOutput);
						VERBOSE_FLOW(ccCyan << "Context changed to: " << context);
					}
					else output << pcMove(verb.mOutput);
				}
			}
		}
		else {
			// If this is reached, then we have non-verb content				
			// Just propagate content													
			output << scope;
		}

		VERBOSE_FLOW(ccGreen << "And-Scope done: " << scope);
		return true;
	}

	/// Nested OR execution																		
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [out] whether to skip verbs after OR success		
	///	@return true if everything is alright											
	bool Verb::ExecuteScopeOR(Any& context, const Any& scope, Any& output, bool& skipVerbs) {
		bool executed = false;
		bool localSkipVerbs = false;

		if (scope.IsDeep()) {
			// DEEP OR																		
			// Execute in order until a successful execution occurs,			
			// then skip verbs - only collect data									
			Any contextSubstitution;
			for (uint i = 0; i < scope.GetCount(); ++i) {
				Any localContext { context };
				Any localOutput;
				if (Verb::ExecuteScope(
					localContext, scope.As<Any>(i), localOutput, localSkipVerbs)
				) {
					// Do a simple check for a changed context					
					if (localContext.GetRaw() != context.GetRaw() || localContext.GetCount() != context.GetCount())
						contextSubstitution = pcMove(localContext);

					executed = true;
					if (!localOutput.IsEmpty())
						output << pcMove(localOutput);
				}
			}

			// If any context substitution happend, now is the time to		
			// apply it - after all branches had been executed in the		
			// original context															
			if (!contextSubstitution.IsEmpty()) {
				context = pcMove(contextSubstitution);
				VERBOSE_FLOW(ccCyan << "Context changed to: " << context);
			}

			skipVerbs = localSkipVerbs;
		}
		else if (scope.Is<Trait>()) {
			// All traits get executed, but the failed scope traits get		
			// discarded																	
			for (uint i = 0; i < scope.GetCount(); ++i) {
				bool unusedSkipVerbs = false;
				Any localContext { context };
				Any localOutput;
				auto& trait = scope.Get<Trait>(i);
				if (Verb::ExecuteScope(
					localContext, trait, localOutput, unusedSkipVerbs)
				) {
					executed = true;
					output << Trait { trait.GetTraitMeta(), pcMove(localOutput) };
				}
			}
		}
		else if (scope.Is<Construct>()) {
			// All constructs get executed, but the failed ones get			
			// discarded																	
			for (uint i = 0; i < scope.GetCount(); ++i) {
				bool unusedSkipVerbs = false;
				Any localContext { context };
				Any localOutput;
				auto& construct = scope.Get<Construct>(i);
				if (Verb::ExecuteScope(
					localContext, construct.GetAll(), localOutput, unusedSkipVerbs)
				) {
					executed = true;
					Construct newc { construct.GetMeta(), pcMove(localOutput) };
					newc.mCharge = construct.mCharge;
					output << pcMove(newc);
				}
			}
		}
		else if (scope.Is<Verb>()) {
			// SHALLOW OR																	
			// We couldn't pick a verb, so execute in order until	a			
			// successful execution occurs. On success simply skip other	
			// verbs - only collect data along the way (using skipVerbs)	
			if (localSkipVerbs) {
				VERBOSE_FLOW(ccDarkYellow << "OR-Scope skipped: " << scope);
				return false;
			}

			// Scary cast, but should be alright									
			Any contextSubstitution;
			SAFETY(if (scope.IsSparse()) throw Except::BadAccess();)
			auto& asVerbs = pcReinterpret<Script>(scope);
			for (auto& constVerb : asVerbs) {
				Verb verb { 
					constVerb.GetChargedID(), 
					constVerb.mSource, 
					constVerb.mArgument
				};

				// Check if verb outputs to context									
				const bool verbOverwritesContext = 
					constVerb.OutputsTo<Traits::Context>();

				Any localContext { context };
				if (!Verb::ExecuteVerb(localContext, verb))
					continue;

				executed = true;
				if (!verb.mOutput.IsEmpty()) {
					if (verbOverwritesContext)
						contextSubstitution = verb.mOutput;
					output << pcMove(verb.mOutput);
				}
			}

			// If any context substitution happened, now is the time to		
			// apply it - after all branches had been executed in the		
			// original context															
			if (!contextSubstitution.IsEmpty()) {
				context = pcMove(contextSubstitution);
				VERBOSE_FLOW(ccCyan << "Context changed to: " << context);
			}

			skipVerbs = localSkipVerbs;
		}
		else {
			// If this is reached, then we have non-verb flat content		
			// Just propagate it															
			output << scope;
			executed = true;
		}

		if (executed)
			VERBOSE_FLOW(ccGreen << "OR-Scope done: " << scope);
		else
			VERBOSE_FLOW(ccRed << "OR-Scope failed: " << scope);
		return executed;
	}

	/// Execute all verbs inside a scope, but capsulate the results				
	/// If the scope is not made of verbs, then data is simply propagated		
	///	@param context - [in/out] the context where scope will be integrated	
	///	@param scope - [in/out] scope to integrate									
	///	@return true if everything is alright											
	bool Verb::IntegrateScope(Any& context, Any& scope) {
		if (!Verb::IsScopeExecutableDeep(scope))
			return true;

		bool unusedSkipVerbs = false;
		Any localOutput;
		if (Verb::ExecuteScope(context, scope, localOutput, unusedSkipVerbs)) {
			scope = pcMove(localOutput);
			return true;
		}

		return false;
	}

	/// Integrate all parts of a verb inside this environment						
	///	@param context - [in/out] the context where verb will be integrated	
	///	@param verb - [in/out] verb to integrate										
	///	@return true if everything is alright											
	bool Verb::IntegrateVerb(Any& context, Verb& verb) {
		// Integrate the verb source to the current context					
		// This might substitute the context										
		if (!Verb::IntegrateScope(context, verb.mSource)) {
			pcLogError << "Error at source: " << verb.mSource;
			return false;
		}

		if (verb.mSource.IsEmpty())
			verb.mSource = context;

		// Integrate the verb argument to the source								
		// This also might substitute the context									
		Any localContext { verb.mSource };
		if (!Verb::IntegrateScope(localContext, verb.mArgument)) {
			pcLogError << "Error at argument: " << verb.mArgument;
			return false;
		}

		// Do context substitution on change										
		if (localContext.GetRaw() != verb.mSource.GetRaw() || localContext.GetCount() != verb.mSource.GetCount())
			context = pcMove(localContext);
		return true;
	}

	/// Execute a single verb, and all subverbs in it, if any						
	///	@param context - [in/out] the context in which verb will be executed	
	///	@param verb - [in/out] verb to execute											
	///	@return true if verb was integrated and satisfied							
	bool Verb::ExecuteVerb(Any& context, Verb& verb) {
		// Integration (and execution of subverbs if any)						
		// Source, and argument will be executed locally 						
		if (!IntegrateVerb(context, verb)) {
			FLOW_ERRORS("Error integrating verb: " << verb);
			return false;
		}

		// At this point, context might contain context substitution,		
		// while verb will contain the outputs of its source & arguments	
		if (verb.Is<Verbs::Do>()) {
			// A Do verb is done at this point, because the subverbs 		
			// inside (if any) should be done in the integration phase		
			// Just making sure that the integrated output/argument/source 
			// are propagated to the verb's output									
			if (verb.mOutput.IsEmpty()) {
				if (!verb.mArgument.IsEmpty())
					verb << verb.mArgument;
				else
					verb << verb.mSource;
			}

			return true;
		}

		VERBOSE_FLOW_TAB("Executing " << ccCyan << verb);

		// Dispatch the verb to the context, executing it						
		// Any results should be inside verb.mOutput								
		if (!DispatchDeep(verb.mSource, verb)) {
			FLOW_ERRORS("Error executing verb: " << verb);
			return false;
		}

		VERBOSE_FLOW("Executed: " << ccGreen << verb);
		return true;
	}

} // namespace Langulus::Flow


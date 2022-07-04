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

	template<CT::Void T, CT::Verb V, bool CUSTOMDISPATCH, bool DEFAULTVERB>
	Count TExecute(T& context, V& verb) {
		if constexpr (DEFAULTVERB)
			return Verb::DefaultDo(context, verb);
		else
			return 0;
	}

	/// Invoke a static verb on a static type												
	///	@tparam RESOLVE - perform runtime type-checking and execute in the	
	///							most concrete result											
	///	@param context - the context in which to dispatch the verb				
	///	@param verb - the verb to send over												
	///	@return the number of successful executions									
	template<CT::Data T, CT::Verb V, bool CUSTOMDISPATCH, bool DEFAULTVERB>
	Count TExecute(T& context, V& verb) {
		static_assert(!CT::Deep<T>, "T must be flat");

		verb.Undo();
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
			auto found = abilities.find(verb.GetToken());
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
	}

} // namespace Langulus::Flow


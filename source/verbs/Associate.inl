///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "../Code.hpp"
#include "Do.inl"
#include "Interpret.inl"
#include "Catenate.inl"

#define VERBOSE_ASSOCIATE(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Associate::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Associate(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Associate(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Associate::Of() noexcept {
		if constexpr (!Associate::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Associate(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Associate(verb, args...);
			};
		}
	}

	/// Execute the association/dissociation verb in a specific context			
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Associate::ExecuteIn(T& context, Verb& verb) {
		static_assert(Associate::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Associate(verb);
		return verb.IsDone();
	}

	/// Execute the default verb in a context												
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Associate::ExecuteDefault(Block& context, Verb& verb) {
		if (context.IsConstant() || context.IsMissing() || verb.IsMissingDeep() || !context.Is(verb.GetType()))
			return false;

		// Attempt directly copying, if possible									
		// This will happen only if types are exactly the same				
		// This is default, fallback routine, let's keep things simple		
		try { verb.Copy(context); }
		catch (const Except::Copy&) {
			return false;
		}

		// At this point, context has a copy of verb's argument				
		// Just make sure it goes to output											
		verb << context;
		return true;

		// Collect all viably typed or interpreted stuff from argument		
		/*Any result;
		bool atLeastOneSuccess {};
		verb.ForEachDeep([&](const Block& group) {
			const auto done = group.ForEach(
				// Nest inside traits manually here, because traits			
				// aren't considered deep containers otherwise					
				[&](const Trait& trait) {
					VERBOSE_ASSOCIATE("Default associating trait "
						<< context << " with " << Logger::Cyan << trait);
					auto nested = verb.PartialCopy().SetArgument(static_cast<const Any&>(trait));
					Associate::ExecuteDefault(context, nested);
					atLeastOneSuccess |= nested.IsDone();
				},
				// Nest inside constructs manually here, because they			
				// aren't considered deep containers, also						
				[&](const Construct& construct) {
					construct.ForEach([&](const Trait& trait) {
						VERBOSE_ASSOCIATE("Default associating trait (from request) " 
							<< context << " with " << Logger::Cyan << trait);
						auto nested = verb.PartialCopy().SetArgument(trait);
						Associate::ExecuteDefault(context, nested);
						atLeastOneSuccess |= nested.IsDone();
					});
				}
			);

			if (done)
				return;

			try {
				// Attempt directly copying, if possible							
				atLeastOneSuccess |= group.Copy(context) > 0;
			}
			catch (const Except::Copy&) {
				// Attempt interpretation												
				if (!group.Is(result.GetType()) || !result.InsertBlock(group)) {
					VERBOSE_ASSOCIATE("Attempting interpretation of "
						<< group << " to " << context.GetMeta());

					Verbs::Interpret interpreter(context.GetType());
					if (DispatchFlat<true, true, false>(group, interpreter)) {
						VERBOSE_ASSOCIATE("Interpreted " << group << " as " << Logger::Cyan
							<< interpreter.GetOutput() << " -- from "
							<< group.GetMeta() << " to " << context.GetType());
						result << Move(interpreter.GetOutput());
					}
				}
			}
		});

		// Concatenate and/or copy the found stuff								
		if (!result.IsEmpty()) {
			result.Optimize();
			VERBOSE_ASSOCIATE("Attempting ovewriting " 
				<< context << " with " << Logger::Cyan << result);

			try {
				atLeastOneSuccess |= result.Copy(context) > 0;
			}
			catch (const Except::Copy&) {
				// Catenate results into one element, and then copy			
				Verbs::Catenate catenater(result);
				auto catenated = Any::FromBlock(context, DataState::Typed);
				catenated.Allocate<true>(1);

				if (DispatchFlat<true, true, false>(catenated, catenater)) {
					// Success																
					atLeastOneSuccess |= catenater.GetOutput().Copy(context) > 0;
				}
				else {
					// Failure																
					VERBOSE_ASSOCIATE(Logger::Red << "Can't overwrite " << context
						<< " with badly catenated " << result);
				}
			}
		}

		// An association verb always pushes context as output				
		if (atLeastOneSuccess) {
			verb << Any {context};
			return true;
		}

		return false;*/
	}

} // namespace Langulus::Verbs


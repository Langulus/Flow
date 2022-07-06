#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_ASSOCIATE(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Associate verb construction															
	///	@param s - what are we associating?												
	///	@param a - what are we associating with?										
	///	@param o - result mask (optional)												
	///	@param c - the charge of the association										
	///	@param sc - is the association short-circuited								
	inline Associate::Associate(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Associate>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T>
	constexpr bool Associate::AvailableFor() noexcept {
		return requires (T t, Verb v) { t.Associate(v); };
	}

	/// Execute the association verb in a specific context							
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Associate::ExecuteIn(T& context, Verb& verb) {
		if constexpr (Associate::AvailableFor<T>()) {
			context.Associate(verb);
			return verb.IsDone();
		}
		else return false;
	}

	/// Execute the default verb in a context												
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Associate::ExecuteDefault(Block& context, Verb& verb) {
		// Collect all viably typed or interpreted stuff from argument		
		Any result;
		bool atLeastOneSuccess {};
		verb.GetArgument().ForEachDeep([&](const Block& group) {
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

					Verbs::Interpret interpreter({}, context.GetType());
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
				Verbs::Catenate catenater({}, result);
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

		return false;
	}

} // namespace Langulus::Verbs


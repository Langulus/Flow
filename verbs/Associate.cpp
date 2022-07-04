#include "../Code.hpp"

#define VERBOSE_ASSOCIATE(a) //Logger::Verbose() << a

namespace Langulus::Flow
{

	/// Default association																		
	/// Attempt a direct copy first, and if that fails - attempts interpret		
	///	@param context - the block to execute in										
	///	@param verb - association verb													
	void Verb::DefaultAssociate(Block& context, Verb& verb) {
		if (context.IsAbstract()) {
			VERBOSE_ASSOCIATE(Logger::Red << "Can't " << verb << " because "
				<< context << " is abstract");
			return;
		}
		else if (context.IsEmpty()) {
			VERBOSE_ASSOCIATE(Logger::Red << "Can't " << verb << " because "
				<< context << " is empty");
			return;
		}
		else if (context.IsConstant()) {
			VERBOSE_ASSOCIATE(Logger::Red << "Can't " << verb << " because "
				<< context << " is immutable");
			return;
		}

		// Collect all viably typed or interpreted stuff from argument		
		Any result;
		bool atLeastOneSuccess {};
		verb.GetArgument().ForEachDeep([&](const Block& group) {
			EitherDoThis
				// Nest inside traits manually here, because traits aren't	
				// considered deep containers otherwise							
				group.ForEach([&](const Trait& trait) {
					VERBOSE_ASSOCIATE("Default associating trait "
						<< context << " with " << Logger::Cyan << trait);
					auto nested = verb.PartialCopy()
						.SetArgument(static_cast<const Any&>(trait));
					Verb::DefaultAssociate(context, nested);
					atLeastOneSuccess |= nested.IsDone();
				})
			OrThis
				// Nest inside constructs manually here, because they			
				// aren't considered deep containers, also						
				group.ForEach([&](const Construct& construct) {
					construct.GetAll().ForEach([&](const Trait& trait) {
						VERBOSE_ASSOCIATE("Default associating trait (from request) " 
							<< context << " with " << Logger::Cyan << trait);
						auto nested = verb.PartialCopy()
							.SetArgument(static_cast<const Any&>(trait));
						Verb::DefaultAssociate(context, nested);
						atLeastOneSuccess |= nested.IsDone();
					});
				})
			AndReturnIfDone;

			try {
				// Attempt directly copying, if possible							
				atLeastOneSuccess |= group.Copy(context) > 0;
			}
			catch (const Except::Copy&) {
				// Attempt interpretation												
				if (!group.Is(result.GetType()) || !result.InsertBlock(group)) {
					VERBOSE_ASSOCIATE("Attempting interpretation of "
						<< group << " to " << context.GetMeta());

					auto interpret = Verbs::Interpret({}, context.GetType());
					if (Verb::DispatchFlat(const_cast<Block&>(group), interpret)) {
						VERBOSE_ASSOCIATE("Interpreted " << group << " as " << Logger::Cyan
							<< interpret.GetOutput() << " -- from "
							<< group.GetMeta() << " to " << context.GetType());
						result << Move(interpret.GetOutput());
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
				auto cat = Verbs::Catenate({}, result);
				auto catenated = Any::FromBlock(context, DataState::Typed);
				catenated.Allocate<true>(1);

				if (Verb::DispatchFlat(catenated, cat)) {
					// Success																
					atLeastOneSuccess |= cat.GetOutput().Copy(context) > 0;
				}
				else {
					// Failure																
					VERBOSE_ASSOCIATE(Logger::Red << "Can't overwrite " << context
						<< " with badly catenated " << result);
				}
			}
		}

		// An association verb always pushes context as output				
		if (atLeastOneSuccess)
			verb << Any {context};
	}

} // namespace Langulus::Flow


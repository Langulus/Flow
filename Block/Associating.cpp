#include "../Code.hpp"
#include "IncludeLogger.hpp"

#define VERBOSE_ASSOCIATE(a) //pcLogVerbose << a

namespace Langulus::Flow
{

	/// Default association																		
	/// Attempt a direct copy first, and if that fails - attempts interpret		
	///	@param context - the block to execute in										
	///	@param verb - association verb													
	void Verb::DefaultAssociate(Block& context, Verb& verb) {
		if (context.IsAbstract()) {
			VERBOSE_ASSOCIATE(ccRed << "Can't " << verb << " because " 
				<< context << " is abstract");
			return;
		}
		else if (context.IsEmpty()) {
			VERBOSE_ASSOCIATE(ccRed << "Can't " << verb << " because " 
				<< context << " is empty");
			return;
		}
		else if (context.IsConstant()) {
			VERBOSE_ASSOCIATE(ccRed << "Can't " << verb << " because " 
				<< context << " is immutable");
			return;
		}

		// Collect all viably typed or interpreted stuff from argument		
		Any result;
		SuccessTrap atLeastOneSuccess;
		verb.GetArgument().ForEachDeep([&](const Block& group) {
			EitherDoThis
				// Nest inside traits manually here, because traits aren't	
				// considered deep containers otherwise							
				group.ForEach([&](const Trait& trait) {
					VERBOSE_ASSOCIATE("Default associating trait "
						<< context << " with " << ccCyan << trait);
					auto nested = verb.PartialCopy()
						.SetArgument(static_cast<const Any&>(trait));
					Verb::DefaultAssociate(context, nested);
					atLeastOneSuccess = nested.IsDone();
				})
			OrThis
				// Nest inside constructs manually here, because they			
				// aren't considered deep containers, also						
				group.ForEach([&](const Construct& construct) {
					construct.GetAll().ForEach([&](const Trait& trait) {
						VERBOSE_ASSOCIATE("Default associating trait (from request) " 
							<< context << " with " << ccCyan << trait);
						auto nested = verb.PartialCopy()
							.SetArgument(static_cast<const Any&>(trait));
						Verb::DefaultAssociate(context, nested);
						atLeastOneSuccess = nested.IsDone();
					});
				})
			AndReturnIfDone;

			try {
				// Attempt directly copying, if possible							
				atLeastOneSuccess = group.Copy(context) > 0;
			}
			catch (const Except::BadCopy&) {
				// Attempt interpretation												
				if (!group.Is(result.GetDataID()) || !result.InsertBlock(group)) {
					VERBOSE_ASSOCIATE("Attempting interpretation of "
						<< group << " to " << context.GetMeta());

					auto interpret = Verb::From<Verbs::Interpret>({}, context.GetMeta());
					if (Verb::DispatchFlat(const_cast<Block&>(group), interpret)) {
						VERBOSE_ASSOCIATE("Interpreted " << group << " as " << ccCyan
							<< interpret.GetOutput() << " -- from "
							<< group.GetMeta() << " to " << context.GetMeta());
						result << pcMove(interpret.GetOutput());
					}
				}
			}
		});

		// Concatenate and/or copy the found stuff								
		if (!result.IsEmpty()) {
			result.Optimize();
			VERBOSE_ASSOCIATE("Attempting ovewriting " 
				<< context << " with " << ccCyan << result);

			try {
				atLeastOneSuccess = result.Copy(context) > 0;
			}
			catch (const Except::BadCopy&) {
				// Catenate results into one element, and then copy			
				auto cat = Verb::From<Verbs::Catenate>({}, result);
				auto catenated = Any::From(context, DState::Typed);
				catenated.Allocate(1, true);
				if (Verb::DispatchFlat(catenated, cat)) {
					// Success																
					//VERBOSE_ASSOCIATE("Ovewriting " << context
					//	<< " with catenated " << ccCyan << cat.GetOutput());
					atLeastOneSuccess = cat.GetOutput().Copy(context) > 0;
				}
				else {
					// Failure																
					VERBOSE_ASSOCIATE(ccRed << "Can't overwrite " << context
						<< " with badly catenated " << result);
				}
			}
		}

		// An association verb always pushes context as output				
		if (atLeastOneSuccess)
			verb << Any {context};
	}

} // namespace Langulus::Flow


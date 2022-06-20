#include "../GASM.hpp"
#include "IncludeLogger.hpp"

#define VERBOSE_CREATION(a) //pcLogVerbose << a

namespace PCFW::Flow
{

	/// A helper delegation function, that calls Create verb in all elements	
	///	@param context - [in/out] the context to gelegate to						
	///	@param argument - the data to use as argument								
	///	@param sideproducts - [out] any resulting sideproducts					
	void Delegate(Any& context, const Any& argument, Any& sideproducts) {
		VERBOSE_CREATION(ccYellow << "Delegating: " << argument << " to " << context);
		auto creator = Verb::From<Verbs::Create>({}, argument);
		if (!Verb::ExecuteVerb(context, creator)) {
			throw Except::BadConstruction(VERBOSE_CREATION(ccRed
				<< "Couldn't delegate " << argument << " inside: " << context
			));
		}

		auto& result = creator.GetOutput();
		if (!result.IsEmpty() && result != context) {
			// Push sideproduct only if the creator returns something new	
			VERBOSE_CREATION(ccYellow << "Sideproduct: " << result);
			sideproducts << pcMove(result);
		}
	}

	/// Set members in all elements inside context to the provided data			
	///	@param context - the contexts to analyze										
	///	@param data - the data to set to													
	///	@return true if at least one member in one element was set				
	void Verb::SetMembers(Any& context, const Any& data) {
		TMap<TMeta, Index> satisfiedTraits;
		TMap<DMeta, Index> satisfiedData;

		// If reached, then delegation failed										
		data.ForEachDeep([&](const Block& group) {
			VERBOSE_CREATION("Manually initializing " << context
				<< " with " << ccCyan << group);

			// Search for similar data in the current context					
			// in an attempt to overwrite member variables and such			
			for (pcptr i = 0; i < group.GetCount(); ++i) {
				Any element = group.GetElementResolved(i);
				if (element.Is<Trait>()) {
					// Search for the trait												
					const auto meta = element.Get<Trait>().GetTraitMeta();
					const auto sati = satisfiedTraits.FindKey(meta);
					const auto index = sati != uiNone
						? satisfiedTraits.GetValue(sati)
						: uiFirst;
					VERBOSE_CREATION("Searching trait " << meta
						<< "... " << " (" << index << ")");

					auto selector = Verb::From<Verbs::Select>({}, Any::Wrap(meta, index));
					Verb::DefaultSelect(context, selector);
					if (!selector.GetOutput().IsEmpty()) {
						VERBOSE_CREATION("Initializing trait " << selector.GetOutput()
							<< " with " << ccCyan << element << " (" << index << ")");

						auto associator = Verb::From<Verbs::Associate>({}, element);
						if (Verb::ExecuteVerb(selector.GetOutput(), associator)) {
							// Trait was found and overwritten						
							if (sati != uiNone)
								++satisfiedTraits.GetValue(sati);
							else
								satisfiedTraits.Add(meta, 1);

							VERBOSE_CREATION(ccYellow << "Initialized " 
								<< selector.GetOutput() << " (" << index << ")");
							continue;
						}
					}
				}

				// Search for the data block											
				// This is only reached if the trait attempt fails 			
				// Failing this is considered critical - context should be	
				// later discarded - it's cosidered ill-formed					
				auto meta = element.Is<Trait>()
					? element.Get<Trait>().GetMeta()
					: element.GetMeta();

				if (meta->InterpretsAs<ANumber>(1)) {
					// If number, keep it abstract									
					//TODO this seems sketchy and i don't like it!
					meta = MetaData::Of<ANumber>();
				}

				const auto sati = satisfiedData.FindKey(meta);
				const auto index = sati != uiNone
					? satisfiedData.GetValue(sati)
					: uiFirst;
				VERBOSE_CREATION("Searching for data " << meta
					<< "... " << " (" << index << ")");

				auto selector = Verb::From<Verbs::Select>({}, Any::Wrap(meta, index));
				Verb::DefaultSelect(context, selector);
				if (!selector.GetOutput().IsEmpty()) {
					VERBOSE_CREATION("Initializing data " << selector.GetOutput()
						<< " with " << ccCyan << element << " (" << index << ")");

					auto associator = Verb::From<Verbs::Associate>({}, pcMove(element));
					if (Verb::ExecuteVerb(selector.GetOutput(), associator)) {
						// Data was found and was overwritten						
						if (sati != uiNone)
							++satisfiedData.GetValue(sati);
						else
							satisfiedData.Add(meta, 1);

						VERBOSE_CREATION(ccYellow << "Initialized " 
							<< selector.GetOutput() << " (" << index << ")");
					}
					else {
						// Can't set the member											
						throw Except::BadConstruction(pcLogError
							<< "Couldn't initialize member ");
					}
				}
				else {
					// Failure occurs for the given argument						
					// It may be because of excess arguments, so check if		
					// the constructed type is fully satisfied at this point	
					if (sati == uiNone || satisfiedData.GetValue(sati) != context.GetMeta()->GetMemberCount(meta)) {
						// The context wasn't satisfied								
						throw Except::BadConstruction(pcLogError
							<< "No member of type " << meta->GetToken());
					}
				}
			}
		});
	}

	/// Satisfy members of a given element													
	///	@param context - the context to initialize									
	///	@param data - the data to satisfy with											
	///	@param sideproducts - [out] sideproducts on nested constructs			
	void Verb::DefaultCreateInner(Any& context, const Any& data, Any& sideproducts) {
		if (data.IsEmpty())
			return;

		// First attempt directly delegating										
		try {
			Delegate(context, data, sideproducts);
			return;
		}
		catch (const Except::BadConstruction&) { }

		Verb::SetMembers(context, data);
	}

	/// Default creation																			
	///	@param context - the block to execute in										
	///	@param verb - the stuff to create												
	void Verb::DefaultCreate(Block& context, Verb& verb) {
		auto& arguments = verb.GetArgument();
		if (context.IsConstant() || arguments.IsEmpty())
			return;

		if (!context.IsEmpty()) {
			if (context.IsStatic()) {
				// The context can't be resized, so just return					
				return;
			}

			TODO();
		}

		// First attempt creating constructs in the verb						
		arguments.ForEachDeep([&](const Construct& construct) {
			if (construct.GetCharge().mMass * verb.GetMass() <= 0)
				TODO();

			SAFETY(if (construct.GetMeta()->GetProducerMeta())
				throw Except::BadConstruction(pcLogError
					<< "Creation of customly produced type "
					<< construct.GetMeta()->GetToken()
					<< " hit default creation, and that should not be allowed - "
					<< "add the Create verb in the producer interface"
				)
			);

			// First allocate all the required defaultly constructed			
			auto created = Any::From(construct.GetMeta());
			created.Allocate(pcptr(construct.GetCharge().mMass), true);

			// Then forward the constructors to each element					
			Any sideproducts;
			if (!construct.IsEmpty()) {
				for (pcptr i = 0; i < created.GetCount(); ++i) {
					Any element {created.GetElement(i)};
					DefaultCreateInner(element, construct.GetAll(), sideproducts);
				}
			}

			// Commit																		
			verb << pcMove(created);
			if (!sideproducts.IsEmpty())
				verb << pcMove(sideproducts);
		});
	}

} // namespace PCFW::PCGASM

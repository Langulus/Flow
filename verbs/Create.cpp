#include "../Code.hpp"

#define VERBOSE_CREATION(a) //Logger::Verbose() << a
#define VERBOSE_SCOPES(a) Logger::Verbose() << a

namespace Langulus::Flow
{

	/// A helper delegation function, that calls Create verb in all elements	
	///	@param context - [in/out] the context to gelegate to						
	///	@param argument - the data to use as argument								
	///	@param sideproducts - [out] any resulting sideproducts					
	void Delegate(Any& context, const Any& argument, Any& sideproducts) {
		VERBOSE_CREATION(Logger::Yellow
			<< "Delegating: " << argument << " to " << context);

		Verbs::Create creator({}, argument);
		if (!Verb::ExecuteVerb(context, creator)) {
			Throw<Except::Construct>(VERBOSE_CREATION(Logger::Yellow
				<< "Couldn't delegate " << argument << " inside: " << context
			));
		}

		auto& result = creator.GetOutput();
		if (!result.IsEmpty() && result != context) {
			// Push sideproduct only if the creator returns something new	
			VERBOSE_CREATION(Logger::Yellow << "Sideproduct: " << result);
			sideproducts << Move(result);
		}
	}

	/// Set members in all elements inside context to the provided data			
	///	@param context - the contexts to analyze										
	///	@param data - the data to set to													
	///	@return true if at least one member in one element was set				
	void Verb::SetMembers(Any& context, const Any& data) {
		THashMap<TMeta, Index> satisfiedTraits;
		THashMap<DMeta, Index> satisfiedData;

		// If reached, then delegation failed										
		data.ForEachDeep([&](const Block& group) {
			VERBOSE_CREATION("Manually initializing " << context
				<< " with " << Logger::Cyan << group);

			// Search for similar data in the current context					
			// in an attempt to overwrite member variables and such			
			for (Count i = 0; i < group.GetCount(); ++i) {
				Any element = group.GetElementResolved(i);
				if (element.Is<Trait>()) {
					// Search for the trait												
					const auto meta = element.Get<Trait>().GetTrait();
					const auto sati = satisfiedTraits.FindKeyIndex(meta);
					const auto index = sati
						? satisfiedTraits.GetValue(sati)
						: Index::First;
					VERBOSE_CREATION("Searching trait " << meta
						<< "... " << " (" << index << ")");

					auto selector = Verbs::Select({}, Any::Wrap(meta, index));
					Verb::DefaultSelect(context, selector);
					if (!selector.GetOutput().IsEmpty()) {
						VERBOSE_CREATION("Initializing trait " << selector.GetOutput()
							<< " with " << Logger::Cyan << element << " (" << index << ")");

						auto associator = Verbs::Associate({}, element);
						if (Verb::ExecuteVerb(selector.GetOutput(), associator)) {
							// Trait was found and overwritten						
							if (sati)
								++satisfiedTraits.GetValue(sati);
							else
								satisfiedTraits.Insert({meta, 1});

							VERBOSE_CREATION(Logger::Yellow << "Initialized "
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
					? element.Get<Trait>().GetType()
					: element.GetType();

				if (meta->CastsTo<A::Number>(1)) {
					// If number, keep it abstract									
					//TODO this seems sketchy and i don't like it!
					meta = MetaData::Of<A::Number>();
				}

				const auto sati = satisfiedData.FindKeyIndex(meta);
				const auto index = sati
					? satisfiedData.GetValue(sati)
					: Index::First;
				VERBOSE_CREATION("Searching for data " << meta
					<< "... " << " (" << index << ")");

				Verbs::Select selector({}, Any::Wrap(meta, index));
				Verb::DefaultSelect(context, selector);
				if (!selector.GetOutput().IsEmpty()) {
					VERBOSE_CREATION("Initializing data " << selector.GetOutput()
						<< " with " << Logger::Cyan << element << " (" << index << ")");

					Verbs::Associate associator({}, Move(element));
					if (Verb::ExecuteVerb(selector.GetOutput(), associator)) {
						// Data was found and was overwritten						
						if (sati)
							++satisfiedData.GetValue(sati);
						else
							satisfiedData.Insert({meta, 1});

						VERBOSE_CREATION(Logger::Yellow << "Initialized "
							<< selector.GetOutput() << " (" << index << ")");
					}
					else {
						// Can't set the member											
						Throw<Except::Construct>(Logger::Error()
							<< "Couldn't initialize member ");
					}
				}
				else {
					// Failure occurs for the given argument						
					// It may be because of excess arguments, so check if		
					// the constructed type is fully satisfied at this point	
					if (!sati || satisfiedData.GetValue(sati) != context.GetType()->GetMemberCount(meta)) {
						// The context wasn't satisfied								
						Throw<Except::Construct>(Logger::Error()
							<< "No member of type " << meta->mToken);
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
		catch (const Except::Construct&) { }

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

			SAFETY(if (construct.GetProducer())
				Throw<Except::Construct>(Logger::Error()
					<< "Creation of customly produced type " << construct.GetToken()
					<< " hit default creation, and that should not be allowed - "
					<< "add the Create verb in the producer interface"
				)
			);

			// First allocate all the required defaultly constructed			
			auto created = Any::FromMeta(construct.GetType());
			created.Allocate<true>(Count(construct.GetCharge().mMass));

			// Then forward the constructors to each element					
			Any sideproducts;
			if (!construct.IsEmpty()) {
				for (Count i = 0; i < created.GetCount(); ++i) {
					Any element {created.GetElement(i)};
					DefaultCreateInner(element, construct.GetAll(), sideproducts);
				}
			}

			// Commit																		
			verb << Move(created);
			if (!sideproducts.IsEmpty())
				verb << Move(sideproducts);
		});
	}

	/// Helper that is analogous to Block::Gather, but also gives the option	
	/// to do a runtime interpretation to elements, that are not compatible		
	/// This is very useful for extracting relevant data from Idea(s)				
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@return the number of gathered elements										
	Count GatherAndInterpret(const Block& input, Block& output, const Index direction) {
		if (output.IsUntyped() || output.IsDeep()) {
			// Output is deep/any, so no need to iterate or interpret		
			return output.InsertBlock(input);
		}

		Count count {};
		if (input.IsDeep()) {
			// Iterate all subpacks														
			if (direction == Index::Front) {
				for (Count i = 0; i < input.GetCount(); ++i)
					count += GatherAndInterpret(input.As<Block>(i), output, direction);
			}
			else {
				for (Count i = input.GetCount(); i > 0; --i)
					count += GatherAndInterpret(input.As<Block>(i - 1), output, direction);
			}

			return count;
		}

		if (output.IsConcatable(input)) {
			// Catenate input if compatible											
			count += output.InsertBlock(input);
		}
		else {
			// Attempt a slow interpretation to the output type				
			Verbs::Interpret interpreter({}, output.GetType());
			Verb::DispatchDeep(const_cast<Block&>(input), interpreter);
			try {
				count += output.InsertBlock(interpreter.GetOutput());
			}
			catch (const Except::Mutate&) {}
		}

		return count;
	}

	/// Produces constructs, strings, etc.													
	///	@param context - the context to execute in									
	///	@param verb - the scope's stuffing												
	void Verb::DefaultScope(const Block& context, Verb& verb) {
		// Scan source for what we're constructing								
		context.ForEach([&verb](DMeta meta) {
			Construct scope(meta);

			// Scan for charges first													
			TAny<Real> mass;
			GatherAndInterpret(verb.GetArgument(), mass, Index::Front);
			if (!mass.IsEmpty()) {
				// Mass is collected, but may be fragmented, so use the		
				// concat verb to coalesce the numbers inside					
				VERBOSE_SCOPES(Logger::Green << "Mass " << mass << " for " << scope);
				Real concatenatedMass = 0;
				for (auto& n : mass)
					concatenatedMass = pcConcat(concatenatedMass, n);
				scope.mMass = concatenatedMass;
			}

			// Scan for data types														
			TAny<DMeta> metaDatas;
			GatherAndInterpret(verb.GetArgument(), metaDatas, Index::Front);
			if (!metaDatas.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green << "DMeta(s) for "
					<< meta->mToken << ": " << metaDatas);
				scope << metaDatas;
			}

			// Scan for constructs														
			TAny<Construct> constructs;
			GatherAndInterpret(verb.GetArgument(), constructs, Index::Front);
			if (!constructs.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green << "Construct(s) for "
					<< meta->mToken << ": " << constructs);
				scope << constructs;
			}

			// Scan for traits															
			TAny<Trait> traits;
			GatherAndInterpret(verb.GetArgument(), traits, Index::Front);
			if (!constructs.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green << "Trait(s) for "
					<< meta->mToken << ": " << traits);
				scope << traits;
			}

			VERBOSE_SCOPES(Logger::Green << "Resulting scope: " << scope);
			verb << scope;
		});
	}

} // namespace Langulus::Flow

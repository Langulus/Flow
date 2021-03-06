#pragma once
#include "../Code.hpp"

#define VERBOSE_CREATION(a) //Logger::Verbose() << a
#define VERBOSE_SCOPES(a) Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Create/destroy verb construction													
	///	@param s - the producer of the stuff											
	///	@param a - the stuff to produce													
	///	@param o - result mask (optional)												
	///	@param c - the charge of the creation											
	///	@param sc - is the creation short-circuited									
	inline Create::Create(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Create>(), s, a, o, c, sc} {}

	/// Check if the verb is available in a type, and with given arguments		
	///	@return true if verb is available in T with arguments A...				
	template<CT::Data T, CT::Data... A>
	constexpr bool Create::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Create(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Create(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Create::Of() noexcept {
		if constexpr (!Create::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Create(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Create(verb, args...);
			};
		}
	}

	/// Execute creation verb in a specific context										
	///	@param context - the producer														
	///	@param verb - the creation/destruction verb									
	///	@return true if verb was satisfied												
	template<CT::Data T>
	bool Create::ExecuteIn(T& context, Verb& verb) {
		static_assert(Create::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Create(verb);
		return verb.IsDone();
	}

	/// Default creation/destruction in a context										
	///	@param context - the producer														
	///	@param verb - the creation/destruction verb									
	///	@return true if verb was satisfied												
	inline bool Create::ExecuteDefault(Anyness::Block& context, Verb& verb) {
		// Attempt creating/destroying constructs									
		verb.GetArgument().ForEachDeep([&](const Construct& construct) {
			SAFETY(if (construct.GetProducer())
				Throw<Except::Construct>(Logger::Error()
					<< "Creation of customly produced type " << construct.GetToken()
					<< " hit default creation, and that should not be allowed - "
					<< "add and reflect the Create verb in the producer"
				)
			);

			if (construct.GetCharge().mMass * verb.GetMass() < 0) {
				//TODO destroy
			}
			else {
				// Create																	
				// First allocate and default-initialize the results			
				auto created = Any::FromMeta(construct.GetType());
				created.Allocate<true>(Count(construct.GetCharge().mMass));
				auto& arguments = construct.GetAll();

				// Then forward the constructors to each element				
				if (!arguments.IsEmpty()) {
					for (Count i = 0; i < created.GetCount(); ++i) {
						Any element {created.GetElement(i)};

						// First attempt delegating									
						VERBOSE_CREATION(Logger::Yellow<<
							"Delegating: " << arguments << " to " << element);

						Verbs::Create creator({}, arguments);
						if (Scope::ExecuteVerb(element, creator)) {
							VERBOSE_CREATION(Logger::Yellow << "Sideproduct: " << creator.GetOutput());
							created.MergeBlock(Abandon(creator.GetOutput()));
							continue;
						}

						VERBOSE_CREATION(Logger::Yellow <<
							"Couldn't delegate " << arguments << " inside: " << element);

						// If reached, let's attempt to set reflected members	
						SetMembers(element, arguments);
					}
				}

				// Commit																	
				verb << Abandon(created);
			}
		});

		return verb.IsDone();
	}

	/// Stateless creation of any type without a producer								
	///	@param verb - the creation verb													
	///	@return true if verb was satisfied												
	inline bool Create::ExecuteStateless(Verb& verb) {
		if (verb.GetArgument().IsEmpty() || verb.GetMass() < 0)
			return false;

		//TODO create only
		return true;
	}

	/// Set members in all elements inside context to the provided data			
	///	@param context - the contexts to analyze										
	///	@param data - the data to set to													
	///	@return true if at least one member in one element was set				
	inline void Create::SetMembers(Any& context, const Any& data) {
		THashMap<TMeta, Count> satisfiedTraits;
		THashMap<DMeta, Count> satisfiedData;

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

					Verbs::Select selector({}, Any::Wrap(meta, index));
					Verb::ExecuteIn(context, selector);
					if (!selector.GetOutput().IsEmpty()) {
						VERBOSE_CREATION("Initializing trait " << selector.GetOutput()
							<< " with " << Logger::Cyan << element << " (" << index << ")");

						Verbs::Associate associator({}, element);
						if (Verb::ExecuteIn(selector.GetOutput(), associator)) {
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
				Verb::ExecuteIn(context, selector);
				if (!selector.GetOutput().IsEmpty()) {
					VERBOSE_CREATION("Initializing data " << selector.GetOutput()
						<< " with " << Logger::Cyan << element << " (" << index << ")");

					Verbs::Associate associator({}, Move(element));
					if (Verb::ExecuteIn(selector.GetOutput(), associator)) {
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
						Throw<Except::Construct>("Couldn't initialize member ");
					}
				}
				else {
					// Failure occurs for the given argument						
					// It may be because of excess arguments, so check if		
					// the constructed type is fully satisfied at this point	
					if (!sati || satisfiedData.GetValue(sati) != context.GetType()->GetMemberCount(nullptr, meta)) {
						// The context wasn't satisfied								
						Throw<Except::Construct>("Excess, or insufficient arguments");
					}
				}
			}
		});
	}

} // namespace Langulus::Verbs


namespace Langulus::Flow
{



	/// Helper that is analogous to Block::Gather, but also gives the option	
	/// to do a runtime interpretation to elements, that are not compatible		
	/// This is very useful for extracting relevant data from Idea(s)				
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@return the number of gathered elements										
	/*Count GatherAndInterpret(const Block& input, Block& output, const Index direction) {
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
			DispatchDeep<true, true, true>(const_cast<Block&>(input), interpreter);
			try {
				count += output.InsertBlock(interpreter.GetOutput());
			}
			catch (const Except::Mutate&) {}
		}

		return count;
	}*/

	/// Produces constructs, strings, etc.													
	///	@param context - the context to execute in									
	///	@param verb - the scope's stuffing												
	/*void Verb::DefaultScope(const Block& context, Verb& verb) {
		// Scan source for what we're constructing								
		context.ForEach([&verb](DMeta meta) {
			Construct scope(meta);

			// Scan for charges first													
			TAny<Real> mass;
			GatherAndInterpret(verb.GetArgument(), mass, Index::Front);
			if (!mass.IsEmpty()) {
				// Mass is collected, but may be fragmented, so use the		
				// concat verb to coalesce the numbers inside					
				VERBOSE_SCOPES(Logger::Green 
					<< "Mass " << mass << " for " << scope);
				Real concatenatedMass = 0;
				for (auto& n : mass)
					concatenatedMass = pcConcat(concatenatedMass, n);
				scope.mMass = concatenatedMass;
			}

			// Scan for data types														
			TAny<DMeta> metaDatas;
			GatherAndInterpret(verb.GetArgument(), metaDatas, Index::Front);
			if (!metaDatas.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green 
					<< "DMeta(s) for " << meta->mToken << ": " << metaDatas);
				scope << metaDatas;
			}

			// Scan for constructs														
			TAny<Construct> constructs;
			GatherAndInterpret(verb.GetArgument(), constructs, Index::Front);
			if (!constructs.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green
					<< "Construct(s) for " << meta->mToken << ": " << constructs);
				scope << constructs;
			}

			// Scan for traits															
			TAny<Trait> traits;
			GatherAndInterpret(verb.GetArgument(), traits, Index::Front);
			if (!constructs.IsEmpty()) {
				VERBOSE_SCOPES(Logger::Green
					<< "Trait(s) for " << meta->mToken << ": " << traits);
				scope << traits;
			}

			VERBOSE_SCOPES(Logger::Green << "Resulting scope: " << scope);
			verb << scope;
		});
	}*/

} // namespace Langulus::Flow

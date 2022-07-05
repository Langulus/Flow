#include "Scope.hpp"
#include "Construct.hpp"
#include "verbs/Do.inl"

#define VERBOSE_FLOW(a)			// Logger::Verbose() << a
#define VERBOSE_FLOW_TAB(a)	// ScopedTab tab; Logger::Verbose() << a << tab
#define FLOW_ERRORS(a)			// Logger::Error() << a

namespace Langulus::Flow
{

	/// Flat check if scope contains executable verbs									
	///	@return true if the flat scope contains verbs								
	bool Scope::IsExecutable() const noexcept {
		if (Is<Verb>())
			return true;

		bool result{};
		EitherDoThis
			// Scan deeper into traits, because they're not marked deep		
			// They are deep only in respect to execution						
			ForEach([&result](const Trait& trait) noexcept {
				const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
				result = scope.IsExecutable();
				return !result;
			})
		OrThis
			// Scan deeper into constructs, because they're not marked deep
			// They are deep only in respect to execution						
			ForEach([&result](const Construct& construct) noexcept {
				const auto& scope = ReinterpretCast<Scope>(construct.GetAll());
				result = scope.IsExecutable();
				return !result;
			});

		return result;
	}

	/// Deep (nested and slower) check if scope contains executable verbs		
	///	@return true if the deep or flat scope contains verbs						
	bool Scope::IsExecutableDeep() const noexcept {
		if (IsExecutable())
			return true;

		bool result{};
		ForEachDeep([&result](const Block& group) noexcept {
			const auto& scope = ReinterpretCast<Scope>(group);
			result = scope.IsExecutable();
			return !result;
		});

		return result;
	}

	/// Nested AND/OR scope execution (discarding outputs)							
	/// TODO optimize for unneeded outputs													
	///	@param environment - the environment in which scope will be executed	
	///	@return true of no errors occured												
	bool Scope::Execute(Any& environment) const {
		Any output;
		bool skipVerbs = false;
		return Execute(environment, output, skipVerbs);
	}

	/// Nested AND/OR scope execution with output										
	///	@param environment - the environment in which scope will be executed	
	///	@param output - [out] verb result will be pushed here						
	///	@return true of no errors occured												
	bool Scope::Execute(Any& environment, Any& output) const {
		bool skipVerbs = false;
		return Execute(environment, output, skipVerbs);
	}

	/// Nested AND/OR scope execution with output										
	///	@param environment - the environment in which scope will be executed	
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [in/out] whether to skip verbs after OR success	
	///	@return true of no errors occured												
	bool Scope::Execute(Any& environment, Any& output, bool& skipVerbs) const {
		bool executed = true;
		auto results = Any::FromState(*this);
		if (!IsEmpty()) {
			VERBOSE_FLOW_TAB("Executing scope: " << this);

			if (IsOr() && GetCount() > 1)
				executed = ExecuteOR(environment, results, skipVerbs);
			else
				executed = ExecuteAND(environment, results, skipVerbs);
		}

		if (executed && !results.IsEmpty()) {
			results.Optimize();
			output = Abandon(results);
		}

		return executed;
	}

	/// Nested AND scope execution															
	///	@param environment - the environment in which scope will be executed	
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [in/out] whether to skip verbs after OR success	
	///	@return true of no errors occured												
	bool Scope::ExecuteAND(Any& environment, Any& output, bool& skipVerbs) const {
		if (IsDeep()) {
			// Nest if deep																
			const auto& asScopes = ReinterpretCast<TAny<Scope>>(*this);
			for (const auto& scope : asScopes) {
				Any local;
				if (!scope.Execute(environment, local, skipVerbs)) {
					VERBOSE_FLOW(Logger::Red << "Deep-AND failed: " << this);
					return false;
				}

				output << Abandon(local);
			}
		}
		else if (Is<Trait>()) {
			// Nest if traits, but retain each trait 								
			const auto& asTraits = ReinterpretCast<TAny<Trait>>(*this);
			for (const auto& trait : asTraits) {
				const auto& traitAsScope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
				Any local;
				if (!traitAsScope.Execute(environment, local, skipVerbs)) {
					VERBOSE_FLOW(Logger::Red << "Trait-AND failed: " << this);
					return false;
				}

				output << Trait {trait.GetTrait(), Abandon(local)};
			}
		}
		else if (Is<Construct>()) {
			// Nest if constructs, but retain each construct					
			const auto& asConstructs = ReinterpretCast<TAny<Construct>>(*this);
			for (const auto& construct : asConstructs) {
				const auto& constructAsScope = ReinterpretCast<Scope>(construct.GetAll());
				Any local;
				if (!constructAsScope.Execute(environment, local, skipVerbs)) {
					VERBOSE_FLOW(Logger::Red << "Construct-AND failed: " << scope);
					return false;
				}

				Construct newc {construct.GetType(), Move(local), construct};
				try {
					// Attempt constructing the construct here if possible	
					newc.StaticCreation(local);
					output << Abandon(local);
				}
				catch (const Except::Construct&) {
					// Construction failed, so just propagate construct		
					// A new attempt will be made at runtime						
					output << Abandon(newc);
				}
			}
		}
		else if (Is<Verb>()) {
			if (skipVerbs)
				return false;

			const auto& asVerbs = ReinterpretCast<Script>(*this);
			for (const auto& constVerb : asVerbs) {
				// Shallow-copy the verb to make it mutable						
				// Also resets the output												
				Verb verb {
					constVerb.mVerb,
					constVerb.mSource,
					constVerb.mArgument, {},
					constVerb,
					constVerb.mShortCircuited
				};

				// Execute the verb														
				if (!Scope::ExecuteVerb(environment, verb)) {
					VERBOSE_FLOW(Logger::Red << "Verb-AND failed: " << this);
					return false;
				}

				if (!verb.mOutput.IsEmpty())
					output << Abandon(verb.mOutput);
			}
		}
		else {
			// If this is reached, then we have non-verb content				
			// Just propagate content													
			output << *this;
		}

		VERBOSE_FLOW(Logger::Green << "And-Scope done: " << this);
		return true;
	}

	/// Nested OR execution																		
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [out] whether to skip verbs after OR success		
	///	@return true of no errors occured												
	bool Scope::ExecuteOR(Any& environment, Any& output, bool& skipVerbs) const {
		bool executed = false;
		bool localSkipVerbs = skipVerbs; //TODO was false in old code, but should it be? doesn't make sense...

		if (IsDeep()) {
			// DEEP OR																		
			// Execute in order until a successful execution occurs,			
			// then skip verbs - only collect data									
			const auto& asScopes = ReinterpretCast<TAny<Scope>>(*this);
			for (const auto& scope : asScopes) {
				//Any localContext {environment};
				Any local;
				if (scope.Execute(environment, local, localSkipVerbs)) {
					executed = true;
					if (!local.IsEmpty())
						output << Abandon(local);
				}
			}

			skipVerbs = localSkipVerbs;
		}
		else if (Is<Trait>()) {
			// All traits get executed, but the failed scope traits get		
			// discarded																	
			for (Count i = 0; i < scope.GetCount(); ++i) {
				bool unusedSkipVerbs = false;
				Any localOutput;
				auto& trait = scope.Get<Trait>(i);
				if (Verb::ExecuteScope(environment, trait, localOutput, unusedSkipVerbs)) {
					executed = true;
					output << Trait {trait.GetTrait(), Move(localOutput)};
				}
			}
		}
		else if (Is<Construct>()) {
			// All constructs get executed, but the failed ones get			
			// discarded																	
			for (Count i = 0; i < scope.GetCount(); ++i) {
				bool unusedSkipVerbs = false;
				Any localOutput;
				auto& construct = scope.Get<Construct>(i);
				if (Verb::ExecuteScope(environment, construct.GetAll(), localOutput, unusedSkipVerbs)) {
					executed = true;
					output << Construct {construct.GetType(), Move(localOutput), construct};
				}
			}
		}
		else if (Is<Verb>()) {
			// SHALLOW OR																	
			// We couldn't pick a verb, so execute in order until	a			
			// successful execution occurs. On success simply skip other	
			// verbs - only collect data along the way (using skipVerbs)	
			if (localSkipVerbs) {
				VERBOSE_FLOW(Logger::DarkYellow << "OR-Scope skipped: " << scope);
				return false;
			}

			auto& asVerbs = ReinterpretCast<Script>(*this);
			for (auto& constVerb : asVerbs) {
				Verb verb { 
					constVerb.mVerb, 
					constVerb.mSource, 
					constVerb.mArgument, {},
					constVerb,
					constVerb.mShortCircuited
				};

				if (!Scope::ExecuteVerb(environment, verb))
					continue;

				executed = true;
				if (!verb.mOutput.IsEmpty())
					output << Move(verb.mOutput);
			}

			skipVerbs = localSkipVerbs;
		}
		else {
			// If this is reached, then we have non-verb flat content		
			// Just propagate it															
			output << *this;
			executed = true;
		}

		if (executed)
			VERBOSE_FLOW(Logger::Green << "OR-Scope done: " << this);
		else
			VERBOSE_FLOW(Logger::Red << "OR-Scope failed: " << this);
		return executed;
	}

	/// Integrate all parts of a verb inside this environment						
	///	@param context - [in/out] the context where verb will be integrated	
	///	@param verb - [in/out] verb to integrate										
	///	@return true of no errors occured												
	bool Scope::IntegrateVerb(Any& environment, Verb& verb) {
		// Integrate the verb source to the current context					
		Any localSource;
		if (!ReinterpretCast<Scope>(verb.mSource).Execute(environment, localSource)) {
			Logger::Error() << "Error at source: " << verb.mSource;
			return false;
		}

		if (localSource.IsEmpty())
			localSource = environment;

		// Integrate the verb argument to the source								
		Any localArgument;
		if (!ReinterpretCast<Scope>(verb.mArgument).Execute(localSource, localArgument)) {
			Logger::Error() << "Error at argument: " << verb.mArgument;
			return false;
		}

		verb.mSource = Abandon(localSource);
		verb.mArgument = Abandon(localArgument);
		return true;
	}

	/// Execute a single verb, and all subverbs in it, if any						
	///	@param context - [in/out] the context in which verb will be executed	
	///	@param verb - [in/out] verb to execute											
	///	@return true of no errors occured												
	bool Scope::ExecuteVerb(Any& context, Verb& verb) {
		// Integration (and execution of subverbs if any)						
		// Source and argument will be executed locally if scripts, and	
		// substituted with their results in the verb							
		if (!Scope::IntegrateVerb(context, verb)) {
			FLOW_ERRORS("Error integrating verb: " << verb);
			return false;
		}

		if (verb.Is<Verbs::Do>()) {
			// A Do verb is done at this point, because the subverbs 		
			// inside (if any) should be done in the integration phase		
			// Just making sure that the integrated argument & source are	
			// propagated to the verb's output										
			if (verb.mOutput.IsEmpty()) {
				if (!verb.mArgument.IsEmpty())
					verb << Move(verb.mArgument); //TODO wasn't a move, can it be?
				else
					verb << Move(verb.mSource); //TODO wasn't a move, can it be?
			}

			return true;
		}

		VERBOSE_FLOW_TAB("Executing " << Logger::Cyan << verb);

		// Dispatch the verb to the context, executing it						
		// Any results should be inside verb.mOutput afterwards				
		if (!DispatchDeep<true, true, true>(verb.mSource, verb)) {
			FLOW_ERRORS("Error executing verb: " << verb);
			return false;
		}

		VERBOSE_FLOW("Executed: " << Logger::Green << verb);
		return true;
	}

} // namespace Langulus::Flow

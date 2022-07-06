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
		ForEach(
			// Scan deeper into traits, because they're not marked deep		
			// They are deep only in respect to execution						
			[&result](const Trait& trait) noexcept {
				const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
				result = scope.IsExecutable();
				return !result;
			},
			// Scan deeper into constructs, because they're not marked deep
			// They are deep only in respect to execution						
			[&result](const Construct& construct) noexcept {
				const auto& scope = ReinterpretCast<Scope>(construct.GetAll());
				result = scope.IsExecutable();
				return !result;
			}
		);

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
		const bool executed = ForEach(
			// Nest if deep																
			[&](const Block& block) {
				const auto& scope = ReinterpretCast<Scope>(block);
				Any local;
				if (!scope.Execute(environment, local, skipVerbs)) {
					Throw<Except::Flow>(VERBOSE_FLOW(Logger::Red
						<< "Deep-AND failed: " << this));
				}

				output << Abandon(local);
			},
			// Nest if traits, but retain each trait 								
			[&](const Trait& trait) {
				const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
				Any local;
				if (!scope.Execute(environment, local, skipVerbs)) {
					Throw<Except::Flow>(VERBOSE_FLOW(Logger::Red
						<< "Trait-AND failed: " << this));
				}

				output << Trait {trait.GetTrait(), Abandon(local)};
			},
			// Nest if constructs, but retain each construct					
			[&](const Construct& construct) {
				const auto& scope = ReinterpretCast<Scope>(construct.GetAll());
				Any local;
				if (!scope.Execute(environment, local, skipVerbs)) {
					Throw<Except::Flow>(VERBOSE_FLOW(Logger::Red
						<< "Construct-AND failed: " << this));
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
			},
			// Execute verbs																
			[&](const Verb& constVerb) {
				if (skipVerbs)
					return false;

				// Shallow-copy the verb to make it mutable						
				// Also resets its output												
				Verb verb {
					constVerb.mVerb,
					constVerb.mSource,
					constVerb.mArgument, {},
					constVerb,
					constVerb.mShortCircuited
				};

				// Execute the verb														
				if (!Scope::ExecuteVerb(environment, verb)) {
					Throw<Except::Flow>(VERBOSE_FLOW(Logger::Red
						<< "Verb-AND failed: " << this));
				}

				if (!verb.mOutput.IsEmpty())
					output << Abandon(verb.mOutput);
				return true;
			}
		);

		if (!executed) {
			// If this is reached, then we had non-verb content				
			// Just propagate its contents											
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
		bool executed {};
		bool localSkipVerbs {};

		ForEach(
			// Nest if deep																
			[&](const Block& block) {
				const auto& scope = ReinterpretCast<Scope>(block);
				Any local;
				if (scope.Execute(environment, local, localSkipVerbs)) {
					executed = true;
					if (!local.IsEmpty())
						output << Abandon(local);
				}
			},
			// Nest if traits, but retain each trait 								
			[&](const Trait& trait) {
				const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
				Any local;
				if (scope.Execute(environment, local)) {
					executed = true;
					output << Trait {trait.GetTrait(), Abandon(local)};
				}
			},
			// Nest if constructs, but retain each construct					
			[&](const Construct& construct) {
				const auto& scope = ReinterpretCast<Scope>(construct.GetAll());
				Any local;
				if (scope.Execute(environment, local)) {
					executed = true;
					output << Construct {construct.GetType(), Abandon(local), construct};
				}
			},
			// Execute verbs																
			[&](const Verb& constVerb) {
				if (localSkipVerbs)
					return false;

				// Shallow-copy the verb to make it mutable						
				// Also resets its output												
				Verb verb {
					constVerb.mVerb,
					constVerb.mSource,
					constVerb.mArgument, {},
					constVerb,
					constVerb.mShortCircuited
				};

				if (!Scope::ExecuteVerb(environment, verb))
					return true;

				executed = true;

				if (!verb.mOutput.IsEmpty())
					output << Abandon(verb.mOutput);
				return true;
			}
		);

		skipVerbs |= localSkipVerbs;

		if (!executed) {
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

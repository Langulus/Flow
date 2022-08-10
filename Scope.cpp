#include "Scope.hpp"
#include "Construct.hpp"
#include "verbs/Do.inl"
#include "verbs/Interpret.inl"

#define VERBOSE(a)		Logger::Verbose() << a
#define VERBOSE_TAB(a)	const auto tab = Logger::Verbose() << a << Logger::Tabs{}
#define FLOW_ERRORS(a)	Logger::Error() << a

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
				const auto& scope = ReinterpretCast<Scope>(
					static_cast<const Block&>(trait));
				result = scope.IsExecutable();
				return !result;
			},
			// Scan deeper into constructs, because they're not marked deep
			// They are deep only in respect to execution						
			[&result](const Construct& construct) noexcept {
				const auto& scope = ReinterpretCast<Scope>(
					construct.GetArgument());
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
		auto results = Any::FromState(*this);
		if (!IsEmpty()) {
			VERBOSE_TAB("Executing scope: " << *this);

			try {
				if (IsOr() && GetCount() > 1)
					ExecuteOR(environment, results, skipVerbs);
				else
					ExecuteAND(environment, results, skipVerbs);
			}
			catch (const Except::Flow&) {
				// Execution failed														
				return false;
			}
		}

		output.SmartPush(Abandon(results));
		return true;
	}

	/// Nested AND scope execution															
	///	@param environment - the environment in which scope will be executed	
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [in/out] whether to skip verbs after OR success	
	///	@return true of no errors occured												
	bool Scope::ExecuteAND(Any& environment, Any& output, bool& skipVerbs) const {
		Count executed {};
		if (IsDeep()) {
			// Nest if deep																
			executed = ForEach([&](const Block& block) {
				const auto& scope = ReinterpretCast<Scope>(block);
				Any local;
				if (!scope.Execute(environment, local, skipVerbs)) {
					VERBOSE(Logger::Red << "Deep AND flow failed: " << *this);
					Throw<Except::Flow>("Deep AND failure");
				}

				output.SmartPush(Abandon(local));
			});
		}
		else {
			executed = ForEach(
				// Nest if traits, but retain each trait 							
				[&](const Trait& trait) {
					const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
					Any local;
					if (!scope.Execute(environment, local, skipVerbs)) {
						VERBOSE(Logger::Red << "Trait AND flow failed: " << *this);
						Throw<Except::Flow>("Trait AND failure");
					}

					output.SmartPush(Trait {trait.GetTrait(), Abandon(local)});
				},
				// Nest if constructs, but retain each construct				
				[&](const Construct& construct) {
					const auto& scope = ReinterpretCast<Scope>(construct.GetArgument());
					Any local;
					if (!scope.Execute(environment, local, skipVerbs)) {
						VERBOSE(Logger::Red << "Construct AND flow failed: " << *this);
						Throw<Except::Flow>("Construct AND failure");
					}

					Construct newc {construct.GetType(), Move(local), construct};
					if (newc.StaticCreation(local))
						output.SmartPush(Abandon(local));
					else {
						// Construction failed, so just propagate construct	
						// A new attempt will be made at runtime					
						output.SmartPush(Abandon(newc));
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
						constVerb.GetArgument(),
						constVerb,
						constVerb.GetVerbState()
					};

					// Execute the verb													
					if (!Scope::ExecuteVerb(environment, verb)) {
						VERBOSE(Logger::Red << "Verb AND flow failed: " << *this);
						Throw<Except::Flow>("Verb AND failure");
					}

					output.SmartPush(Abandon(verb.mOutput));
					return true;
				}
			);
		}

		if (!executed) {
			// If this is reached, then we had non-verb content				
			// Just propagate its contents											
			output.SmartPush(static_cast<const Any&>(*this));
		}

		VERBOSE(Logger::Green << "AND scope done: " << *this);
		return true;
	}

	/// Nested OR execution																		
	///	@param context - the context in which scope will be executed			
	///	@param scope - the scope to execute												
	///	@param output - [out] verb result will be pushed here						
	///	@param skipVerbs - [out] whether to skip verbs after OR success		
	///	@return true of no errors occured												
	bool Scope::ExecuteOR(Any& environment, Any& output, bool& skipVerbs) const {
		Count executed {};
		bool localSkipVerbs {};

		if (IsDeep()) {
			// Nest if deep																
			executed = ForEach([&](const Block& block) {
				const auto& scope = 
					ReinterpretCast<Scope>(block);

				Any local;
				if (scope.Execute(environment, local, localSkipVerbs)) {
					executed = true;
					output.SmartPush(Abandon(local));
				}
			});
		}
		else {
			executed = ForEach(
				// Nest if traits, but retain each trait 							
				[&](const Trait& trait) {
					const auto& scope = 
						ReinterpretCast<Scope>(static_cast<const Block&>(trait));

					Any local;
					if (scope.Execute(environment, local)) {
						executed = true;
						output.SmartPush(Trait {trait.GetTrait(), Abandon(local)});
					}
				},
				// Nest if constructs, but retain each construct				
				[&](const Construct& construct) {
					const auto& scope = 
						ReinterpretCast<Scope>(construct.GetArgument());

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
						constVerb.GetArgument(),
						constVerb,
						constVerb.GetVerbState()
					};

					if (!Scope::ExecuteVerb(environment, verb))
						return true;

					executed = true;
					output.SmartPush(Abandon(verb.mOutput));
					return true;
				}
			);
		}

		skipVerbs |= localSkipVerbs;

		if (!executed) {
			// If this is reached, then we have non-verb flat content		
			// Just propagate it															
			output.SmartPush(static_cast<const Any&>(*this));
			++executed;
		}

		if (executed)
			VERBOSE(Logger::Green << "OR scope done: " << *this);
		else
			VERBOSE(Logger::Red << "OR scope failed: " << *this);
		return executed;
	}

	/// Integrate all parts of a verb inside this environment						
	///	@param context - [in/out] the context where verb will be integrated	
	///	@param verb - [in/out] verb to integrate										
	///	@return true of no errors occured												
	bool Scope::IntegrateVerb(Any& environment, Verb& verb) {
		// Integrate the verb source to the current context					
		Any localSource;
		if (!ReinterpretCast<Scope>(verb.mSource)
			.Execute(environment, localSource)) {
			Logger::Error() << "Error at source: " << verb.mSource;
			return false;
		}

		if (localSource.IsInvalid())
			localSource = environment;

		// Integrate the verb argument to the source								
		Any localArgument;
		if (!ReinterpretCast<Scope>(verb.GetArgument())
			.Execute(localSource, localArgument)) {
			Logger::Error() << "Error at argument: " << verb.GetArgument();
			return false;
		}

		verb.GetSource() = Abandon(localSource);
		verb.GetArgument() = Abandon(localArgument);
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
			FLOW_ERRORS("Error integrating verb: " << verb
				<< " (" << verb.GetVerb()->mToken << ")");
			return false;
		}

		if (verb.Is<Verbs::Do>()) {
			// A Do verb is done at this point, because the subverbs 		
			// inside (if any) should be done in the integration phase		
			// Just making sure that the integrated argument & source are	
			// propagated to the verb's output										
			if (verb.mOutput.IsEmpty()) {
				if (!verb.GetArgument().IsEmpty())
					verb << Move(verb.GetArgument());
				else
					verb << Move(verb.GetSource());
			}

			return true;
		}

		VERBOSE_TAB("Executing verb: " << Logger::Cyan << verb 
			<< " (" << verb.GetVerb()->mToken << ")");

		// Dispatch the verb to the context, executing it						
		// Any results should be inside verb.mOutput afterwards				
		if (!DispatchDeep(verb.mSource, verb)) {
			FLOW_ERRORS("Error executing verb: " << verb
				<< " (" << verb.GetVerb()->mToken << ")");
			return false;
		}

		VERBOSE("Executed: " << Logger::Green << verb
			<< " (" << verb.GetVerb()->mToken << ")");
		return true;
	}

} // namespace Langulus::Flow

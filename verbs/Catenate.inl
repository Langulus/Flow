#pragma once
#include "../Code.hpp"

namespace Langulus::Verbs
{

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Catenate::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Catenate(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Catenate(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Catenate::Of() noexcept {
		if constexpr (!Catenate::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Catenate(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Catenate(verb, args...);
			};
		}
	}

	/// Execute the catenation/splitting verb in a specific context				
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Catenate::ExecuteIn(T& context, Verb& verb) {
		static_assert(Catenate::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Catenate(verb);
		return verb.IsDone();
	}

	/// Default catenation/splitting in an immutable context							
	/// Produces a shallow copy of the catenated context and arguments			
	///	@param context - the block to execute in										
	///	@param verb - catenation/splitting verb										
	inline bool Catenate::ExecuteDefault(const Block& context, Verb& verb) {
		if (verb.IsMissingDeep())
			return false;

		if (verb.IsEmpty()) {
			verb << context;
			return true;
		}

		//TODO split
		TAny<Any> shallow;
		shallow << Any {context};
		shallow << verb.GetArgument();
		verb << Abandon(shallow);
		return true;
	}

	/// Default catenation/splitting in a mutable context								
	/// Reuses the context, by catenating/splitting inside it if possible		
	///	@param context - the block to execute in										
	///	@param verb - catenation/splitting verb										
	inline bool Catenate::ExecuteDefault(Block& context, Verb& verb) {
		if (verb.IsMissingDeep())
			return false;

		if (verb.IsEmpty()) {
			verb << context;
			return true;
		}

		//TODO split
		context.SmartPush(Move(verb.GetArgument()));
		verb << context;
		return true;
	}

	/// A stateless catenation - just results in RHS									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Catenate::ExecuteStateless(Verb& verb) {
		verb << Move(verb.GetArgument());
		return true;
	}

} // namespace Langulus::Verbs

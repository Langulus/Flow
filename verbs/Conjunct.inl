#pragma once
#include "../Code.hpp"

namespace Langulus::Verbs
{

	/// Default Conjunct/Disjunct verb construction										
	inline Conjunct::Conjunct()
		: Verb {RTTI::MetaVerb::Of<Conjunct>()} {}

	/// Conjunct/Disjunct verb construction by shallow copy							
	///	@param a - RHS																			
	///	@param c - the charge of the conjunction										
	///	@param sc - is the conjunction/disjunction short-circuited				
	template<CT::Data T>
	Conjunct::Conjunct(const T& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Conjunct>(), a, c, sc} {}

	/// Conjunct/Disjunct verb construction by move										
	///	@param a - RHS																			
	///	@param c - the charge of the conjunction										
	///	@param sc - is the conjunction/disjunction short-circuited				
	template<CT::Data T>
	Conjunct::Conjunct(T&& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Conjunct>(), Forward<T>(a), c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Conjunct::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Conjunct(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Conjunct(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Conjunct::Of() noexcept {
		if constexpr (!Conjunct::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Conjunct(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Conjunct(verb, args...);
			};
		}
	}

	/// Execute the conjunction/disjunction verb in a specific context			
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Conjunct::ExecuteIn(T& context, Verb& verb) {
		static_assert(Conjunct::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Conjunct(verb);
		return verb.IsDone();
	}

	/// Default conjunction/disjunction in an immutable context						
	/// Produces a shallow copy of the provided context and arguments				
	///	@param context - the block to execute in										
	///	@param verb - conjunction/disjunction verb									
	inline bool Conjunct::ExecuteDefault(const Block& context, Verb& verb) {
		if (verb.IsEmpty()) {
			verb << context;
			return true;
		}

		TAny<Any> shallow;
		if (verb.GetMass() < 0)
			shallow.MakeOr();
		shallow << Any {context};
		shallow << verb.GetArgument();
		verb << Abandon(shallow);
		return true;
	}

	/// Default conjunction/disjunction in a mutable context							
	/// Reuses the context																		
	///	@param context - the block to execute in										
	///	@param verb - conjunction/disjunction verb									
	inline bool Conjunct::ExecuteDefault(Block& context, Verb& verb) {
		if (verb.IsEmpty()) {
			verb << context;
			return true;
		}

		if (verb.GetMass() < 0)
			context.MakeOr();
		context.SmartPush(Move(verb.GetArgument()));
		verb << context;
		return true;
	}

	/// Stateless conjunction/disjunction													
	/// Essentially forwards the arguments to the output								
	///	@param verb - the conjunction/disjunction verb								
	///	@return true if verb was satisfied												
	inline bool Conjunct::ExecuteStateless(Verb& verb) {
		verb << Move(verb.GetArgument());
		return true;
	}

} // namespace Langulus::Verbs

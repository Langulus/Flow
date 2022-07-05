#pragma once
#include "../Code.hpp"

namespace Langulus::Verbs
{

	/// Conjunct/disjunct verb construction												
	///	@param s - LHS																			
	///	@param a - RHS																			
	///	@param o - result mask (optional)												
	///	@param c - the charge of the conjunction										
	///	@param sc - is the conjunction short-circuited								
	inline Conjunct::Conjunct(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Conjunct>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T>
	constexpr bool Conjunct::AvailableFor() noexcept {
		return requires (T t, Verb v) { t.Conjunct(v); };
	}

	/// Execute the conjunction/disjunction verb in a specific context			
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Conjunct::ExecuteIn<false, T>(T& context, Verb& verb) {
		static_assert(Conjunct::AvailableFor<T>(),
			"Verb is not available for this context");
		context.Associate(verb);
		return verb.IsDone();
	}

	/// Default conjunction/disjunction														
	///	@param context - the block to execute in										
	///	@param verb - conjunction verb													
	template<CT::Data T>
	bool Conjunct::ExecuteIn<true, T>(T& context, Verb& verb) {
		if (verb.GetArgument().IsEmpty()) {
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

} // namespace Langulus::Verbs

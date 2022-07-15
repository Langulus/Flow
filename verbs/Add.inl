#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_ADD(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Add/Subtract verb construction														
	///	@param s - left hand side															
	///	@param a - right hand side															
	///	@param o - result mask (optional)												
	///	@param c - the charge of the addition											
	///	@param sc - is the addition short-circuited									
	inline Add::Add(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Add>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Add::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0) {
			return
				requires (T& t, Verb& v) { t.Add(v); }
			|| requires (T& t) { t += t; }
			|| requires (T& t) { t -= t; }
			|| requires (const T& t) { {t + t} -> CT::Same<T>; }
			|| requires (const T& t) { {t - t} -> CT::Same<T>; };
		}
		else if constexpr (sizeof...(A) == 1) {
			return
				requires (T& t, Verb& v, A... a) { t.Add(v, a...); }
			|| requires (T& t, A... a) { t += (a + ...); }
			|| requires (T& t, A... a) { t -= (a - ...); }
			|| requires (const T& t, A... a) { {t + (a + ...)} -> CT::Same<T>; }
			|| requires (const T& t, A... a) { {t - (a + ...)} -> CT::Same<T>; };
		}
		else return requires (T& t, Verb& v, A... a) { t.Add(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Add::Of() noexcept {
		if constexpr (!Add::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Add(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Add(verb, args...);
			};
		}
	}

	/// Execute the add/subtract verb in a specific context							
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Add::ExecuteIn(T& context, Verb& verb) {
		static_assert(Add::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Add(verb);
		return verb.IsDone();
	}

	/// A stateless subtraction																
	/// Basically negates rhs when mass is below zero, otherwise does nothing	
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Add::ExecuteStateless(Verb& verb) {
		//TODO negate
		return false;
	}

} // namespace Langulus::Verbs


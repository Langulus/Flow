#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_MUL(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Exponentiate/Logarithm verb construction											
	///	@param s - the base number															
	///	@param a - the power																	
	///	@param o - result mask (optional)												
	///	@param c - the charge of the exponentiation/logarithm						
	///	@param sc - is the exponentiation/logarithm short-circuited				
	inline Exponent::Exponent(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Exponent>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Exponent::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Exponent(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Exponent(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Exponent::Of() noexcept {
		if constexpr (!Exponent::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Exponent(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Exponent(verb, args...);
			};
		}
	}

	/// Execute the exponentiation/logarithm verb in a specific context			
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Exponent::ExecuteIn(T& context, Verb& verb) {
		if constexpr (Exponent::AvailableFor<T>()) {
			context.Exponent(verb);
			return verb.IsDone();
		}
		else return false;
	}

} // namespace Langulus::Verbs


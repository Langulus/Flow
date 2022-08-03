#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_MUL(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Default Exponentiate/Logarithm verb construction								
	inline Exponent::Exponent()
		: Verb {RTTI::MetaVerb::Of<Exponent>()} {}

	/// Exponentiate/Logarithm verb construction by shallow-copy					
	///	@param a - the power																	
	///	@param c - the charge of the exponentiation/logarithm						
	///	@param sc - is the exponentiation/logarithm short-circuited				
	template<CT::Data T>
	Exponent::Exponent(const T& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Exponent>(), a, c, sc} {}

	/// Exponentiate/Logarithm verb construction by move								
	///	@param a - the power																	
	///	@param c - the charge of the exponentiation/logarithm						
	///	@param sc - is the exponentiation/logarithm short-circuited				
	template<CT::Data T>
	Exponent::Exponent(T&& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Exponent>(), Forward<T>(a), c, sc} {}

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
		static_assert(Exponent::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Exponent(verb);
		return verb.IsDone();
	}

} // namespace Langulus::Verbs


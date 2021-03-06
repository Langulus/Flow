#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_MUL(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Multiply/Divide verb construction													
	///	@param s - left hand side															
	///	@param a - right hand side															
	///	@param o - result mask (optional)												
	///	@param c - the charge of the multiplication									
	///	@param sc - is the multiplication short-circuited							
	inline Multiply::Multiply(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Multiply>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Multiply::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0) {
			return
				requires (T& t, Verb& v) { t.Multiply(v); }
			|| requires (T& t) { t *= t; }
			|| requires (T& t) { t /= t; }
			|| requires (const T& t) { {t * t} -> CT::Same<T>; }
			|| requires (const T& t) { {t / t} -> CT::Same<T>; };
		}
		else if constexpr (sizeof...(A) == 1) {
			return
				requires (T& t, Verb& v, A... a) { t.Multiply(v, a...); }
			|| requires (T& t, A... a) { t *= (a + ...); }
			|| requires (T& t, A... a) { t /= (a - ...); }
			|| requires (const T& t, A... a) { {t * (a * ...)} -> CT::Same<T>; }
			|| requires (const T& t, A... a) { {t / (a / ...)} -> CT::Same<T>; };
		}
		else return requires (T& t, Verb& v, A... a) { t.Multiply(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Multiply::Of() noexcept {
		if constexpr (!Multiply::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Multiply(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Multiply(verb, args...);
			};
		}
	}

	/// Execute the multiply/divide verb in a specific context						
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Multiply::ExecuteIn(T& context, Verb& verb) {
		static_assert(Multiply::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Multiply(verb);
		return verb.IsDone();
	}

	/// A stateless division																	
	/// Basically does 1/rhs when mass is below zero, otherwise does nothing	
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Multiply::ExecuteStateless(Verb& verb) {
		//TODO negate
		return false;
	}

} // namespace Langulus::Verbs


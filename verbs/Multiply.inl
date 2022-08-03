#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_MUL(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

	/// Default Multiply/Divide verb construction										
	inline Multiply::Multiply()
		: Verb {RTTI::MetaVerb::Of<Multiply>()} {}

	/// Multiply/Divide verb construction by shallow-copy								
	///	@param a - right hand side															
	///	@param c - the charge of the multiplication									
	///	@param sc - is the multiplication short-circuited							
	template<CT::Data T>
	Multiply::Multiply(const T& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Multiply>(), a, c, sc} {}

	/// Multiply/Divide verb construction by move										
	///	@param a - right hand side															
	///	@param c - the charge of the multiplication									
	///	@param sc - is the multiplication short-circuited							
	template<CT::Data T>
	Multiply::Multiply(T&& a, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Multiply>(), Forward<T>(a), c, sc} {}

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


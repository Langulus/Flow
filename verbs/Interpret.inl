#pragma once
#include "../Verb.hpp"
#include "../Serial.hpp"
#include "Do.inl"

#define VERBOSE_CONVERSION(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{
	
	/// Interpretation verb construction													
	///	@param s - what are we converting?												
	///	@param a - what are we converting to?											
	///	@param o - result mask (optional)												
	///	@param c - the charge of the conversion										
	///	@param sc - is the conversion short-circuited								
	inline Interpret::Interpret(const Any& s, const Any& a, const Any& o, const Charge& c, bool sc)
		: Verb {RTTI::MetaVerb::Of<Interpret>(), s, a, o, c, sc} {}

	/// Compile-time check if a verb is implemented in the provided type			
	///	@return true if verb is available												
	template<CT::Data T, CT::Data... A>
	constexpr bool Interpret::AvailableFor() noexcept {
		if constexpr (sizeof...(A) == 0)
			return requires (T& t, Verb& v) { t.Interpret(v); };
		else
			return requires (T& t, Verb& v, A... a) { t.Interpret(v, a...); };
	}

	/// Get the verb functor for the given type and arguments						
	///	@return the function, or nullptr if not available							
	template<CT::Data T, CT::Data... A>
	constexpr auto Interpret::Of() noexcept {
		if constexpr (!Interpret::AvailableFor<T, A...>()) {
			return nullptr;
		}
		else if constexpr (CT::Constant<T>) {
			return [](const void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<const T*>(context);
				typedContext->Interpret(verb, args...);
			};
		}
		else {
			return [](void* context, Flow::Verb& verb, A... args) {
				auto typedContext = static_cast<T*>(context);
				typedContext->Interpret(verb, args...);
			};
		}
	}

	/// Execute the interpretation verb in a specific context						
	///	@param context - the context to execute in									
	///	@param verb - the verb to execute												
	///	@return true if verb has been satisfied										
	template<CT::Data T>
	bool Interpret::ExecuteIn(T& context, Verb& verb) {
		static_assert(Interpret::AvailableFor<T>(),
			"Verb is not available for this context, this shouldn't be reached by flow");
		context.Interpret(verb);
		return verb.IsDone();
	}

	/// Execute the default verb in an immutable context								
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	inline bool Interpret::ExecuteDefault(const Block& context, Verb& verb) {
		//TODO
		return false;
	}

	/// Statically optimized interpret verb												
	///	@param from - the element to convert											
	///	@return the converted element														
	template<class TO, class FROM>
	TO Interpret::To(const FROM& from) {
		if constexpr (CT::Same<TO, FROM>) {
			// Types are the same														
			return from;
		}
		else if constexpr (CT::Convertible<FROM, TO>) {
			// Directly convert if constructs/conversion operators			
			// exist. These might throw! For example, converting Any to		
			// Text may	fail, because Text is type-constrained, and Any		
			// might not be of the same type											
			try {
				return static_cast<TO>(from);
			}
			catch (const Langulus::Exception&) {
				// Conversion failed, but not fatal - we can attempt to		
				// serialize to one of the supported types						
				if constexpr (CT::SameAsOneOf<TO, Code, Text, Debug, Bytes>)
					return Serialize<TO>(from);
			}

			Throw<Except::Convert>("Interpret::To failed");
		}
		else if constexpr (CT::SameAsOneOf<TO, Code, Text, Debug, Bytes>) {
			// No constructor/conversion operator exists, that would do		
			// the conversion, but we can rely on the serializer, if TO is	
			// supported																	
			return Serialize<TO>(from);
		}
		else LANGULUS_ASSERT("No static conversion routine exists between these types");
	}

} // namespace Langulus::Verbs


namespace Langulus
{

	/// Extend the logger to be capable of logging anything considered deep		
	///	@param lhs - the logger interface												
	///	@param rhs - the block to stringify												
	///	@return a reference to the logger for chaining								
	template<CT::Deep T>
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (Logger::A::Interface& lhs, const T& rhs) {
		return lhs << Verbs::Interpret::To<Flow::Debug>(rhs);
	}

	/// Extend the logger to be capable of logging verbs								
	///	@param lhs - the logger interface												
	///	@param rhs - the verb to stringify												
	///	@return a reference to the logger for chaining								
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (Logger::A::Interface& lhs, const Flow::Verb& rhs) {
		return lhs << Verbs::Interpret::To<Flow::Debug>(rhs);
	}

} // namespace Langulus


namespace Langulus::Anyness
{

	/// Define the otherwise undefined Langulus::Anyness::Block::AsCast			
	/// to use the interpret verb pipeline for runtime conversion					
	///	@tparam T - the type to convert to												
	///	@tparam FATAL_FAILURE - true to throw on failure, otherwise				
	///									return a default-initialized T on fail			
	///	@return the first element, converted to T										
	template<CT::Data T, bool FATAL_FAILURE>
	T Block::AsCast() const {
		// Attempt pointer arithmetic conversion first							
		try { return As<T>(); }
		catch (const Except::Access&) {}

		// If this is reached, we attempt runtime conversion by				
		// dispatching Verbs::Interpret to the first element					
		const auto meta = MetaData::Of<T>();
		Verbs::Interpret interpreter {{}, meta};
		if (!Flow::DispatchDeep(GetElementResolved(0), interpreter)) {
			// Failure																		
			if constexpr (FATAL_FAILURE)
				Throw<Except::Convert>("Unable to AsCast");
			else if constexpr (CT::Defaultable<T>)
				return {};
			else {
				LANGULUS_ASSERT(
					"Unable to AsCast to non-default-constructible type, "
					"when lack of FATAL_FAILURE demands it");
			}
		}

		// Success																			
		return interpreter.GetOutput().As<T>();
	}

} // namespace Langulus::Anyness

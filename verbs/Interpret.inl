#pragma once
#include "../Verb.hpp"
#include "../Scope.hpp"
#include "../Serial.hpp"
#include "Do.inl"

#define VERBOSE_CONVERSION(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{
	
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
		verb.ForEach([&](DMeta to) {
			if (to->CastsTo<A::Text>())
				return !InterpretTo<Text>::ExecuteDefault(context, verb);
			return true;
		});

		return verb.IsDone();
	}

	/// Execute the default verb in an immutable context								
	/// Statically optimized to avoid passing an argument								
	///	@param context - the context to execute in									
	///	@param verb - the verb instance to execute									
	///	@return true if execution was a success										
	template<class TO>
	bool InterpretTo<TO>::ExecuteDefault(const Block& context, Verb& verb) {
		if constexpr (CT::Text<TO>) {
			const auto from = context.GetType();

			// Stringify context, if it matches any of its named values		
			for (auto& named : from->mNamedValues) {
				if (from->mComparer(named->mPtrToValue, context.GetRaw())) {
					verb << Text {named->mToken};
					return true;
				}
			}
			return false;
		}
		else return false;
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
		else if constexpr (CT::Same<TO, Any>) {
			// Always interpreted as deserialization								
			if constexpr (CT::SameAsOneOf<FROM, Code, Bytes>)
				return Deserialize(from);
			else LANGULUS_ASSERT(
				"No deserializer exists between these types");
		}
		else if constexpr (CT::Convertible<FROM, TO>) {
			// Directly convert if constructs/conversion operators exist	
			return static_cast<TO>(from);
		}
		else if constexpr (CT::SameAsOneOf<TO, Code, Text, Debug, Bytes>) {
			// No constructor/conversion operator exists, that would do		
			// the conversion, but we can rely on the serializer,				
			// if TO is	supported														
			return Serialize<TO>(from);
		}
		else LANGULUS_ASSERT(
			"No static conversion routine, or dynamic serializer "
			"exists between these types");
	}

} // namespace Langulus::Verbs


namespace Langulus
{

	/// Extend the logger to be capable of logging any meta							
	///	@param lhs - the logger interface												
	///	@param rhs - the meta to stringify												
	///	@return a reference to the logger for chaining								
	template<CT::Meta T>
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (
		Logger::A::Interface& lhs, const T& rhs) noexcept {
		if constexpr (CT::Sparse<T>)
			return lhs.operator << (rhs ? rhs->mToken : Decay<T>::DefaultToken);
		else
			return lhs.operator << (rhs.mToken);
	}

	/// Extend the logger to be capable of logging anything considered deep		
	///	@param lhs - the logger interface												
	///	@param rhs - the block to stringify												
	///	@return a reference to the logger for chaining								
	template<CT::Deep T>
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (
		Logger::A::Interface& lhs, const T& rhs) {
		return lhs.operator << (Token {Verbs::Interpret::To<Flow::Debug>(DenseCast(rhs))});
	}

	/// Extend the logger to be capable of logging anything statically			
	/// convertible to Debug string															
	///	@param lhs - the logger interface												
	///	@param rhs - the verb to stringify												
	///	@return a reference to the logger for chaining								
	template<CT::Flat T>
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (
		Logger::A::Interface& lhs, const T& rhs) requires (CT::Convertible<T, Flow::Debug> && !Logger::Formattable<T>) {
		return lhs.operator << (Token {Verbs::Interpret::To<Flow::Debug>(DenseCast(rhs))});
	}

	/// Extend the logger to be capable of logging any shared pointer				
	///	@param lhs - the logger interface												
	///	@param rhs - the pointer															
	///	@return a reference to the logger for chaining								
	template<CT::Data T>
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (
		Logger::A::Interface& lhs, const Anyness::TOwned<T>& rhs) {
		if constexpr (CT::Sparse<T>) {
			const auto block = rhs.GetBlock();
			if (block.Get() == nullptr) {
				lhs << block.GetType();
				lhs << "(null)";
				return lhs;
			}
			else return lhs << (*rhs.Get());
		}
		else return lhs << (rhs.Get());
	}
	
	/// Extend the logger to be capable of logging Trait								
	///	@param lhs - the logger interface												
	///	@param rhs - the trait to stringify												
	///	@return a reference to the logger for chaining								
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (
		Logger::A::Interface& lhs, const Anyness::Trait& rhs) {
		lhs << rhs.GetTrait()
			? rhs.GetTrait()->mToken
			: RTTI::MetaTrait::DefaultToken;
		lhs << '(';
		lhs << static_cast<const Anyness::Any&>(rhs);
		lhs << ')';
		return lhs;
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
		Verbs::Interpret interpreter {meta};
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

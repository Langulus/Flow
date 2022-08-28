#pragma once
#include "../Code.hpp"
#include "Do.inl"

#define VERBOSE_ADD(a) //Logger::Verbose() << a

namespace Langulus::Verbs
{

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

	/// Directly reinterprets lhs and rhs as the provided T and uses operator	
	/// + or - on each of the elements														
	///	@tparam T - type to interpret as													
	///	@param lhs - left operand															
	///	@param rhs - right operand															
	template<CT::Data T>
	LANGULUS(ALWAYSINLINE) void Add::BatchOperator(const Block& lhs, Verb& rhs) {
		//TODO use TSIMD to batch compute
		//TODO once vulkan module is available, lock and replace the ExecuteDefault in MVulkan to incorporate compute shader for even batcher batching!!1
		//TODO detect underflows and overflows
		TAny<T> result;
		result.template Allocate<false, true>(lhs.GetCount());
		const T* ilhs = lhs.GetRawAs<T>();
		const T* const ilhsEnd = ilhs + lhs.GetCount();
		const T* irhs = rhs.GetRawAs<T>();
		T* ires = result.template GetRawAs<T>();
		if (rhs.GetMass() < 0) {
			while (ilhs != ilhsEnd)
				*(ires++) = *(ilhs++) - *(irhs++);
		}
		else {
			while (ilhs != ilhsEnd)
				*(ires++) = *(ilhs++) + *(irhs++);
		}

		rhs << Abandon(result);
	}

	/// Default add/subtract in an immutable context									
	///	@param context - the block to execute in										
	///	@param verb - add/subtract verb													
	inline bool Add::ExecuteDefault(const Block& context, Verb& verb) {
		const auto common = context.ReinterpretAs(verb);
		if (common.CastsTo<A::Number>()) {
			if (common.CastsTo<int8_t, true>()) UNLIKELY()
				BatchOperator<int8_t>(common, verb);
			else if (common.CastsTo<uint8_t, true>()) UNLIKELY()
				BatchOperator<uint8_t>(common, verb);
			else if (common.CastsTo<int16_t, true>()) UNLIKELY()
				BatchOperator<int16_t>(common, verb);
			else if (common.CastsTo<uint16_t, true>()) UNLIKELY()
				BatchOperator<uint16_t>(common, verb);
			else if (common.CastsTo<int32_t, true>())
				BatchOperator<int32_t>(common, verb);
			else if (common.CastsTo<uint32_t, true>())
				BatchOperator<uint32_t>(common, verb);
			else if (common.CastsTo<int64_t, true>())
				BatchOperator<int64_t>(common, verb);
			else if (common.CastsTo<uint64_t, true>())
				BatchOperator<uint64_t>(common, verb);
			else if (common.CastsTo<RealSP, true>()) LIKELY()
				BatchOperator<RealSP>(common, verb);
			else if (common.CastsTo<RealDP, true>()) LIKELY()
				BatchOperator<RealDP>(common, verb);
		}

		return verb.IsDone();
	}

	/// Default add/subtract in mutable context											
	///	@param context - the block to execute in										
	///	@param verb - add/subtract verb													
	inline bool Add::ExecuteDefault(Block& context, Verb& verb) {
		//TODO
		return true;
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


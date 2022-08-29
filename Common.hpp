#pragma once
#include <Langulus.Logger.hpp>
#include <Langulus.Anyness.hpp>

#ifndef LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY
#error Langulus::Flow can be compiled only with enabled LANGULUS_FEATURE_MANAGED_MEMORY
#endif

#ifndef LANGULUS_ENABLE_FEATURE_MANAGED_REFLECTION
#error Langulus::Flow can be compiled only with enabled LANGULUS_ENABLE_MANAGED_REFLECTION
#endif

LANGULUS_EXCEPTION(Flow);

namespace Langulus::Flow
{

	using namespace Anyness;
	using Anyness::Inner::Allocator;

	using RTTI::VMeta;
	using RTTI::TMeta;
	using RTTI::DMeta;
	using RTTI::CMeta;
	using RTTI::MetaData;
	using RTTI::MetaVerb;
	using RTTI::MetaTrait;
	using RTTI::MetaConst;

	class Charge;
	class Construct;
	class Code;
	class Verb;

	
	///																								
	/// Charge, carrying the four verb dimensions										
	///																								
	struct Charge {
		LANGULUS(POD) true;
		LANGULUS(NULLIFIABLE) false;

		// Mass of the verb																
		Real mMass = DefaultMass;
		// Frequency of the verb														
		Real mFrequency = DefaultFrequency;
		// Time of the verb																
		Real mTime = DefaultTime;
		// Priority of the verb															
		Real mPriority = DefaultPriority;

	public:
		static constexpr Real DefaultMass {1};
		static constexpr Real DefaultFrequency {0};
		static constexpr Real DefaultTime {0};

		static constexpr Real DefaultPriority {0};
		static constexpr Real MinPriority {-10000};
		static constexpr Real MaxPriority {+10000};

		constexpr Charge(
			Real = DefaultMass,
			Real = DefaultFrequency,
			Real = DefaultTime,
			Real = DefaultPriority
		) noexcept;

		NOD() constexpr bool operator == (const Charge&) const noexcept;

		NOD() constexpr Charge operator * (const Real&) const noexcept;
		NOD() constexpr Charge operator ^ (const Real&) const noexcept;

		NOD() constexpr Charge& operator *= (const Real&) noexcept;
		NOD() constexpr Charge& operator ^= (const Real&) noexcept;

		NOD() constexpr bool IsDefault() const noexcept;
		NOD() Hash GetHash() const noexcept;
		void Reset() noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;
	};

} // namespace Langulus::Flow
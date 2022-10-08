///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include <LangulusAnyness.hpp>

LANGULUS_EXCEPTION(Flow);
LANGULUS_EXCEPTION(Link);

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
	///	Charge, carrying the four verb dimensions										
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
		NOD() constexpr bool IsFlowDependent() const noexcept;
		NOD() Hash GetHash() const noexcept;
		void Reset() noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;
	};

} // namespace Langulus::Flow


LANGULUS_DEFINE_TRAIT(Mass,
	"Mass of anything with charge, or with physical mass");
LANGULUS_DEFINE_TRAIT(Frequency,
	"Frequency of anything with charge, or with physical frequency");
LANGULUS_DEFINE_TRAIT(Time,
	"Time of anything with charge, or with a temporal component");
LANGULUS_DEFINE_TRAIT(Priority,
	"Priority of anything with charge, or some kind of priority");
///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "../Verb.hpp"

namespace Langulus::Flow::Inner
{

	constexpr auto NoPriority = Charge::MaxPriority;
	struct MissingPast;
	struct MissingFuture;
	using Pasts = TAny<MissingPast*>;
	using Futures = TAny<MissingFuture*>;


	///																								
	///	A missing point inside a flow														
	///																								
	struct Missing {
		TAny<DMeta> mFilter;
		Any mContent;
		Real mPriority {NoPriority};

		Missing() = default;
		Missing(const TAny<DMeta>&, Real priority);
		Missing(const Block&, Real priority);

		NOD() bool Accepts(const Block&) const;
		NOD() bool IsSatisfied() const;

		bool Push(const Any&, const Block& environment);
		Any Link(const Block&, const Block& environment, bool& consumedPast) const;
		Any Collapse() const;

		operator Debug() const;
		LANGULUS_CONVERSIONS(Missing, Debug);
	};


	///																								
	///	A missing past point inside a flow												
	///																								
	struct MissingPast : public Missing {
		LANGULUS_BASES(Missing);
		using Missing::Missing;
		MissingPast();
	};


	///																								
	///	A missing future point inside a flow											
	///																								
	struct MissingFuture : public Missing {
		LANGULUS_BASES(Missing);
		using Missing::Missing;
		MissingFuture();
		static Futures Find(const Block& scope, const Real priority, Real& priorityFeedback);
	};

} // namespace Langulus::Flow::Inner

#include "Missing.inl"
///																									
/// Langulus::Entity																				
/// Copyright(C) 2013 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Verb.hpp"

namespace Langulus::Flow
{

	constexpr auto NoPriority = Charge::MaxPriority;

	class MissingPoint;
	using Futures = TAny<MissingPoint*>;
	using Pasts = TAny<MissingPoint*>;
	using Filter = TAny<DMeta>;


	///																								
	///	A missing point inside a flow														
	///																								
	class MissingPoint {
	public:
		NOD() bool IsFork() const noexcept;
		NOD() const Filter& GetFilter() const SAFETY_NOEXCEPT();
		NOD() bool Accepts(const Block&) const SAFETY_NOEXCEPT();
		NOD() const Block& GetContent() const SAFETY_NOEXCEPT();
		NOD() bool HasRelevantContent() const SAFETY_NOEXCEPT();

		void AddContent(Any&);
		void Collapse();

		Futures& AddBranch();

		void Dump() const;

		template<bool ATTEMPT, bool CLONE>
		bool FilterAndInsert(Any&, const TFunctor<Pasts(Any&)>&);
		
		template<bool ATTEMPT, bool CLONE>
		static bool Insert(MissingPoint&, Any&, Any&, const TFunctor<Pasts(Any&)>&);

	public:
		Real mPriority;
		Ptr<Any> mPack;
		Charge* mChargeFor {};
		Count mChanges = 0;
	};


	///																								
	///	A fork - place where the flow separates in several branches				
	///																								
	struct Fork {
		Ptr<Any> mRoot;
		Any mIdentity;
		bool mDedicatedIdentity = false;
		TAny<Futures> mBranches;
	};

} // namespace Langulus::Flow

#include "Missing.inl"
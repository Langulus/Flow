///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Missing.hpp"

#define VERBOSE_MISSING_POINT(a)			//Logger::Verbose() <<
#define VERBOSE_MISSING_POINT_TAB(a)	//auto tab = Logger::Verbose() << a << Logger::Tabs{}

namespace Langulus::Flow
{
   
	/// Check if the future point branches out											
	///	@return true if the future point is a branch									
	bool MissingPoint::IsFork() const noexcept {
		return mPack->Is<Fork>();
	}

	/// Get the filter part of the point													
	///	@return a reference to the filter part											
	const Filter& MissingPoint::GetFilter() const SAFETY_NOEXCEPT() {
		LANGULUS_ASSUME(DevAssumes, !IsFork(),
			"Point is a fork, and must be handled explicitly");

		if (mPack->IsMissing()) {
			const auto& filter = mPack->IsDeep() ? mPack->Get<Any>(0) : *mPack;
			LANGULUS_ASSUME(DevAssumes, filter.IsEmpty() || filter.Is<DMeta>(),
				"Bad filter format");
			return ReinterpretCast<Filter>(filter);
		}

		static const Filter fallback{};
		return fallback;
	}

	/// Get the filter part of the point													
	///	@param content - the content to push											
	///	@return a reference to the filter part											
	NOD() inline bool MissingPoint::Accepts(const Block& content) const SAFETY_NOEXCEPT() {
		if (GetFilter().IsEmpty())
			return true;

		for (auto type : GetFilter()) {
			if (content.CastsToMeta(type))
				return true;
		}

		return false;
	}

	/// Get the content part of the point													
	///	@return a reference to the filter part											
	NOD() inline const Block& MissingPoint::GetContent() const SAFETY_NOEXCEPT() {
		LANGULUS_ASSUME(DevAssumes, !IsFork(),
			"Point is a fork, and must be handled explicitly");

		static const Block fallback{};
		if (mPack->IsMissing())
			return mPack->IsDeep() ? mPack->Get<Any>(1) : fallback;
		return *mPack;
	}

	/// Get the content part of the point													
	///	@return a reference to the filter part											
	NOD() inline bool MissingPoint::HasRelevantContent() const SAFETY_NOEXCEPT() {
		auto& content = GetContent();
		if (content.IsEmpty() || GetFilter().IsEmpty())
			return false;

		bool satisfied = false;
		content.ForEachDeep([&](const Block& b) {
			if (Accepts(b))
				satisfied = true;
			return !satisfied;
		});

		return satisfied;
	}

	/// Push content to the point - always clones										
	///	@param content - the content to push											
	void MissingPoint::AddContent(Any& content) {
		LANGULUS_ASSUME(DevAssumes, !IsFork(),
			"Point is a fork, and must be handled explicitly");

		// This is reached only if !ATTEMPT											
		if (mPriority == NoPriority) {
			VERBOSE_MISSING_POINT(Logger::Purple
				<< "Point changed (no priority):");
		}
		else {
			VERBOSE_MISSING_POINT(Logger::Purple
				<< "Point changed (priority " << mPriority << "):");
		}

		VERBOSE_MISSING_POINT(Logger::Purple << " - was " << mPack);

		if (mPack->IsMissing()) {
			// Deepen the point if not deep yet, so that we keep filter		
			if (!mPack->IsDeep())
				mPack->Deepen<Any, false>();

			if (mPack->GetCount() == 1)
				*mPack << Any{};

			mPack->Get<Any>(1) << Move(content);
		}
		else {
			// Doesn't have a filter, so just push									
			*mPack << Move(content);
		}

		VERBOSE_MISSING_POINT(Logger::Purple << " - now " << mPack);
	}

	/// Collapse the point, clearing contents, polarity, filters					
	void MissingPoint::Collapse() {
		LANGULUS_ASSUME(DevAssumes, !IsFork(),
			"Point is a fork, and must be handled explicitly");

		if (mPack->IsMissing()) {
			if (mPack->IsFuture()) {
				// Remove only content if dedicated future						
				if (mPack->IsDeep() && mPack->GetCount() == 2)
					mPack->RemoveIndex(1);
			}
			else if (mPack->IsPast()) {
				// Remove everything if dedicated past								
				mPack->Reset();
			}
			else {
				// Remove only content if unspecified polarity					
				if (mPack->IsDeep() && mPack->GetCount() == 2)
					mPack->RemoveIndex(1);
			}
		}
		else mPack->Reset();
	}

	/// Add a branch, making this future point a fork									
	///	@return the new branch																
	Futures& MissingPoint::AddBranch() {
		{
			VERBOSE_MISSING_POINT_TAB(Logger::Yellow << "CREATING BRANCH IN: ");
			Dump();
		}

		if (!IsFork()) {
			// Turn a normal point to a fork											
			Fork fork;
			fork.mRoot = mPack;
			fork.mIdentity = Move(*mPack);
			fork.mDedicatedIdentity = true;
			mPack->Reset();
			mPack->MakeOr();
			mPack = Ptr<Any>::New(Move(fork));
		}

		// At this point, this future point is a fork							
		// Clone the identity, pushing it to the root							
		Fork& fork = mPack->Get<Fork>();
		if (!fork.mDedicatedIdentity) {
			// Make sure that we have a dedicated identity before				
			// making any changes														
			fork.mIdentity = fork.mIdentity.Clone();
			fork.mDedicatedIdentity = true;
		}

		*fork.mRoot << fork.mIdentity.Clone();

		// Push the new branch, interfacing that cloned identity				
		Futures branch;
		auto newPoint = Ptr<MissingPoint>::New(mPriority, fork.mRoot->As<Any*>(IndexLast));
		branch << newPoint.Get();
		fork.mBranches << Move(branch);

		{
			VERBOSE_MISSING_POINT_TAB(Logger::Yellow << "RESULTING BRANCHES: ");
			Dump();
		}

		return mPack->Get<Fork>().mBranches.Last();
	}

	/// Dump the contents of the point in the log										
	void MissingPoint::Dump() const {
		if (IsFork()) {
			for (auto& branch : mPack->Get<Fork>().mBranches) {
				VERBOSE_MISSING_POINT_TAB(Logger::Yellow << "BRANCH: ");
				for (auto p : branch)
					p->Dump();
			}
		}
		else {
			if (mPriority == NoPriority) {
				VERBOSE_MISSING_POINT(Logger::Purple
					<< "FUTURE (no priority): ");
			}
			else {
				VERBOSE_MISSING_POINT(Logger::Purple
					<< "FUTURE (priority " << mPriority << "): ");
			}

			Logger::Append() << *mPack;
		}
	}

} // namespace Langulus::Flow
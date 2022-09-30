///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Missing.hpp"
#include "Fork.hpp"
#include "../verbs/Do.inl"
#include "../verbs/Interpret.inl"

#define VERBOSE_MISSING_POINT(a)			Logger::Verbose() << a
#define VERBOSE_MISSING_POINT_TAB(a)	const auto tabs = Logger::Verbose() << a << Logger::Tabs{}
#define VERBOSE_FUTURE(a) 

namespace Langulus::Flow::Inner
{

	/// Initialize a missing point by a precompiled filter							
	///	@param filter - the filter to set												
	inline Missing::Missing(const TAny<DMeta>& filter)
		: mFilter {filter} { }

	/// Initialize a missing point by a filter, will be precompiled				
	/// i.e. all meta data definitions will be gathered								
	///	@param filter - the filter to set												
	inline Missing::Missing(const Block& filter) {
		filter.Gather(mFilter, DataState::Missing);
		mFilter.SetState(filter.GetState());
	}

	/// Check if immediate contents are accepted by the filter of this point	
	/// Verbs are always accepted																
	///	@param content - the content to check											
	///	@return true if contents are accepted											
	inline bool Missing::Accepts(const Block& content) const {
		if (mFilter.IsEmpty() || content.CastsTo<Verb, true>())
			return true;

		for (auto type : mFilter) {
			if (content.template CastsToMeta<true>(type))
				return true;
		}

		return false;
	}

	/// Check if the missing point has been satisfied by pushed contents			
	///	@return true if point was satisfied												
	inline bool Missing::IsSatisfied() const {
		if (mContent.IsEmpty() || mFilter.IsEmpty())
			return false;

		bool satisfied = false;
		mContent.ForEachDeep([&](const Block& b) {
			if (Accepts(b))
				satisfied = true;
			return !satisfied;
		});

		return satisfied;
	}

	/// Push content to the missing point, if filters allow it						
	///	@attention assumes content is a valid container								
	///	@param content - the content to push											
	inline bool Missing::Push(const Any& content) {
		bool atLeastOneSuccess = false;

		if (content.IsDeep()) {
			// Always nest deep contents, we must filter each part and		
			// make sure branches are correctly inserted in forks				
			if (content.IsOr()) {
				// We're building a fork, we should take special care to		
				// preserve the hierarchy of the branches							
				Missing fork {mFilter};
				fork.mContent.MakeOr();
				
				content.ForEach([&](const Any& subcontent) {
					atLeastOneSuccess |= fork.Push(subcontent);
				});

				mContent.SmartPush(Abandon(fork.mContent));
			}
			else {
				// Just nest																
				content.ForEach([&](const Any& subcontent) {
					atLeastOneSuccess |= Push(subcontent);
				});
			}

			return atLeastOneSuccess;
		}

		//																						
		// If reached, we're pushing flat data										
		// Let's check if there's a filter											
		if (mFilter.IsEmpty()) {
			// No filter, just push														
			if (!content.template CastsTo<Verb, true>()) {
				// Always try interpreting scope as verbs							
				Verbs::InterpretTo<Verb> interpreter;
				interpreter.ShortCircuit(false);

				if (DispatchDeep(content, interpreter)) {
					// Something was interpreted to verbs							
					//TODO integrate and attempt compile-time execution for interpreted stuff
					return Push(interpreter.GetOutput());
				}
			}

			// Scope is either verbs or something else, just push				
			//TODO still, we've got to integrate missing traits/constructs!
			mContent << content;
			return true;
		}

		//																						
		// Filters are available, interpret source as requested				
		//TODO Always add an interpretation to verbs? deprecated?
		VERBOSE_MISSING_POINT_TAB("Satisfying filter " << mFilter
			<< " by interpreting " << content);

		Verbs::Interpret interpreter {mFilter};
		interpreter.ShortCircuit(false);

		if (DispatchDeep(content, interpreter)) {
			// If results in verb skip insertion									
			// Instead delay push to an unfiltered pointer later on			
			//TODO is this really required? removed for now
			//TODO integrate and attempt compile-time execution for interpreted stuff
			return Push(interpreter.GetOutput());
		}

		// Nothing pushed to this point												
		return false;
	}

	/// Log the missing point																	
	inline Missing::operator Debug() const {
		Code result;
		result += Code::OpenScope;
		result += Verbs::Interpret::To<Debug>(mFilter);
		if (!mContent.IsEmpty()) {
			result += ", ";
			result += Verbs::Interpret::To<Debug>(mContent);
		}
		result += Code::CloseScope;
		return result;
	}

	/// Default past point																		
	inline MissingPast::MissingPast() {
		mFilter.MakePast();
	}

	/// Default future point																	
	inline MissingFuture::MissingFuture() {
		mFilter.MakeFuture();
	}







	/// Find future points inside a scope													
	///	@param scope - the scope to analyze												
	///	@param priority - the priority under which the scope falls				
	///	@param priorityFeedback - [in/out] tracks priority change forwards	
	///	@return the collection of future points, paired with verb priority	
	/*Futures MissingFuture::Find(const Block& scope, const Real priority, Real& priorityFeedback) {
		auto localPriorityFeedback = Charge::MinPriority;
		Futures result;
		const auto pushToFutures = [&]() {
			// Check if scope is a missing future									
			if (!scope.IsPast() && scope.IsMissing()) {
				VERBOSE_FUTURE(Logger::Input()
					<< "Pushed future point (priority " 
					<< priority << "): " << scope);
				auto newPoint = Ptr<MissingPoint>::New(priority, ReinterpretCast<Any>(&scope));
				result << newPoint.Get();
			}
		};

		if (scope.IsEmpty()) {
			priorityFeedback = priority;
			pushToFutures();
			return result;
		}

		if (scope.IsDeep()) {
			// Scope is deep																
			// There is no escape from this scope once entered					
			if (scope.IsOr() && scope.GetCount() > 1) {
				VERBOSE_FUTURE(const auto tab = Logger::Special()
					<< "Scanning deep fork for future points (<= priority " 
					<< priority << "): " << scope << Logger::Tabs {});

				// DEEP OR																	
				// An OR scope represents branches inside the flow				
				// Future points in those are combined under one priority	
				Fork fork;
				scope.ForEach([&](Any& branch) {
					auto branchFuturePoints = 
						FindFuturePoints(branch, priority, localPriorityFeedback);
					if (!branchFuturePoints.IsEmpty())
						fork.mBranches << Move(branchFuturePoints);
					else {
						// Even if no future point is available in the		
						// branch we are obliged to provide the branch		
						// itself as a future point								
						auto newPoint = Ptr<MissingPoint>::New(priority, &branch);
						branchFuturePoints << newPoint.Get();
						fork.mBranches << Move(branchFuturePoints);
					}
				});

				if (!fork.mBranches.IsEmpty()) {
					fork.mRoot = ReinterpretCast<Any>(&scope);
					fork.mIdentity = ReinterpretCast<Any>(scope);
					fork.mDedicatedIdentity = false;
					VERBOSE_FUTURE(Logger::Input()
						<< "Fork with " << fork.mBranches.GetCount() 
						<< " branches inserted at priority " << priority);
					auto newPoint = Ptr<MissingPoint>::New(priority, Ptr<Any>::New(Move(fork)));
					result << newPoint.Get();
				}
			}
			else {
				VERBOSE_FUTURE(const auto tab = Logger::Special()
					<< "Scanning scope for future points (<= priority "
					<< priority << "): " << scope << tab);

				// DEEP AND																	
				// Scan scope in reverse and push futures to result			
				scope.ForEachRev([&](Any& subscope) {
					auto branchFuturePoints = 
						FindFuturePoints(subscope, priority, localPriorityFeedback);
					if (!branchFuturePoints.IsEmpty())
						result.InsertBlock(Move(branchFuturePoints));
				});

				if (priority != NoPriority && priority > localPriorityFeedback && localPriorityFeedback != Charge::MinPriority) {
					// The end of an AND scope is always considered a future	
					// if priority changed on the boundary							
					VERBOSE_FUTURE(Logger::Input()
						<< "Pushed priority-boundary future point due to transition from priority "
						<< priority << " to " << localPriorityFeedback << ": " << scope);
					auto newPoint = Ptr<MissingPoint>::New(priority, ReinterpretCast<Any>(&scope));
					result << newPoint.Get();
					priorityFeedback = priority;
				}
			}

			if (localPriorityFeedback > priorityFeedback)
				priorityFeedback = localPriorityFeedback;
			pushToFutures();
			return result;
		}

		// If this is reached, then scope is flat									
		if (scope.IsOr() && scope.GetCount() > 1) {
			const auto tab = Logger::Special()
				<< "Scanning flat fork for future points (<= priority " 
				<< priority << "): " << scope << Logger::Tabs {};

			// FLAT OR																		
			Fork fork;
			for (Offset i = 0; i < scope.GetCount(); ++i) {
				auto branch = scope.GetElementResolved(i);
				auto branchFuturePoints = FindFuturePoints(
					branch, priority, localPriorityFeedback);
				if (!branchFuturePoints.IsEmpty())
					fork.mBranches << Move(branchFuturePoints);
			}

			if (!fork.mBranches.IsEmpty()) {
				fork.mRoot = ReinterpretCast<Any>(&scope);
				fork.mDedicatedIdentity = true;
				VERBOSE_FUTURE(Logger::Input()
					<< "Fork with " << fork.mBranches.GetCount() 
					<< " branches inserted at priority " << priority);
				auto newPoint = Ptr<MissingPoint>::New(priority, Ptr<Any>::New(Move(fork)));
				result << newPoint.Get();
			}
		}
		else {
			// FLAT AND																		
			scope.ForEachRev(
				[&](Verb& verb) {
					const auto nextp = verb.GetPriority();
					localPriorityFeedback = ::std::max(nextp, localPriorityFeedback);
					Futures localResult;

					// Nest in each verb argument										
					localResult = FindFuturePoints(
						verb.GetArgument(), nextp, unusedFeedback);
					if (!localResult.IsEmpty())
						result.InsertBlock(Move(localResult));

					// Nest in each verb source										
					localResult = FindFuturePoints(
						verb.GetSource(), nextp, unusedFeedback);
					if (!localResult.IsEmpty())
						result.InsertBlock(Move(localResult));
				},
				[&](Trait& trait) {
					// Nest inside traits												
					auto localResult = FindFuturePoints(
						trait, priority, localPriorityFeedback);
					if (!localResult.IsEmpty())
						result.InsertBlock(Move(localResult));
				},
				[&](Construct& construct) {
					// Nest inside constructs											
					auto localResult = FindFuturePoints(
						construct, priority, localPriorityFeedback);
					if (!localResult.IsEmpty())
						result.InsertBlock(Move(localResult));
				}
			);
		}

		if (localPriorityFeedback > priorityFeedback)
			priorityFeedback = localPriorityFeedback;

		pushToFutures();
		return result;
	}*/


	   
	/// Filter and push content to this point												
	///	@param content - the content to filter and insert							
	///	@return true if anything was pushed												
	/*template<bool ATTEMPT, bool CLONE>
	bool Missing::FilterAndInsert(Any& content, const TFunctor<Pasts(Any&)>& FindPastPoints) {
		if (content.IsDeep()) {
			// Don't allow deep content to be reset at once!					
			// Nest to prevent this														
			bool atLeastOneSuccess {};
			if (content.IsOr()) {
				content.ForEach([&](Any& subcontent) {
					atLeastOneSuccess |= FilterAndInsert<ATTEMPT, true>(subcontent, FindPastPoints);
				});

				if constexpr (!ATTEMPT)
					content.Reset();
			}
			else {
				content.ForEach([&](Any& subcontent) {
					atLeastOneSuccess |= FilterAndInsert<ATTEMPT, CLONE>(subcontent, FindPastPoints);
					if constexpr (!ATTEMPT && !CLONE) {
						if (subcontent.IsEmpty())
							content.RemoveValue(&subcontent);
					}
				});
			}

			return atLeastOneSuccess;
		}

		if (mChargeFor) {
			if constexpr (!ATTEMPT) {
				// Special case when integrating implicit charges				
				// Always succeeds, because it is not mandatory					
				Real mass = mChanges ? mChargeFor->mMass : 0;
				auto interpreter = Verbs::Interpret {MetaData::Of<Real>()}
					.ShortCircuit(false);
				if (DispatchDeep(content, interpreter)) {
					interpreter.GetOutput().ForEachDeep(
						[&mass](const Real& n) noexcept {
							mass = ConcatenateNumbers(mass, n);
						}
					);

					VERBOSE_MISSING_POINT(Logger::Purple 
						<< "Charge changed (via " << content << "): ");
					VERBOSE_MISSING_POINT(Logger::Purple
						<< " - was " << *mChargeFor);
					mChargeFor->mMass = mass;
					++mChanges;
					VERBOSE_MISSING_POINT(Logger::Purple
						<< " - now " << *mChargeFor);
					content.Reset();
				}
			}

			// Charge always succeeds													
			return true;
		}

		if (GetFilter().IsEmpty()) {
			// No filter, just push														
			if (!content.CastsTo<Verb, true>()) {
				// Try interpreting scope as verbs and try pushing them		
				// If that fails just push the scope itself						
				auto interpreter = Verbs::Interpret {MetaData::Of<Verb>()}
					.ShortCircuit(false);
				if (DispatchDeep(content, interpreter)) {
					Any inserted;
					if (Insert<ATTEMPT, CLONE>(*this, interpreter.GetOutput(), inserted, FindPastPoints)) {
						if constexpr (!ATTEMPT) {
							AddContent(inserted);
							content.Reset();
						}
						return true;
					}
				}
			}
				
			// Scope doesn't seem to be made of verbs, so just push			
			Any inserted;
			if (Insert<ATTEMPT, CLONE>(*this, content, inserted, FindPastPoints)) {
				if constexpr (!ATTEMPT) {
					AddContent(inserted);
					content.Reset();
				}
				return true;
			}

			return false;
		}

		// If filters are available, interpret source as requested			
		// Always push a verb filter if there isn't one yet					
		auto& filters = GetFilter();
		VERBOSE_MISSING_POINT_TAB("Satisfying filter " << filters 
			<< " by interpreting " << content);

		auto interpreter = Verbs::Interpret {filters}
			.ShortCircuit(false);
		if (Flow::DispatchDeep(content, interpreter)) {
			// If results in verb skip insertion, delay for unfiltered		
			bool skip {};
			interpreter.GetOutput().ForEachDeep([&skip](const Verb&) {
				skip = true;
				return false;
			});

			if (skip)
				return false;

			Any inserted;
			if (Insert<ATTEMPT, CLONE>(*this, interpreter.GetOutput(), inserted, FindPastPoints)) {
				if constexpr (!ATTEMPT) {
					AddContent(inserted);
					content.Reset();
				}
				return true;
			}
		}

		return false;
	}*/
		
	/// Helper for inserting content to a flow point									
	///	@param context - [in/out] the context we use for integration			
	///						- May be consumed													
	///	@param content - [in/out] the content to insert								
	///						- May have past-points integrated within context		
	///	@param output - [out] the resulting output									
	///	@return true if anything was pushed												
	/*template<bool ATTEMPT, bool CLONE>
	bool Missing::Insert(Missing& context, Any& content, Any& output, const TFunctor<Pasts(Any&)>& FindPastPoints) {
		if (content.IsDeep()) {
			if (!content.IsOr()) {
				// Nest AND																	
				// Subsequent insertions are allowed to consume contexts		
				bool failure {};
				content.ForEach([&](Any& b) {
					failure |= !Insert<ATTEMPT, CLONE>(context, b, output, FindPastPoints);
					return !failure;
				});

				return !failure;
			}
			else {
				// Nest OR																	
				// Subsequent insertions are NOT allowed to consume			
				// contexts - the whole context is reset after all				
				// branches have been processed										
				bool success {};
				auto localOutput = Any::FromState(content);
				content.ForEach([&](Any& b) {
					success |= Insert<ATTEMPT, true>(context, b, localOutput, FindPastPoints);
				});

				if constexpr (!ATTEMPT) {
					if (success && !localOutput.IsEmpty()) {
						context.Collapse();
						if (localOutput.GetCount() == 1)
							output.InsertBlock(localOutput);
						else
							output << Move(localOutput);
					}
				}

				return success;
			}
		}

		// If reached, then content is flat											
		// We must either clone or move the original content					
		Any localContent;
		if constexpr (!ATTEMPT) {
			if constexpr (CLONE)
				localContent = content.Clone();
			else
				localContent = Move(content);
		}
		else localContent = content;

		// First we have to integrate the insertion, by filling any			
		// past points it has, with the available context						
		auto pasts = FindPastPoints(localContent);
		Ptr<Any> pastContent;
		if (!pasts.IsEmpty()) {
			// Get the relevant context we'll be integrating past with		
			if (context.mPack->IsMissing()) {
				if (context.mPack->IsDeep() && context.mPack->GetCount() > 1)
					pastContent = context.mPack->Get<Any*>(1);
			}
			else pastContent = context.mPack;

			if (!pastContent || pastContent->IsEmpty()) {
				// If no content available for past - fail						
				// Unless there's already relevant content						
				if (context.HasRelevantContent())
					return true;

				if constexpr (!ATTEMPT) {
					VERBOSE_MISSING_POINT(Logger::Red
						<< "Can't past-integrate with empty past: " << localContent);
				}

				return false;
			}
		}

		for (auto past : pasts) {
			// Push past to the point													
			// This is the point at which context might be consumed			
			if (past->FilterAndInsert<ATTEMPT, false>(*pastContent, FindPastPoints))
				continue;
				
			// Nothing relevant found in past, so we skip inserting			
			// Unless there's already relevant content							
			if (past->mChargeFor || context.HasRelevantContent())
				continue;

			// This is optimization that minimizes branching a lot			
			if constexpr (!ATTEMPT) {
				VERBOSE_MISSING_POINT(Logger::Red
					<< "Can't past-integrate: " << localContent);
				VERBOSE_MISSING_POINT(Logger::Red
					<< "With: " << pastContent);
				VERBOSE_MISSING_POINT(Logger::Red
					<< "Requires: " << past->GetFilter());
			}

			return false;
		}

		if constexpr (!ATTEMPT)
			output << Move(localContent);
		return true;
	}*/

} // namespace Langulus::Flow::Inner

#undef VERBOSE_MISSING_POINT
#undef VERBOSE_MISSING_POINT_TAB

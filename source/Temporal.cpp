///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Temporal.hpp"
#include "Missing.hpp"

#define VERBOSE_TEMPORAL(a) Logger::Verbose() << a
#define VERBOSE_TEMPORAL_TAB(a) const auto tab = Logger::Verbose() << a << Logger::Tabs{}
#define VERBOSE_FUTURE(a) 

namespace Langulus::Flow
{

	Pasts FindPastPoints(Any&);

	/// Construct as a sub-flow																
	///	@param parent - flow producer and parent										
	Temporal::Temporal(Temporal* parent)
		: mParent {parent} { }

	/// This will essentially fork the parent flow, if any							
	///	@return the cloned flow																
	Temporal Temporal::Clone() const {
		Temporal clone;
		// Note, that the producer and owners are not cloned or copied		
		clone.mFrequencyStack = mFrequencyStack.Clone();
		clone.mTimeStack = mTimeStack.Clone();
		clone.mPriorityStack = mPriorityStack.Clone();
		clone.mPreviousTime = mPreviousTime;
		clone.mCurrentTime = mCurrentTime;
		clone.mDuration = mDuration;
		return clone;
	}

	/// Reset progress for the priority stack												
	void Temporal::Reset() {
		mPreviousTime = mCurrentTime = {};
		VERBOSE_TEMPORAL("Reset");
	}
	
	/// Compare two flows																		
	///	@param other - the flow to compare with										
	///	@return true if both flows are the same										
	bool Temporal::operator == (const Temporal& other) const {
		return mFrequencyStack == other.mFrequencyStack
			&& mTimeStack == other.mTimeStack
			&& mPriorityStack == other.mPriorityStack;
	}

	/// Check if flow contains anything executable										
	///	@return true if flow contains at least one verb								
	bool Temporal::IsValid() const {
		return !mFrequencyStack.IsEmpty()
			|| !mTimeStack.IsEmpty()
			|| !mPriorityStack.IsEmpty();
	}

	/// Dump the contents of the flow to the log											
	void Temporal::Dump() const {
		Logger::Verbose() << mPriorityStack;
		Logger::Verbose() << mTimeStack;
		Logger::Verbose() << mFrequencyStack;
	}

	/// Remove all missing states inside all output's subpacks						
	///	@param output - [in/out] the pack to fully iterate							
	void ResetPointState(Block& output) {
		output.ForEachDeep<false>([](Block& part) {
			part.MakeMissing(false);
			part.MakeNow();
			part.ForEach(
				[](Verb& v) {
					ResetPointState(v.GetSource());
					ResetPointState(v.GetArgument());
					ResetPointState(v.GetOutput());
				},
				[](Trait& v) {
					ResetPointState(v);
				},
				[](Construct& v) {
					ResetPointState(v);
				}
			);
		});
	}

	/// Remove all filters, polarization and missingness								
	/// Strictly typed contents will be finalized										
	///	@param scope - [in/out] the scope to prepare									
	void PrepareForExecution(Block& scope) {
		scope.ForEachDeep<false>([](Any& part) {
			if (part.IsMissing()) {
				if (part.IsDeep() && part.GetCount() == 2) {
					// Check if filter is of a single type and whether			
					// contents match that type or not								
					Any& filter = part.Get<Any>(0);
					Any content = part.Get<Any>(1);

					const auto meta = filter.Get<DMeta>();
					if (filter.GetCount() == 1 && !content.Is(meta)) {
						Logger::Verbose() << "Finalizing " << content << " to " << meta;
						auto finalized = Any::FromMeta(meta);
						finalized.Allocate<true>(1);

						auto catenator = Verbs::Catenate {content};
						if (Flow::DispatchFlat<false>(finalized, catenator)) {
							Logger::Verbose() << "Finalized " << content << " to " << finalized;
							content = Move(finalized);
						}
					}

					part = Move(content);
				}
				else if (!part.IsDeep())
					part.Reset();

				part.MakeMissing(false);
				part.MakeNow();
			}

			part.ForEach(
				[](Verb& v) {
					PrepareForExecution(v.GetSource());
					PrepareForExecution(v.GetArgument());
					PrepareForExecution(v.GetOutput());
				},
				[](Trait& v) {
					PrepareForExecution(v);
				},
				[](Construct& v) {
					PrepareForExecution(v);
				}
			);
		});
	}

	/// Advance the flow - moves time forward, executes stacks						
	///	@param context - the context to execute any verbs							
	///	@param dt - delta time																
	void Temporal::Update(Block& context, Time dt) {
		if (!mCurrentTime) {
			// If we're at the beginning of time - prepare for execution	
			// This will omit any compilation-stage junk that remains in	
			// the priority stack - we no longer need it							
			PrepareForExecution(mPriorityStack);

			// Now execute the priority stack										
			// It will have two products:												
			//																					
			// 1. The outputs of the execution, that is, the verb outputs	
			//		and data propagation that happens in the stack				
			//		The outputs are short-lived, and may contain some			
			//		results to queries and other temporary data					
			//																					
			//	2. The context of the execution, that changes in its own		
			//		way, and will later be used to link the next pushes		
			//		to this flow. The context transcends a flow exection and	
			//		propagates to the next push										
			//																					
			Any output;
			Any localContext {context};
			if (!mPriorityStack.Execute(localContext, output)) {
				// oh-oh - an error occurs												
				Throw<Except::Flow>("Update failed");
			}

			// Propagate the context to the next update							
			Any argument {};
			argument.MakeFuture();
			argument.MakeMissing();

			if (localContext.GetRaw() == context.GetRaw() && localContext.GetCount() == context.GetCount()) {
				mPriorityStack = Verbs::Do {argument}
					.SetSource(localContext)
					.SetPriority(8); //TODO contexts on multiple levels can be implemented via this priority modifier! Neat!
			}
			else {
				auto source = Any::Wrap(localContext, context);
				source.MakeOr();
				mPriorityStack = Verbs::Do {argument}
					.SetSource(Abandon(source))
					.SetPriority(8); //TODO contexts on multiple levels can be implemented via this priority modifier! Neat!
			}

			VERBOSE_TEMPORAL(Logger::Purple 
				<< "Flow after execution " << mPriorityStack);
		}

		if (!dt) {
			// Avoid updating anything else, if no time had passed			
			return;
		}

		// Advance the global cycler for the flow									
		mPreviousTime = mCurrentTime;
		mCurrentTime += dt;

		// Execute flows that occur periodically									
		for (auto pair : mFrequencyStack) {
			pair.mValue.mDuration += dt;
			if (pair.mValue.mDuration >= pair.mKey) {
				// Time to execute the periodic flow								
				pair.mValue.mPreviousTime = mPreviousTime;
				pair.mValue.mCurrentTime = mCurrentTime;
				pair.mValue.mDuration -= pair.mKey;

				// Update the flow														
				// It might have periodic flows inside								
				pair.mValue.Update(context, pair.mKey);
			}
		}

		// Execute flows that occur after a given point in time				
		for (auto pair : mTimeStack) {
			if (mCurrentTime < pair.mKey)
				// The time stack is sorted, so no point in continuing		
				break;

			// Update the time flow														
			// It might have periodic flows inside									
			pair.mValue.Update(context, dt);
		}
	}

	/// Execute the whole flow in a specific context									
	///	@param context - context to execute in											
	///	@param offset - time offset to set before execution						
	///	@param period - the period to execute											
	void Temporal::Execute(Block& context, const TimePoint offset, const Time period) {
		mCurrentTime = mPreviousTime = offset;
		Update(context, period);
	}

	/// Merge a flow																				
	///	@param other - the flow to merge with this one								
	void Temporal::Merge(const Temporal& other) {
		// Concatenate priority stacks												
		mPriorityStack += other.mPriorityStack;

		// Merge time stacks																
		for (auto pair : other.mTimeStack) {
			const auto found = mTimeStack.FindKeyIndex(pair.mKey);
			if (!found)
				mTimeStack.Insert(pair.mKey, Temporal {this});
			mTimeStack[pair.mKey].Merge(pair.mValue);
		};

		// Merge periodic stacks														
		for (auto pair : other.mFrequencyStack) {
			const auto found = mFrequencyStack.FindKeyIndex(pair.mKey);
			if (!found)
				mFrequencyStack.Insert(pair.mKey, Temporal {this});
			mFrequencyStack[pair.mKey].Merge(pair.mValue);
		};
	}

	/// Find past points inside a scope														
	///	@param scope - the scope to analyze												
	///	@return the collection of past points											
	Pasts FindPastPoints(Any& scope) {
		Pasts result;
		const auto pushToPasts = [&]() {
			// Check if scope is a missing future									
			if (!scope.IsFuture() && scope.IsMissing()) {
				auto newPoint = Ptr<MissingPoint>::New(Real {}, &scope);
				result << newPoint.Get();
			}
		};

		if (scope.IsEmpty()) {
			pushToPasts();
			return result;
		}

		if (scope.IsDeep()) {
			// Scope is deep																
			// There is no escape from this scope once entered					
			scope.ForEach([&](Any& subscope) {
				auto pastPoints = FindPastPoints(subscope);
				if (!pastPoints.IsEmpty())
					result.InsertBlock(Move(pastPoints));
			});

			pushToPasts();
			return result;
		}

		// If this is reached, then scope is flat									
		Pasts localResult;
		scope.ForEach(
			[&](Verb& verb) {
				// Nest in each verb output											
				localResult = FindPastPoints(verb.GetOutput());
				if (!localResult.IsEmpty())
					result.InsertBlock(Move(localResult));

				// Nest in each verb argument											
				localResult = FindPastPoints(verb.GetArgument());
				if (!localResult.IsEmpty())
					result.InsertBlock(Move(localResult));

				// Nest in each verb source											
				localResult = FindPastPoints(verb.GetSource());
				if (!localResult.IsEmpty())
					result.InsertBlock(Move(localResult));
			},
			[&](Trait& trait) {
				// Nest inside traits													
				localResult = FindPastPoints(trait);
				if (!localResult.IsEmpty())
					result.InsertBlock(Move(localResult));
			},
			[&](Construct& construct) {
				// Nest inside constructs												
				localResult = FindPastPoints(construct);
				if (!localResult.IsEmpty()) {
					result.InsertBlock(Move(localResult));

					// Each construct also implicitly seeks charge from past	
					//TODO this is a pretty radical heuristic - only constructs with missing arguments 
					// search for implicit mass, but what if you want to make a number of constructs, that are explicitly defined in the ontology?
					// works for now...
					auto missingNumber = Ptr<Any>::New(
						MetaData::Of<A::Number>());
					missingNumber->MakeMissing();
					missingNumber->MakePast();
					auto newPoint = Ptr<MissingPoint>::New(
						Real {}, missingNumber, &construct);
					result << newPoint.Get();
				}
			}
		);

		pushToPasts();
		return result;
	}

	/// Find future points inside a scope													
	///	@param scope - the scope to analyze												
	///	@param priority - the priority under which the scope falls				
	///	@param priorityFeedback - [in/out] tracks priority change forwards	
	///	@return the collection of future points, paired with verb priority	
	Futures FindFuturePoints(Block& scope, const Real priority, Real& priorityFeedback) {
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

					// Nest in each verb output										
					Real unusedFeedback = priority;
					localResult = FindFuturePoints(
						verb.GetOutput(), nextp, unusedFeedback);
					if (!localResult.IsEmpty())
						result.InsertBlock(Move(localResult));

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
	}

	/// Push a scope of elements to the flow, branching if required, removing	
	/// old futures if consumed by past requests											
	///	@param futures - the future points, where scopes can be inserted		
	///	@param scope - the scope to push													
	///	@param branchOut - whether or not to push to a branch						
	///	@return true if at least one element been pushed successfully			
	template<bool ATTEMPT, bool CLONE>
	bool InnerPush(Futures& futures, const Block& content, bool branchOut) {
		if (content.IsEmpty())
			return false;

		bool done {};
		if (content.IsDeep()) {
			// Content is deep															
			if (content.IsOr() && content.GetCount() > 1) {
				// Content is deep OR													
				// First do a dry-run to check how many branches we'll need	
				TAny<Count> anticipatedBranches;

				Count counter {};
				content.ForEach([&](const Block& subscope) {
					if (InnerPush<true, true>(futures, subscope, branchOut))
						anticipatedBranches << counter;
					++counter;
				});

				if (anticipatedBranches.IsEmpty())
					return false;

				if (anticipatedBranches.GetCount() == 1) {
					// Only one success is anticipated, so we do not need		
					// any branching - push everything conventionally to		
					// the future point. This saves a LOT of overhead.			
					InnerPush<ATTEMPT, CLONE>(futures, 
						content.Get<Block>(anticipatedBranches[0]), branchOut);
				}
				else for (auto& index : anticipatedBranches) {
					// Make future point a fork, if not a fork yet,				
					// pushing each subpack to a separate branch					
					auto& branch = content.Get<Block>(anticipatedBranches[index]);
					VERBOSE_TEMPORAL_TAB(Logger::Yellow << "INSERTING BRANCH: " << branch);
					InnerPush<ATTEMPT, true>(futures, branch, true);
				}

				done = true;
			}
			else {
				// Content is deep AND													
				// It is safe to push each subscope individually				
				content.ForEach([&](const Block& subscope) {
					done |= InnerPush<ATTEMPT, CLONE>(futures, subscope, branchOut);
				});
			}

			return done;
		}

		// If this is reached, then scope is flat									
		if (content.IsOr() && content.GetCount() > 1) {
			// Scope is flat OR															
			// First do a dry-run to check how many branches we'll need		
			TAny<Count> anticipatedBranches;
			for (Offset i = 0; i < content.GetCount(); ++i) {
				if (InnerPush<true, true>(futures, content.GetElementResolved(i), branchOut))
					anticipatedBranches << i;
			};

			if (anticipatedBranches.IsEmpty())
				return false;

			if (anticipatedBranches.GetCount() == 1) {
				// Only one success is anticipated, so we do not need			
				// any branching - push everything conventionally to the		
				// future point. This saves a LOT of overhead.					
				InnerPush<ATTEMPT, CLONE>(futures, 
					content.GetElement(anticipatedBranches[0]), branchOut);
			}
			else for (auto& index : anticipatedBranches) {
				// Make future point a fork, if not a fork yet,					
				// pushing each subpack to a separate branch						
				auto branch = content.GetElement(anticipatedBranches[index]);
				VERBOSE_TEMPORAL_TAB(Logger::Yellow << "INSERTING BRANCH: " << branch);
				InnerPush<ATTEMPT, true>(futures, branch, true);
			}

			return true;
		}
		
		// Scope is flat AND																
		// It is safe to just push each verb individually						
		for (Offset i = 0; i < content.GetCount(); ++i) {
			auto element = content.GetElementResolved(i);
			for (auto future : futures) {
				if (element.Is<Verb>()) {
					// Skip future points that are below a verb's priority	
					if (future->mPriority < element.Get<Verb>().GetPriority())
						continue;
				}

				if (!branchOut || ATTEMPT) {
					if (future->IsFork()) {
						// Multicast in parallel futures...							
						for (auto& branch : future->mPack->Get<Fork>().mBranches) {
							done = InnerPush<ATTEMPT, true>(branch, element, false);
						}
					}
					else {
						// Push the element												
						Any wrappedElement {element};
						done = future->FilterAndInsert<ATTEMPT, CLONE>(wrappedElement, FindPastPoints);
					}
				}
				else {
					// Push element in a new branch...								
					auto& branch = future->AddBranch();
					done = InnerPush<ATTEMPT, true>(branch, element, false);
				}

				// Break on success, take care of the next element				
				if (done)
					break;
			}
		}

		return done;
	}

	/// Push a scope of verbs and data to the flow										
	///	@param scope - the scope to analyze and push									
	///	@return true if anything was pushed to the flow								
	bool Temporal::Push(const Any& scope) {
		if (scope.IsEmpty())
			return true;

		VERBOSE_TEMPORAL_TAB(this << ": Pushing: " << scope);

		// Collect all future points inside the priority stack				
		// those points might or might not have filters;						
		// all the points are paired with a priority if contained in		
		// verbs																				
		auto smallestPriority = NoPriority;
		auto futures = FindFuturePoints(mPriorityStack, NoPriority, smallestPriority);

		// Always push entire priority stack as the last option				
		auto newPoint = Ptr<MissingPoint>::New(NoPriority, &mPriorityStack);
		futures << newPoint.Get();

		#if LANGULUS_DEBUG()
			VERBOSE_TEMPORAL(Logger::Purple << "=================");
			for (const auto future : futures)
				future->Dump();
		#endif

		return InnerPush<false, false>(futures, scope, false);
	}

} // namespace Langulus::Flow

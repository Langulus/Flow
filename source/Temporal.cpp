///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#include "Temporal.hpp"
#include "inner/Missing.hpp"
#include "inner/Fork.hpp"

#define VERBOSE_TEMPORAL(a)		Logger::Verbose() << *this << ": " << a
#define VERBOSE_TEMPORAL_TAB(a)	const auto tab = Logger::Verbose() << *this << ": " << a << Logger::Tabs{}

namespace Langulus::Flow
{

	/// Default constructor, add the initial missing future point					
	///	@param environment - the initial flow environment							
	Temporal::Temporal(const Any& environment) {
		mEnvironment = environment;
		mPriorityStack << Inner::MissingFuture {};
	}

	/// Construct as a sub-flow																
	///	@attention assumes parent is a valid pointer									
	///	@param parent - the parent flow													
	///	@param background - the background charge, as provided from parent	
	Temporal::Temporal(Temporal* parent, const State& state)
		: mParent {parent}
		, mState {state} { }

	/// Clone the flow, but fork the parent flow, if any								
	///	@return the cloned flow																
	Temporal Temporal::Clone() const {
		Temporal clone;
		// Note, that the parent flow is never cloned							
		clone.mFrequencyStack = mFrequencyStack.Clone();
		clone.mTimeStack = mTimeStack.Clone();
		clone.mPriorityStack = mPriorityStack.Clone();
		clone.mPreviousTime = mPreviousTime;
		clone.mCurrentTime = mCurrentTime;
		clone.mDuration = mDuration;
		clone.mState = mState;
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

	/// Advance the flow - moves time forward, executes stacks						
	///	@param dt - delta time																
	void Temporal::Update(Time dt) {
		if (!mCurrentTime) {
			// If we're at the beginning of time - prepare for execution	
			auto collapsed = Collapse(mPriorityStack);

			// Now execute the collapsed priority stack							
			Any output;
			if (!collapsed.Execute(mEnvironment, output))
				LANGULUS_THROW(Flow, "Update failed");

			// Then, set the priority stack to the output, by wrapping it	
			// in a hight-priority Do verb with future attachment				
			// This guarantees, that a Push is possible after the Update	
			Any future; future.MakeFuture();
			mPriorityStack = Verbs::Do {future}
				.SetSource(Abandon(output))
				.SetPriority(8);

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
				pair.mValue.Update(pair.mKey);
			}
		}

		// Execute flows that occur after a given point in time				
		for (auto pair : mTimeStack) {
			if (mCurrentTime < mState.mStart + pair.mKey)
				// The time stack is sorted, so no point in continuing		
				break;

			// Update the time flow														
			// It might have periodic flows inside									
			pair.mValue.Update(dt);
		}
	}

	/// Merge a flow																				
	///	@param other - the flow to merge with this one								
	void Temporal::Merge(const Temporal& other) {
		// Concatenate priority stacks												
		mPriorityStack += other.mPriorityStack;

		// Merge time stacks																
		for (auto pair : other.mTimeStack) {
			const auto found = mTimeStack.FindKeyIndex(pair.mKey);
			if (!found) {
				const State state {
					TimePoint {mState.mStart + pair.mKey},
					Time {mState.mTime + pair.mKey},
					mState.mPeriod
				};

				mTimeStack.Insert(pair.mKey, Temporal {this, state});
			}

			mTimeStack[pair.mKey].Merge(pair.mValue);
		};

		// Merge periodic stacks														
		for (auto pair : other.mFrequencyStack) {
			const auto found = mFrequencyStack.FindKeyIndex(pair.mKey);
			if (!found) {
				const State state {
					mState.mStart,
					mState.mTime,
					pair.mKey
				};

				mFrequencyStack.Insert(pair.mKey, Temporal {this, state});
			}

			mFrequencyStack[pair.mKey].Merge(pair.mValue);
		};
	}

	/// Push a scope of elements to the flow, branching if required, removing	
	/// old futures if consumed by past requests											
	///	@param futures - the future points, where scopes can be inserted		
	///	@param scope - the scope to push													
	///	@param branchOut - whether or not to push to a branch						
	///	@return true if at least one element been pushed successfully			
	/*template<bool ATTEMPT, bool CLONE>
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
	}*/

	/// Push a scope of verbs and data to the flow										
	///	@attention assumes argument is a valid scope									
	///	@param scope - the scope to analyze and push									
	///	@return true if anything was pushed to the flow								
	bool Temporal::Push(Any scope) {
		VERBOSE_TEMPORAL_TAB("Pushing: " << scope);

		// Compile pushed scope to an intermediate format						
		auto compiled = Compile(scope);
		VERBOSE_TEMPORAL("Compiled to: " << compiled);

		// Link new scope with the available stacks								
		const bool done = Link(compiled, mPriorityStack);
		VERBOSE_TEMPORAL(Logger::Purple << "Flow state: " << mPriorityStack);
		return done;
	}

	/// This will omit any compile-time junk that remains in the provided		
	/// scope, so we can execute it conventionally										
	///	@param scope - the scope to collapse											
	///	@return the collapsed scope														
	Scope Temporal::Collapse(const Block& scope) {
		Scope result;
		if (scope.IsOr())
			result.MakeOr();

		if (scope.IsDeep()) {
			// Nest deep scopes															
			scope.ForEach([&](const Block& subscope) {
				auto collapsed = Collapse(subscope);
				if (!collapsed.IsEmpty())
					result << Abandon(collapsed);
			});

			if (result.GetCount() < 2)
				result.MakeAnd();
			return Abandon(result);
		}

		const auto done = scope.ForEach(
			[&](const Trait& subscope) {
				auto collapsed = Collapse(subscope);
				if (!collapsed.IsEmpty()) {
					result << Trait {
						subscope.GetTrait(), 
						Abandon(collapsed)
					};
				}
			},
			[&](const Construct& subscope) {
				auto collapsed = Collapse(subscope);
				if (!collapsed.IsEmpty()) {
					result << Construct {
						subscope.GetType(),
						Abandon(collapsed),
						subscope.GetCharge()
					};
				}
			},
			[&](const Verb& subscope) {
				auto collapsedArgument = Collapse(subscope.GetArgument());
				if (!collapsedArgument.IsEmpty()) {
					Verb v {
						subscope.GetVerb(),
						Abandon(collapsedArgument),
						subscope.GetCharge(),
						subscope.GetVerbState()
					};
					v.SetSource(Collapse(subscope.GetSource()));
					result << Abandon(v);
				}
			},
			[&](const Inner::MissingPast& subscope) {
				if (subscope.IsSatisfied())
					result << subscope.mContent;
			},
			[&](const Inner::MissingFuture& subscope) {
				if (subscope.IsSatisfied())
					result << subscope.mContent;
			}
		);

		if (!done && !scope.IsEmpty())
			result = scope;
		if (result.GetCount() < 2)
			result.MakeAnd();
		return Abandon(result);
	}

	/// Compiles a scope into an intermediate form, used by the flow				
	///	@attention assumes argument is a valid scope									
	///	@param scope - the scope to compile												
	///	@return the compiled scope															
	Scope Temporal::Compile(const Block& scope) {
		Scope result;
		if (scope.IsOr())
			result.MakeOr();

		if (scope.IsPast()) {
			// Convert the scope to a MissingPast intermediate format		
			result = Inner::MissingPast {scope};
			return Abandon(result);
		}
		else if (scope.IsFuture()) {
			// Convert the scope to a MissingFuture intermediate format		
			result = Inner::MissingFuture {scope};
			return Abandon(result);
		}
		else if (scope.IsDeep()) {
			// Nest deep scopes															
			scope.ForEach([&](const Block& subscope) {
				result << Compile(subscope);
			});
			return Abandon(result);
		}

		const auto done = scope.ForEach(
			[&](const Trait& subscope) {
				result << Trait {
					subscope.GetTrait(), 
					Compile(subscope)
				};
			},
			[&](const Construct& subscope) {
				result << Construct {
					subscope.GetType(),
					Compile(subscope),
					subscope.GetCharge()
				};
			},
			[&](const Verb& subscope) {
				Verb v {
					subscope.GetVerb(),
					Compile(subscope.GetArgument()),
					subscope.GetCharge(),
					subscope.GetVerbState()
				};
				v.SetSource(Compile(subscope.GetSource()));
				result << Abandon(v);
			}
		);

		if (!done) {
			// Just propagate content													
			result = ReinterpretCast<Scope>(scope);
		}

		return Abandon(result);
	}

	/// Links the missing past points of the provided scope, with the missing	
	/// future point (or any nested future points inside).							
	/// Anything could be pushed to provided future point as a fallback,			
	/// as long as state and filters allows it!											
	///	@attention assumes argument is a valid scope									
	///	@param scope - the scope to link													
	///	@param stack - [in/out] the stack to link with								
	///	@return true if scope was linked successfully								
	bool Temporal::Link(const Scope& scope, Inner::MissingFuture& future) const {
		// Attempt linking to the contents first									
		if (Link(scope, future.mContent))
			return true;

		//																						
		// If reached, then future point is flat and boring, fallback by	
		// directly linking against it												
		VERBOSE_TEMPORAL_TAB("Linking to: " << future.mContent);
		return future.Push(scope, mEnvironment);
	}

	/// Links the missing past points of the provided scope, with the missing	
	/// future points of the provided stack. But anything new could go into		
	/// old future points, as long as state and filters allows it!					
	///	@attention assumes argument is a valid scope									
	///	@param scope - the scope to link													
	///	@param stack - [in/out] the stack to link with								
	///	@return true if scope was linked successfully								
	bool Temporal::Link(const Scope& scope, Block& stack) const {
		bool atLeastOneSuccess = false;

		if (stack.IsDeep()) {
			// Nest deep stack															
			stack.ForEachRev([&](Block& substack) {
				atLeastOneSuccess |= Link(scope, substack);
				// Continue linking only if the stack is branched				
				return !(stack.IsOr() && atLeastOneSuccess);
			});
			return atLeastOneSuccess;
		}

		// Iterate backwards - the last future points are always most		
		// relevant for linking															
		// Lets start, by scanning all future points in the available		
		// stack. Scope will be duplicated for each encountered branch		
		stack.ForEachRev(
			[&](Trait& substack) {
				atLeastOneSuccess |= Link(scope, substack);
				// Continue linking only if the stack is branched				
				return !(stack.IsOr() && atLeastOneSuccess);
			},
			[&](Construct& substack) {
				atLeastOneSuccess |= Link(scope, substack);
				// Continue linking only if the stack is branched				
				return !(stack.IsOr() && atLeastOneSuccess);
			},
			[&](Verb& substack) {
				if (Link(scope, substack.GetArgument())) {
					atLeastOneSuccess = true;
					// Continue linking only if the stack is branched			
					return !stack.IsOr();
				}
				if (Link(scope, substack.GetSource())) {
					atLeastOneSuccess = true;
					// Continue linking only if the stack is branched			
					return !stack.IsOr();
				}

				return true;
			},
			[&](Inner::MissingFuture& future) {
				atLeastOneSuccess |= Link(scope, future);
				return !(stack.IsOr() && atLeastOneSuccess);
			}
		);

		return atLeastOneSuccess;
	}

	/// Stringify the context (shows class type and an identifier)					
	Temporal::operator Debug() const {
		return IdentityOf(this);
	}

} // namespace Langulus::Flow

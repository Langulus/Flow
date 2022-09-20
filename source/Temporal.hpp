///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Resolvable.hpp"
#include "Time.hpp"

namespace Langulus::Flow
{

	///																								
	///	Temporal flow																			
	///																								
	///	Gives temporality to anything, by providing a time gradient.			
	///	Can be used to select time points and temporal context.					
	///	It registers all executed verbs and automatically executes them on	
	/// update, if periodic or delayed. It has buckets for verbs that occur at	
	/// specific time and/or period. An analogy for a flow is a git repository	
	/// where you record all changes (executed actions). You can, therefore,	
	/// fork, branch, etc...																	
	///	The flow can also act as a session serializer - you can use it to		
	/// record the sequence you used to make your game, play your game, or		
	/// truly anything you can imagine, that can be described by a sequence		
	/// of actions. Which is practically everything there is.						
	///	You can execute scripts with missing future/past/any elements in		
	/// them, which means that the temporal flow acts as a time-based linker,	
	/// that actively seeks the past and future inputs for suitable data to		
	/// complete your scripts at runtime.													
	///																								
	class Temporal final {
	private:
		Temporal* mParent {};

		// Increments on each call to Update()										
		TimePoint mPreviousTime;
		TimePoint mCurrentTime;

		// Accumulated flow duration													
		Time mDuration;

		// Priority stack																	
		Scope mPriorityStack;

		// Verb temporal stack - flows that trigger at given time			
		TMap<TimePoint, Temporal> mTimeStack;

		// Verb frequency stack - flows that trigger periodically			
		TUnorderedMap<Time, Temporal> mFrequencyStack;

	public:
		Temporal(const Temporal&) = delete;
		Temporal(Temporal* parent = nullptr);
		Temporal(Temporal&&) noexcept = default;

		Temporal& operator = (Temporal&&) noexcept = default;
		Temporal& operator = (const Temporal&) = delete;

	public:
		NOD() bool operator == (const Temporal&) const;

		NOD() Temporal Clone() const;
		NOD() bool IsValid() const;

		void Dump() const;
		void Merge(const Temporal&);
		bool Push(const Any&);

		void Reset();
		void Update(Block&, Time);
		void Execute(Block&, TimePoint = {}, Time = {});
	};

} // namespace Langulus::Flow


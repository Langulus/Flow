///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Missing.hpp"
#include "verbs/Do.inl"
#include "verbs/Interpret.inl"

#define VERBOSE_MISSING_POINT(a) Logger::Verbose() << a
#define VERBOSE_MISSING_POINT_TAB(a) const auto tabs = Logger::Verbose() << a << Logger::Tabs{}

namespace Langulus::Flow
{

	/// Number of digits in a value															
	/// Credit goes to http://stackoverflow.com/questions/1489830					
	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(uint8_t x) noexcept {
		return (x < 10u ? 1 : (x < 100u ? 2 : 3));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(int8_t x) noexcept {
		return CountDigits(static_cast<uint8_t>(::std::abs(x)));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(uint16_t x) noexcept {
		return (x < 10u ? 1 : (x < 100u ? 2 : (x < 1000u ? 3 : (x < 10000u ? 4 : 5))));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(int16_t x) noexcept {
		return CountDigits(static_cast<uint16_t>(::std::abs(x)));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(uint32_t x) noexcept {
		return
			(x < 10u ? 1 :
			(x < 100u ? 2 :
			(x < 1000u ? 3 :
			(x < 10000u ? 4 :
			(x < 100000u ? 5 :
			(x < 1000000u ? 6 :
			(x < 10000000u ? 7 :
			(x < 100000000u ? 8 :
			(x < 1000000000u ? 9 : 10)))))))));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count pcNumDigits(int32_t x) noexcept {
		return CountDigits(static_cast<uint32_t>(::std::abs(x)));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(uint64_t x) noexcept {
		return
			(x < 10ull ? 1 :
			(x < 100ull ? 2 :
			(x < 1000ull ? 3 :
			(x < 10000ull ? 4 :
			(x < 100000ull ? 5 :
			(x < 1000000ull ? 6 :
			(x < 10000000ull ? 7 :
			(x < 100000000ull ? 8 :
			(x < 1000000000ull ? 9 :
			(x < 10000000000ull ? 10 :
			(x < 100000000000ull ? 11 :
			(x < 1000000000000ull ? 12 :
			(x < 10000000000000ull ? 13 :
			(x < 100000000000000ull ? 14 :
			(x < 1000000000000000ull ? 15 :
			(x < 10000000000000000ull ? 16 :
			(x < 100000000000000000ull ? 17 :
			(x < 1000000000000000000ull ? 18 :
			(x < 10000000000000000000ull ? 19 : 20
		)))))))))))))))))));
	}

	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(int64_t x) noexcept {
		// http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs	
		int const mask = x >> (sizeof(int64_t) * 8 - 1);
		return CountDigits(static_cast<uint64_t>((x + mask) ^ mask));
	}

	/// Count digits in real numbers															
	/// The dot in the real number is considered a digit, too						
	///	@param x - real number to cound digits of										
	template<CT::Real T>
	NOD() constexpr LANGULUS(ALWAYSINLINE) Count CountDigits(T x) noexcept {
		T floored;
		T fraction {::std::abs(::std::modf(x, &floored))};
		if (fraction == 0)
			return CountDigits(static_cast<uint64_t>(floored));

		floored = ::std::abs(floored);
		T limit {1};
		Count fract_numbers {};
		while (fraction < limit && limit < T {1000}) {
			fraction *= T {10};
			limit *= T {10};
			++fract_numbers;
		}

		return CountDigits(static_cast<uint64_t>(floored)) + fract_numbers + Count {1};
	}

	/// Concatenate two numbers																
	///	@param lhs - left number															
	///	@param rhs - right number															
	///	@return the concatenation of the two numbers									
	template<CT::Number T>
	NOD() LANGULUS(ALWAYSINLINE) T ConcatenateNumbers(const T& lhs, const T& rhs) {
		T result {lhs};
		result *= ::std::pow(T {10}, static_cast<T>(CountDigits(rhs)));
		result += rhs;
		return result;
	}

   
	/// Filter and push content to this point												
	///	@param content - the content to filter and insert							
	///	@return true if anything was pushed												
	template<bool ATTEMPT, bool CLONE>
	bool MissingPoint::FilterAndInsert(Any& content, const TFunctor<Pasts(Any&)>& FindPastPoints) {
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
			if (!content.Is<Verb>()) {
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
	}
		
	/// Helper for inserting content to a flow point									
	///	@param context - [in/out] the context we use for integration			
	///						- May be consumed													
	///	@param content - [in/out] the content to insert								
	///						- May have past-points integrated within context		
	///	@param output - [out] the resulting output									
	///	@return true if anything was pushed												
	template<bool ATTEMPT, bool CLONE>
	bool MissingPoint::Insert(MissingPoint& context, Any& content, Any& output, const TFunctor<Pasts(Any&)>& FindPastPoints) {
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
	}

} // namespace Langulus::Flow

#undef VERBOSE_MISSING_POINT
#undef VERBOSE_MISSING_POINT_TAB

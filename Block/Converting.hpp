#pragma once
#include "../Verb.hpp"

LANGULUS_DECLARE_EXCEPTION(MissingConversion)

#define PC_VERBOSE_CONVERSION(a) //pcLogVerbose << a

namespace PCFW::Flow
{
	namespace InnerInterpretation
	{

		/// Common interpret call, that can be assigned as an ability				
		///	@tparam FROM - the type of the context we're converting				
		///	@tparam TO - the type we're converting to									
		///	@param what - memory instance to interpret FROM							
		///	@param verb - the verb to satisfy											
		template<class FROM, class TO>
		void pcInterpret(void* what, Verb& verb) {
			TO converted;
			Memory::TConverter<FROM, TO>::Convert(
				*static_cast<const FROM*>(what), converted);
			verb << converted;
		}

		/// Common interpret call, that calls the low-level converter				
		/// for a type and then nests until all types are converted					
		///	@tparam FROM - the type of the context we're converting				
		///	@tparam HEAD, TAIL... - the types we're able to convert to			
		///	@param what - memory instance to interpret								
		///	@param verb - the verb to satisfy											
		///	@param fromMeta - the type of the instance we're converting			
		///	@param toMeta - the type we're converting to								
		///	@param distance - [in/out] the shortest distance between types		
		///	@param chosen - [in/out] the closest chosen converter					
		template<class FROM, class HEAD, class... TAIL>
		void Splitter(void* what, Verb& verb, DMeta fromMeta, DMeta toMeta, pcptr& distance, FVerb& chosen) {
			if constexpr (Convertible<FROM, HEAD>) {
				// Check distance between runtime request and static type	
				static auto splitMeta = MetaData::Of<HEAD>();
				const auto typeDistance = std::min(
					toMeta->GetDistanceTo(splitMeta), 
					splitMeta->GetDistanceTo(toMeta)
				);

				if (typeDistance < distance) {
					// If it is closer, set the chosen converter					
					distance = typeDistance;
					chosen = pcInterpret<FROM, HEAD>;
				}
			}

			if constexpr (sizeof...(TAIL) > 0)
				Splitter<FROM, TAIL...>(what, verb, fromMeta, toMeta, distance, chosen);
			else {
				(what); (verb); (fromMeta); (toMeta); (distance); (chosen);
			}
		}

	} // namespace InnerInterpretation


	/// Common interpret call that attempts conversion to multiple types			
	///	@tparam FROM - the type of the context we're converting					
	///	@tparam HEAD, TAIL... - the types we're able to convert to				
	///	@param what - memory instance to interpret									
	///	@param verb - the verb to satisfy												
	template<class FROM, class HEAD, class... TAIL>
	void pcInterpretMultiple(void* what, Verb& verb) {
		if constexpr (Same<FROM, HEAD>) {
			// Skip matching types														
			(what);
			(verb);
			return;
		}
		else {
			static auto fromMeta = MetaData::Of<FROM>();
			const auto doer = [&](DMeta type) {
				// Find the closest type												
				FVerb chosen;
				pcptr typeDistance = std::numeric_limits<pcptr>::max();

				// Scan all available types in HEAD and TAIL, finding the	
				// closest distance to FROM											
				InnerInterpretation::Splitter<FROM, HEAD, TAIL...
				>(what, verb, fromMeta, type, typeDistance, chosen);

				if (chosen) {
					// If a custom converter was found - use it					
					chosen(what, verb);
				}
			};

			// For each DataID or MetaData inside verb argument				
			verb.GetArgument().ForEachDeep([&doer](const Block& group) {
				EitherDoThis
					group.ForEach([&doer](const DataID& t) { doer(t.GetMeta()); })
				OrThis
					group.ForEach([&doer](const MetaData* t) { doer(t); });
			});
		}
	}

	/// Interpret to multiple types via a type list										
	template<class FROM, template<class...> class LIST, class... T>
	constexpr auto pcInterpretList(const LIST<T...>&) {
		return pcInterpretMultiple<FROM, T...>;
	}

} // namespace PCFW::Flow

#define REFLECT_CONVERSIONS(...) \
	MEMBER_VERB_EXT(Interpret, (::PCFW::Flow::pcInterpretMultiple<ME, __VA_ARGS__>))
#define REFLECT_CONVERSIONS_EXT(owner, ...) \
	MEMBER_VERB_EXT(Interpret, (::PCFW::Flow::pcInterpretMultiple<owner, __VA_ARGS__>))

#define REFLECT_CONVERSION_LIST(LIST) \
	MEMBER_VERB_EXT(Interpret, (::PCFW::Flow::pcInterpretList<ME>(LIST{})))
#define REFLECT_CONVERSION_LIST_EXT(owner, LIST) \
	MEMBER_VERB_EXT(Interpret, (::PCFW::Flow::pcInterpretList<owner>(LIST{})))

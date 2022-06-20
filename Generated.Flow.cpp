#include "include/PCFW.Flow.hpp"

PC_MONOPOLIZE_MEMORY(0)

//TODO automatically generate this file

namespace PCFW::Flow
{

	REFLECT_BEGIN(Verb)
		REFLECT_INFO("a single GASM statement")
		REFLECT_BEGIN_MEMBERS
			REFLECT_MEMBER_TRAIT(mVerb.mID, Type),
			REFLECT_MEMBER_TRAIT(mVerb.mCharge.mMass, Mass),
			REFLECT_MEMBER_TRAIT(mVerb.mCharge.mFrequency, Frequency),
			REFLECT_MEMBER_TRAIT(mVerb.mCharge.mTime, Time),
			REFLECT_MEMBER_TRAIT(mVerb.mCharge.mPriority, Priority),
			REFLECT_MEMBER_TRAIT(mSource, Context),
			REFLECT_MEMBER_TRAIT(mArgument, Argument),
			REFLECT_MEMBER_TRAIT(mOutput, Output)
		REFLECT_END_MEMBERS
		REFLECT_BEGIN_ABILITIES
			REFLECT_CONVERSIONS(Debug,GASM)
		REFLECT_END_ABILITIES
	REFLECT_END

	REFLECT_BEGIN(ChargedVerbID)
		REFLECT_INFO("a charged verb ID")
		REFLECT_BEGIN_ABILITIES
			REFLECT_CONVERSIONS(GASM)
		REFLECT_END_ABILITIES
	REFLECT_END

	REFLECT_BEGIN(Construct)
		REFLECT_INFO("a nested type descriptor")
		REFLECT_BEGIN_MEMBERS
			REFLECT_MEMBER_TRAIT(mHeader, Type),
			REFLECT_MEMBER_TRAIT(mArguments, Argument)
		REFLECT_END_MEMBERS
		REFLECT_BEGIN_ABILITIES
			REFLECT_CONVERSIONS(Debug,GASM)
		REFLECT_END_ABILITIES
	REFLECT_END

	REFLECT_BEGIN(GASM)
		REFLECT_INFO("a GASM code container")
		REFLECT_BEGIN_BASES
			REFLECT_BASE(ACode),
			REFLECT_BASE(Text)
		REFLECT_END_BASES
	REFLECT_END

	template<class T>
	void TraitSerializer(const Trait& trait, Verb& verb, const DataID& type) {
		if (!type.Is<T>())
			return;

		GASM result;
		if (!trait.GetTraitMeta())
			result += TraitID::DefaultToken;
		else
			result += trait.GetTraitMeta()->GetToken();
		result += GASM::OpenScope;
		result += pcSerialize<T, true, Block>(trait);
		result += GASM::CloseScope;
		verb << T{ pcMove(result) };
	}

	template<class T>
	void CharSerializer(const T& letter, Verb& verb, const DataID& type) {
		if (type.Is<Debug>() || type.Is<GASM>()) {
			GASM result;
			result += GASM::OpenCharacter;
			result += letter;
			result += GASM::CloseCharacter;
			if (type.Is<Debug>())
				verb << Debug{ pcMove(result) };
			else
				verb << pcMove(result);
		}
		else if (type.Is<Text>()) {
			Text result;
			result += letter;
			verb << pcMove(result);
		}
	}


	/// Initialize all meta data required by the GASM pipeline						
	///	@return true on success																
	bool pcInitGASM() {
		static bool guard = false;
		if (guard)
			return true;
		guard = true;

		Logger::pcInitLogger();
		Memory::pcInitMemory();

		MetaData::REGISTER<GASM, Verb, ChargedVerbID, Construct>();

		{
			auto meta = MetaData::LOCK<DataID>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(DataID, GASM));
		}

		{
			auto meta = MetaData::LOCK<TraitID>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(TraitID, GASM));
		}

		{
			auto meta = MetaData::LOCK<ConstID>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(ConstID, GASM));
		}

		{
			auto meta = MetaData::LOCK<VerbID>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(VerbID, GASM));
		}

		{
			auto meta = MetaData::LOCK<MetaData>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(MetaData, GASM));
		}

		{
			auto meta = MetaData::LOCK<MetaTrait>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(MetaTrait, GASM));
		}

		{
			auto meta = MetaData::LOCK<MetaConst>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(MetaConst, GASM));
		}

		{
			auto meta = MetaData::LOCK<MetaVerb>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(MetaVerb, GASM));
		}

		{
			auto meta = MetaData::LOCK<bool>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(bool, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(bool, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pci8>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pci8, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pci8, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pci16>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pci16, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pci16, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pci32>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pci32, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pci32, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pci64>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pci64, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pci64, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcu8>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcu8, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcu8, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcu16>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcu16, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcu16, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcu32>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcu32, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcu32, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcu64>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcu64, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcu64, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcr32>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcr32, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcr32, NumberTypes));
		}

		{
			auto meta = MetaData::LOCK<pcr64>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(pcr64, GASM));
			meta->AddAbility(REFLECT_CONVERSION_LIST_EXT(pcr64, NumberTypes));
		}

		{
			const auto serializer = [](void* context, Verb& verb) {
				auto& trait = *static_cast<Trait*>(context);
				EitherDoThis
					verb.GetArgument().ForEach([&trait, &verb](const DataID& type) {
						TraitSerializer<Debug>(trait, verb, type);
						TraitSerializer<GASM>(trait, verb, type);
					})
				OrThis
					verb.GetArgument().ForEach([&trait, &verb](const MetaData* type) {
						TraitSerializer<Debug>(trait, verb, type->GetID());
						TraitSerializer<GASM>(trait, verb, type->GetID());
					});
			};

			auto meta = MetaData::LOCK<Trait>();
			meta->AddAbility(MEMBER_VERB_EXT(Interpret, serializer));
		}

		{
			auto meta = MetaData::LOCK<Index>();
			meta->AddAbility(REFLECT_CONVERSIONS_EXT(Index, GASM));
		}

		{
			const auto serializer = [](void* context, Verb& verb) {
				auto& letter = *static_cast<char8*>(context);
				EitherDoThis
					verb.GetArgument().ForEach([&letter, &verb](const DataID& type) {
						CharSerializer(letter, verb, type);
					})
				OrThis
					verb.GetArgument().ForEach([&letter, &verb](const MetaData* type) {
						CharSerializer(letter, verb, type->GetID());
					});
			};

			auto meta = MetaData::LOCK<char8>();
			meta->AddAbility(MEMBER_VERB_EXT(Interpret, serializer));
		}

		{
			const auto serializer = [](void* context, Verb& verb) {
				auto& letter = *static_cast<charw*>(context);
				EitherDoThis
					verb.GetArgument().ForEach([&letter, &verb](const DataID& type) {
						CharSerializer(letter, verb, type);
					})
				OrThis
					verb.GetArgument().ForEach([&letter, &verb](const MetaData* type) {
						CharSerializer(letter, verb, type->GetID());
					});
			};

			auto meta = MetaData::LOCK<charw>();
			meta->AddAbility(MEMBER_VERB_EXT(Interpret, serializer));
		}

		{
			const auto concatenator = [](void* context, Verb& verb) {
				auto text = static_cast<Text*>(context);
				verb.GetArgument().ForEachDeep([&text](const Block& a) {
					a.ForEach([&text](const Text& t) {
						*text += t;
					});
				});
				verb << text;
			};

			auto meta = MetaData::LOCK<Text>();
			meta->AddAbility(MEMBER_VERB_EXT(Catenate, concatenator));
		}

		pcLogVerbose << ccGreen << "PCFW::GASM initialized";
		return true;
	}

} // namespace PCFW::Flow


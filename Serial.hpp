#pragma once
#include "GASM.hpp"

namespace PCFW::Flow
{

	template<class TO, bool HEADER = true, class FROM>
	NOD() TO pcSerialize(const FROM&);

	template<class FROM>
	NOD() Any pcDeserialize(const FROM&);

	namespace Detail
	{

		///																							
		///	General GASM/Debug serializer tools											
		///																							
		NOD() bool NeedsScope(const Block& block) noexcept;
		NOD() GASM Separator(bool isOr);

		template<class TO>
		NOD() pcptr SerializeBlockToText(const Block&, TO&);

		template<class TO>
		void SerializeStateToText(const Block&, TO&);

		template<class META, class TO>
		void SerializeMetaToText(const Block&, TO&, const LinkedMember*);

		template<class TO>
		void SerializeMembersToText(const Block&, TO&);

		///																							
		///	General binary serializer tools												
		///																							
		#pragma pack(push, 1)
		struct EMPTY_BASE() Header : POD {
			Header() noexcept;

			enum {
				Default = 0,
				/// Mark the data inside to be big-endian									
				BigEndian = 1,
				/// Export all types as strings instead of hashes, which makes		
				/// files bigger, but keeps them compatible for all architectures	
				Portable = 2
			};

			bool operator == (const Header&) const noexcept;
			bool operator != (const Header&) const noexcept;

			pcu8 mAtomSize;
			pcu8 mFlags;
			pcu16 mUnused;
		};
		#pragma pack(pop)

		template<bool HEADER>
		void SerializeBlockToBinary(const Block&, Bytes&);

		using Loader = TFunctor<void(Bytes&, pcptr)>;

		NOD() pcptr DeserializeAtomFromBinary(const Bytes&, pcptr&, pcptr, const Header&, const Loader&);

		template<bool HEADER>
		NOD() pcptr DeserializeBlockFromBinary(const Bytes&, Block&, pcptr, const Header&, const Loader&);

		template<class META>
		NOD() pcptr DeserializeMetaFromBinary(const Bytes&, META const*&, pcptr, const Header&, const Loader&);

		template<class INTERNAL>
		NOD() pcptr DeserializeInternalFromBinary(const Bytes&, INTERNAL&, pcptr, const Header&, const Loader&);

	} // namespace Detail
} // namespace PCFW::PCGASM

#include "Serial.inl"
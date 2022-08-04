#pragma once
#include "Code.hpp"

namespace Langulus::Flow
{

	template<CT::Block TO, bool HEADER = true, CT::Dense FROM>
	NOD() TO Serialize(const FROM&);
	template<CT::Block TO, bool HEADER = true, CT::Sparse FROM>
	NOD() TO Serialize(FROM);
	template<CT::Block FROM>
	NOD() Any Deserialize(const FROM&);


	namespace Detail
	{

		///																							
		///	General Code/Debug serializer tools											
		///																							
		NOD() bool NeedsScope(const Block& block) noexcept;
		NOD() Code Separator(bool isOr);

		template<CT::Text TO>
		NOD() Count SerializeBlock(const Block&, TO&);

		template<class META, CT::Text TO>
		void SerializeMeta(const Block&, TO&, const RTTI::Member*);

		template<CT::Text TO>
		void SerializeMembers(const Block&, TO&);


		///																							
		///	General binary serializer tools												
		///																							
		#pragma pack(push, 1)
		struct Header {
			::std::uint8_t mAtomSize;
			::std::uint8_t mFlags;
			::std::uint16_t mUnused;

		public:
			Header() noexcept;

			enum {
				Default = 0,
				/// Mark the data inside to be big-endian									
				BigEndian = 1,
			};

			bool operator == (const Header&) const noexcept;
		};
		#pragma pack(pop)

		template<bool HEADER>
		void SerializeBlock(const Block&, Bytes&);

		using Loader = TFunctor<void(Bytes&, Size)>;

		NOD() Size DeserializeAtom(const Bytes&, Offset&, Offset, const Header&, const Loader&);

		template<bool HEADER>
		NOD() Size DeserializeBlock(const Bytes&, Block&, Offset, const Header&, const Loader&);

		template<class META>
		NOD() Size DeserializeMeta(const Bytes&, META const*&, Offset, const Header&, const Loader&);

	} // namespace Detail
} // namespace Langulus::Flow

#include "Serial.inl"
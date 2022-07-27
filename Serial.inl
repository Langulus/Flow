#pragma once
#include "Serial.hpp"
#include "verbs/Do.inl"

#define LGLS_VERBOSE_SERIALIZATION(a)

namespace Langulus::Flow
{

	/// Conversion routine, that is specialized for serialization					
	///	@tparam TO - the type we're serializing to									
	///	@tparam HEADER - ignored when serializing to text formats				
	///		-	when serializing to binary formats: true if you want to write	
	///			a portability header for the data (useful for serializing		
	///			standalone data). The serializer uses this internally when		
	///			nesting, to reduce redundant writes										
	///	@tparam FROM - the type of the item (deducible)								
	///	@param item - the item to serialize												
	///	@return the serialized item														
	template<CT::Block TO, bool HEADER, CT::Sparse FROM>
	TO Serialize(FROM item) {
		if (!item)
			return "<null>";
		return Serialize<TO, HEADER>(*item);
	}

	/// Conversion routine, that is specialized for serialization					
	///	@tparam TO - the type we're serializing to									
	///	@tparam HEADER - ignored when serializing to text formats				
	///		-	when serializing to binary formats: true if you want to write	
	///			a portability header for the data (useful for serializing		
	///			standalone data). The serializer uses this internally when		
	///			nesting, to reduce redundant writes										
	///	@tparam FROM - the type of the item (deducible)								
	///	@param item - the item to serialize												
	///	@return the serialized item														
	template<CT::Block TO, bool HEADER, CT::Dense FROM>
	TO Serialize(const FROM& item) {
		if constexpr (CT::SameAsOneOf<TO, Debug, Text>) {
			///	DEBUG SERIALIZER														
			// Debug serializer doesn't have any restrictions. It is			
			// useful for omitting redundant or irrelevant data, and is		
			// a one-way process. Extensively used by the logger				
			const auto block = Block::From(item);

			try {
				// Attempt converting to debug via reflected converters		
				Debug result;
				(void)Detail::SerializeBlock(block, result);
				return result;
			}
			catch (const Except::Convert&) {}

			// Direct serialization to Debug failed if this is reached, but
			// we can still attempt to serialize to something readable		

			try {
				// Debug serializer also employs the Code serializer as		
				// an alternative stringifier - it might include a lot		
				// of redundant and irrelevant data, but better something	
				// than nothing...														
				Code result;
				(void)Detail::SerializeBlock(block, result);
				return result;
			}
			catch (const Except::Convert&) {}
		}
		else if constexpr (CT::Same<TO, Code>) {
			///	CODE SERIALIZER														
			// Code serializer is strict to allow for deserialization		
			const auto block = Block::From(item);

			try {
				Code result;
				(void)Detail::SerializeBlock(block, result);
				return result;
			}
			catch (const Except::Convert&) {}
		}
		else if constexpr (CT::Same<TO, Bytes>) {
			///	BINARY SERIALIZER														
			// Byte serializer is strict to allow for deserialization		
			const auto block = Block::From(item);

			try {
				Bytes result;
				(void)Detail::SerializeBlock<HEADER>(block, result);
				return result;
			}
			catch (const Except::Convert&) {}
		}
		else LANGULUS_ASSERT("Serializer not implemented");

		// If this is reached, then we weren't able to serialize the item	
		// to the desired type															
		throw Except::Convert("Can't serialize");
	}

	/// Deserialize from Code/Bytes															
	///	@tparam FROM - the type we're deserializing from (deducible)			
	///	@param item - the item to deserialize											
	///	@return the deserialized contents inside a container						
	template<CT::Block FROM>
	Any Deserialize(const FROM& item) {
		if constexpr (CT::Same<FROM, Debug>) {
			LANGULUS_ASSERT(
				"You can't deserialize debug containers "
				" - debug serialization is a one-way process");
		}
		else if constexpr (CT::Same<FROM, Code>) {
			return item.Parse();
		}
		else if constexpr (CT::Same<FROM, Bytes>) {
			Detail::Header header;
			Any result;
			(void)Detail::DeserializeBlock<true>(item, result, 0, header, {});
			return result;
		}
		else LANGULUS_ASSERT("Deserializer not implemented");
	}

	/// Convert a byte to hexadecimal string, and append it to text container	
	///	@param from - the byte to convert to hexadecimal							
	///	@param to - the container to append to											
	inline void ToHex(const Byte& from, Text& to) {
		fmt::format_to_n(to.Extend(2).GetRaw(), 2, "{:x}", from);
	}

	/// Check if a memory block needs a Code scope decorated							
	///	@param block - the memory block to check										
	///	@return true if a scope is required around the block						
	inline bool Detail::NeedsScope(const Block& block) noexcept {
		return block.GetCount() > 1
			|| block.IsMissing()
			|| block.IsPhased()
			|| block.IsEmpty();
	}

	/// Add a separator																			
	///	@param isOr - OR separator or not												
	///	@return the text equivalent of the separator									
	inline Code Detail::Separator(bool isOr) {
		return isOr ? Code::Or : Code::And;
	}
	
	/// Serialize a data state																	
	///	@param from - the block to scan for state										
	///	@param to - the serialized state													
	template<CT::Text TO>
	void Detail::SerializeState(const Block& from, TO& to) {
		if (from.IsPast())
			to += Code {Code::Past};
		else if (from.IsFuture())
			to += Code {Code::Future};

		if (from.IsMissing())
			to += Code {Code::Missing};
	}

	/// Serialize any block to any string format											
	///	@param from - the block to serialize											
	///	@param to - [out] the serialized block goes here							
	///	@return the number of written characters										
	template<CT::Text TO>
	Count Detail::SerializeBlock(const Block& from, TO& to) {
		const auto initial = to.GetCount();
		SerializeState(from, to);

		const bool scoped = NeedsScope(from);
		if (scoped)
			to += Code {Code::OpenScope};

		// Serialize text																	
		if (from.IsUntyped() || from.IsEmpty()) {
			// Do nothing																	
		}
		else if (from.IsDeep()) {
			// Nested serialization, wrap it in content scope					
			for (Offset i = 0; i < from.GetCount(); ++i) {
				(void) SerializeBlock(from.As<Block>(i), to);
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<bool>()) {
			// Contained type is boolean												
			for (Offset i = 0; i < from.GetCount(); ++i) {
				to += from.As<bool>(i) ? "yes" : "no";
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<Code>()) {
			// Contained type is code, wrap it in code scope					
			for (Offset i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<Code>(i);
				to += Code {Code::OpenCode};
				to += text;
				to += Code {Code::CloseCode};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<Text>()) {
			// Contained type is text, wrap it in string scope					
			for (Offset i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<Text>(i);
				to += Code {Code::OpenString};
				to += text;
				to += Code {Code::CloseString};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<Letter>()) {
			// Contained type is characters, wrap it in char scope			
			for (Offset i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<Letter>(i);
				to += Code {Code::OpenCharacter};
				to += Token {&text, 1};
				to += Code {Code::CloseCharacter};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<RTTI::Meta>()) {
			// Contained type is meta definitions, write the token			
			for (Offset i = 0; i < from.GetCount(); ++i) {
				auto& meta = from.As<RTTI::Meta>(i);
				to += meta.mToken;
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.CastsTo<Byte>()) {
			// Contained type is raw bytes, wrap it in byte scope				
			auto raw_bytes = from.GetRaw();
			if (!from.IsOr()) {
				to += Code {Code::OpenByte};
				for (Offset i = 0; i < from.GetCount(); ++i)
					ToHex(raw_bytes[i], to);
				to += Code {Code::CloseByte};
			}
			else for (Offset i = 0; i < from.GetCount(); ++i) {
				to += Code {Code::OpenByte};
				ToHex(raw_bytes[i], to);
				to += Code {Code::CloseByte};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else {
			// Serialize all elements one by one using RTTI						
			for (Offset i = 0; i < from.GetCount(); ++i) {
				Verbs::Interpret interpreter({}, MetaData::Of<TO>());
				auto element = from.GetElementResolved(i);

				// Element is already resolved, so don't resolve it			
				if (DispatchFlat<false>(element, interpreter)) {
					if (!interpreter.GetOutput().template Is<TO>())
						Throw<Except::Convert>(
							"Can't serialize resolved element to text");

					to += interpreter.GetOutput().template Get<TO>();
				}
				else Throw<Except::Convert>(
					"Can't serialize resolved element to text");

				// Separate each element												
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}

		// Close scope																		
		if (scoped)
			to += Code {Code::CloseScope};
		return to.GetCount() - initial;
	}

	/// A snippet for serializing a reference to a meta object						
	///	@param from - the member block to serialize									
	///	@param to - [out] the serialized data											
	///	@param member - reflection data about the member							
	template<class META, CT::Text TO>
	void Detail::SerializeMeta(const Block& from, TO& to, const RTTI::Member* member) {
		static_assert(CT::DerivedFrom<META, RTTI::Meta>,
			"META has to be an AMeta derivative");

		auto meta = member->As<META>(from.GetRaw());
		if (meta)	to += meta->GetToken();
		else			to += Decay<META>::DefaultToken;
	}

	/// Serialize all reflected data members in all bases								
	///	@param from - the member block to serialize									
	///	@param to - the serialized data													
	template<CT::Text TO>
	void Detail::SerializeMembers(const Block& from, TO& to) {
		if (from.Is<Block>() || from.Is<Any>()) {
			SerializeBlock(from.Get<Block>(), to);
			return;
		}

		// Append a separator?															
		bool separate = false;

		// First we serialize all bases' members									
		for (auto& base : from.GetType()->mBases) {
			if (base.mType->mSize > 0) {
				if (separate) {
					to += Code {Code::And};
					separate = false;
				}

				const auto initial = to.GetCount();
				SerializeMembers(from.GetBaseMemory(base.mType, base), to);
				if (initial < to.GetCount())
					separate = true;
			}
		}

		// Iterate members for each object											
		for (auto& member : from.GetType()->mMembers) {
			if (separate)
				to += Code {Code::And};

			if (member.mType->Is<DMeta>())
				SerializeMeta<DMeta>(from, to, &member);
			else if (member.mType->Is<TMeta>())
				SerializeMeta<TMeta>(from, to, &member);
			else if (member.mType->Is<VMeta>())
				SerializeMeta<VMeta>(from, to, &member);
			else
				SerializeBlock(from.GetMember(member), to);

			separate = true;
		}
	}

	/// Default header constructor															
	inline Detail::Header::Header() noexcept {
		mAtomSize = sizeof(Size);

		// First bit of the flag means the file was written by a big		
		// endian machine																	
		mFlags = Default;
		if constexpr (BigEndianMachine)
			mFlags = BigEndian;
		mUnused = 0;
	}

	inline bool Detail::Header::operator == (const Header& rhs) const noexcept {
		return mAtomSize == rhs.mAtomSize && mFlags == rhs.mFlags;
	}

	/// Inner binary serialization routine													
	///	@tparam HEADER - true if you want to write a portability header		
	///						  (useful for serializing standalone data)				
	///	@param source - the block to serialize											
	///	@param result - [out] the resulting byte array								
	///	@return the number of written bytes												
	template<bool HEADER>
	void Detail::SerializeBlock(const Block& source, Bytes& result) {
		if constexpr (HEADER) {
			result += source.GetCount();
			result += source.GetUnconstrainedState();
			result += source.GetType();
		}

		if (source.IsEmpty() || source.IsUntyped())
			return;

		const bool resolvable = source.IsResolvable();
		if (!resolvable) {
			if (source.IsPOD()) {
				// If data is POD, optimize by directly memcpying it			
				const auto denseStride = source.GetStride();
				const auto byteCount = denseStride * source.GetCount();
				result.Allocate(result.GetCount() + byteCount);
				if (source.IsSparse()) {
					// ... pointer by pointer if sparse								
					auto p = source.GetRawSparse();
					const auto pEnd = p + source.GetCount();
					while (p != pEnd) {
						result += Bytes {p->mPointer, denseStride};
						++p;
					}
				}
				else {
					// ... at once if dense												
					result += Bytes {source.GetRaw(), byteCount};
				}

				return;
			}
			else if (source.IsDeep()) {
				// If data is deep, nest-serialize each sub-block				
				source.ForEach([&result](const Block& block) {
					SerializeBlock<true>(block, result);
				});

				return;
			}
			else if (source.CastsTo<RTTI::Meta>()) {
				// Serialize meta															
				source.ForEach(
					[&result](DMeta meta) { result += meta->mToken; },
					[&result](VMeta meta) { result += meta->mToken; },
					[&result](TMeta meta) { result += meta->mToken; }
				);

				return;
			}
		}
		
		if (source.IsDefaultable()) {
			// Type is statically producible, and has default constructor,	
			// therefore we can serialize it by serializing each reflected	
			// base and member															
			for (Count i = 0; i < source.GetCount(); ++i) {
				auto element = source.GetElementResolved(i);
				if (resolvable)
					result += element.GetType();

				// Serialize all reflected bases										
				for (auto& base : element.GetType()->mBases) {
					// Imposed bases are never serialized							
					if (base.mImposed)
						continue;

					const auto baseBlock = element.GetBaseMemory(base);
					SerializeBlock<false>(baseBlock, result);
				}

				// Serialize all reflected members									
				for (auto& member : element.GetType()->mMembers) {
					const auto memberBlock = element.GetMember(member);
					SerializeBlock<false>(memberBlock, result);
				}
			}

			return;
		}

		// Failure if reached															
		Throw<Except::Convert>("Can't binary serialize");
	}

	namespace Detail
	{
		inline void RequestMoreBytes(const Bytes& source, Offset read, Size byteCount, const Loader& loader) {
			if (read >= source.GetCount() || source.GetCount() - read < byteCount) {
				if (!loader)
					Throw<Except::Access>("Deserializer has no loader");
				loader(const_cast<Bytes&>(source), byteCount - (source.GetCount() - read));
			}
		}
	}

	/// Read an atom-sized unsigned integer, based on the provided header		
	///	@param source - the serialized byte source									
	///	@param result - [out] the resulting deserialized number					
	///	@param read - offset to apply to serialized byte array					
	///	@param header - environment header												
	///	@param loader - loader for streaming											
	///	@return the number of read bytes from byte container						
	inline Size Detail::DeserializeAtom(const Bytes& source, Offset& result, Offset read, const Header& header, const Loader& loader) {
		if (header.mAtomSize == 4) {
			// We're deserializing data, that was serialized on a 32-bit	
			// architecture																
			uint32_t count4 = 0;
			RequestMoreBytes(source, read, 4, loader);
			::std::memcpy(&count4, source.At(read), 4);
			read += 4;
			result = static_cast<Offset>(count4);
		}
		else if (header.mAtomSize == 8) {
			// We're deserializing data, that was serialized on a 64-bit	
			// architecture																
			uint64_t count8 = 0;
			RequestMoreBytes(source, read, 8, loader);
			::std::memcpy(&count8, source.At(read), 8);
			read += 8;
			if (count8 > std::numeric_limits<Offset>::max()) {
				Throw<Except::Convert>(
					"Deserialized atom contains a value "
					"too powerful for your architecture");
			}
			result = static_cast<Offset>(count8);
		}
		else {
			Throw<Except::Convert>(
				"An unknown atomic size was deserialized "
				"from source - is the source corrupted?");
		}

		return read;
	}

	/// Inner deserialization routine from binary										
	///	@tparam HEADER - true if you want to read a portability header			
	///						  (useful for deserializing standalone data)				
	///	@param source - bytes to deserialize											
	///	@param result - [out] the resulting deserialized data						
	///	@param readOffset - offset to apply to serialized byte array			
	///	@param header - environment header												
	///	@param loader - loader for streaming											
	///	@return the number of read/peek bytes from byte container				
	template<bool HEADER>
	Size Detail::DeserializeBlock(const Bytes& source, Block& result, Offset readOffset, const Header& header, const Loader& loader) {
		Count deserializedCount = 0;
		auto read = readOffset;

		if constexpr (HEADER) {
			// Read the header - means we have unpredictable data				
			// The header contains instructions on how data was serialized	
			read = DeserializeAtom(
				source, deserializedCount, read, header, loader);

			// First read the serialized data state								
			DataState deserializedState {};
			RequestMoreBytes(source, read, sizeof(DataState), loader);
			::std::memcpy(&deserializedState, source.At(read), sizeof(DataState));
			read += sizeof(DataState);
			result.AddState(deserializedState);

			// Finally, read type														
			DMeta deserializedType;
			read = DeserializeMeta(
				source, deserializedType, read, header, loader);

			if (!deserializedType)
				return read;

			result.SetType<false>(deserializedType);
		}
		else {
			// Don't read header - we have a predictable single element,	
			// like a member, a base, or a cast operator sequence				
			// In this case, result should already be allocated and known	
			if (result.IsUntyped() || result.IsEmpty())
				Throw<Except::Convert>("Bad resulting block");

			deserializedCount = result.GetCount();
		}

		if (deserializedCount == 0)
			return read;

		// Fill memory																		
		const bool resolvable = result.IsResolvable();
		if (!resolvable) {
			if (result.IsPOD()) {
				if (result.IsSparse())
					TODO();

				// If data is POD, optimize by directly memcpying it			
				if constexpr (HEADER) {
					result.Allocate<false>(deserializedCount);
					result.mCount += deserializedCount;
				}

				const auto byteSize = result.GetByteSize();
				RequestMoreBytes(source, read, byteSize, loader);
				::std::memcpy(result.GetRaw(), source.At(read), byteSize);
				read += byteSize;
				return read;
			}
			else if (result.IsDeep()) {
				// If data is deep, nest each sub-block							
				if constexpr (HEADER)
					result.Allocate<true>(deserializedCount);

				result.ForEach<false>([&](Block& block) {
					read = DeserializeBlock<true>(
						source, block, read, header, loader);
				});

				return read;
			}
			else if (result.CastsTo<RTTI::Meta>()) {
				SAFETY(if (!result.IsSparse())
					Throw<Except::Convert>("Meta block is not sparse"));

				// Deserialize meta definitions										
				if constexpr (HEADER) {
					result.Allocate<false>(deserializedCount);
					result.mCount += deserializedCount;
				}

				auto p = result.GetRawSparse();
				const auto pEnd = p + result.GetCount();
				if (result.Is<MetaData>()) {
					while (p != pEnd) {
						DMeta ptr;
						read = DeserializeMeta(source, ptr, read, header, loader);
						p->mPointer = ptr;
						++p;
					}
				}
				else if (result.Is<MetaTrait>()) {
					while (p != pEnd) {
						TMeta ptr;
						read = DeserializeMeta(source, ptr, read, header, loader);
						p->mPointer = ptr;
						++p;
					}
				}
				else if (result.Is<MetaVerb>()) {
					while (p != pEnd) {
						VMeta ptr;
						read = DeserializeMeta(source, ptr, read, header, loader);
						p->mPointer = ptr;
						++p;
					}
				}

				return read;
			}
		}

		if (result.IsDefaultable) {
			if (result.IsSparse())
				TODO();

			// Type is statically producible, and has default constructor,	
			// therefore we can deserialize it by making a default copy		
			// and then filling in the reflected members and bases			
			for (Count i = 0; i < deserializedCount; ++i) {
				auto resolvedType = result.GetType();
				if (resolvable)
					read = DeserializeMeta(source, resolvedType, read, header, loader);

				Any element;
				if constexpr (HEADER) {
					// Create default copy only if not predictable				
					element = Any::From(resolvedType);
					element.Allocate<true>(1);
				}
				else {
					// We don't make a default copy if already predictable	
					// It is your responsibility to preallocate and define	
					// the result container												
					element = result.GetElementDense(i);
				}

				// Deserialize all reflected bases									
				for (auto& base : element.GetType()->mBases) {
					if (base.mImposed)
						continue;

					auto baseBlock = element.GetBaseMemory(base);
					read = DeserializeBlock<false>(source, baseBlock, read, header, loader);
				}

				// Deserialize all reflected members								
				for (auto& member : element.GetType()->mMembers) {
					auto memberBlock = element.GetMember(member);
					read = DeserializeBlock<false>(source, memberBlock, read, header, loader);
				}

				if constexpr (HEADER)
					result.InsertBlock(element);
			}

			return read;
		}

		// Failure if reached															
		Throw<Except::Convert>("Can't binary-deserialize");
	}

	/// A snippet for conveniently deserializing a meta from binary				
	///	@tparam META - type of meta we're deserializing (deducible)				
	///	@param source - the bytes to deserialize										
	///	@param result - [out] the deserialized meta goes here						
	///	@param read - byte offset inside 'from'										
	///	@param header - environment header												
	///	@param loader - loader for streaming											
	///	@return number of read bytes														
	template<class META>
	Size Detail::DeserializeMeta(const Bytes& source, META const*& result, Offset read, const Header& header, const Loader& loader) {
		Count count = 0;
		read = DeserializeAtom(source, count, read, header, loader);
		if (count) {
			RequestMoreBytes(source, read, count, loader);
			const Token token {source.As<char*>(read), count};
			if constexpr (CT::Same<META, MetaData>)
				result = RTTI::Database.GetMetaData(token);
			else if constexpr (CT::Same<META, MetaVerb>)
				result = RTTI::Database.GetMetaVerb(token);
			else if constexpr (CT::Same<META, MetaTrait>)
				result = RTTI::Database.GetMetaTrait(token);
			else
				LANGULUS_ASSERT("Unsupported meta deserialization");

			return read + count;
		}

		result = {};
		return read;
	}

} // namespace Langulus::Flow


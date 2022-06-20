#define LGLS_VERBOSE_SERIALIZATION(a)

namespace PCFW::Flow
{

	/// Check if a memory block needs a GASM scope decorated							
	///	@param block - the memory block to check										
	///	@return true if a scope is required around the block						
	inline bool Detail::NeedsScope(const Block& block) noexcept {
		return block.GetCount() > 1 || block.IsMissing() || block.IsPolar() || block.IsEmpty();
	}

	/// Add a separator																			
	///	@param isOr - OR separator or not												
	///	@return the text equivalent of the separator									
	inline GASM Detail::Separator(bool isOr) {
		return isOr
			? GASM {GASM::OrSeparator}
			: GASM {GASM::AndSeparator};
	}
	
	/// Invoke reflected converters to Text or GASM for FROM							
	///	@tparam TO - the type we're serializing to (either Debug or GASM)		
	///	@tparam HEADER - ignored when serializing to text formats				
	///						- when serializing to binary formats: true if you		
	///						  want to write a portability header for the data		
	///						  (useful for serializing standalone data)				
	///	@tparam FROM - the type of the item (deducible)								
	///	@param item - the item to serialize												
	///	@return the serialized item														
	template<class TO, bool HEADER, class FROM>
	TO pcSerialize(const FROM& item) {
		if constexpr (Dense<FROM>) {
			if constexpr (Same<FROM, TO>) {
				// Avoid infinite regressions											
				return item;
			}
			else if constexpr (!pcIsDeep<FROM> && Convertible<FROM, TO>) {
				// This constexpr branch is not only an optimization, but	
				// also acts as a stringifier for more fundamental types		
				TO result;
				Memory::TConverter<FROM, TO>::Convert(item, result);
				return result;
			}
			else if constexpr (Same<TO, Debug>){
				/// DEBUG SERIALIZER														
				// Debug serializers don't have any restrictions, and are	
				// useful for omitting redundant or irrelevant data			
				// Debug serialization is one-way process							
				const auto block = Block::From(item);

				try {
					// Attempt converting to debug via reflected converters	
					Debug result;
					(void)Detail::SerializeBlockToText(block, result);
					return result;
				}
				catch (const Except::BadSerialization&) {}

				try {
					// Debug serializer also employs the GASM serializer as	
					// an alternative stringifier - it might include a lot	
					// of redundant and irrelevant data, but better				
					// something than nothing...										
					GASM result;
					(void)Detail::SerializeBlockToText(block, result);
					return result;
				}
				catch (const Except::BadSerialization&) {}

				// If this is reached, then we can't serialize the item		
				throw Except::BadSerialization(pcLogError
					<< "An element of type " << DataID::Of<FROM>
					<< " couldn't be serialized to " << DataID::Of<TO>
				);
			}
			else if constexpr (Same<TO, GASM>) {
				/// GASM SERIALIZER														
				// GASM serializer is strict to allow for deserialization	
				const auto block = Block::From(item);

				try {
					GASM result;
					(void)Detail::SerializeBlockToText(block, result);
					return result;
				}
				catch (const Except::BadSerialization&) {}

				// If this is reached, then we can't serialize the item		
				throw Except::BadSerialization(pcLogError
					<< "An element of type " << DataID::Of<FROM>
					<< " couldn't be serialized to " << DataID::Of<TO>
				);
			}
			else if constexpr (Same<TO, Bytes>) {
				/// BINARY SERIALIZER													
				// Byte serializer is strict to allow for deserialization	
				const auto block = Block::From(item);

				try {
					Bytes result;
					(void)Detail::SerializeBlockToBinary<HEADER>(block, result);
					return result;
				}
				catch (const Except::BadSerialization&) {}

				// If this is reached, then we can't serialize the item		
				throw Except::BadSerialization(pcLogError
					<< "An element of type " << DataID::Of<FROM>
					<< " couldn't be serialized to " << DataID::Of<TO>
				);
			}
			else LANGULUS_ASSERT("Serializer not implemented");
		}
		else {
			// A known pointer - dereference it and nest							
			if (!item)
				return "<null>";
			return pcSerialize<TO, HEADER, pcDecay<FROM>>(*item);
		}
	}

	/// Deserialize from GASM/Bytes															
	///	@tparam FROM - the type we're deserializing from (deducible)			
	///	@param item - the item to deserialize											
	///	@return the deserialized contents inside a container						
	template<class FROM>
	Any pcDeserialize(const FROM& item) {
		if constexpr (Same<FROM, Debug>) {
			LANGULUS_ASSERT("You can't deserialize debug containers "
				" - debug serialization is a one-way process");
		}
		else if constexpr (Same<FROM, GASM>)
			return item.Parse();
		else if constexpr (Same<FROM, Bytes>) {
			Detail::Header header;
			Any result;
			(void)Detail::DeserializeBlockFromBinary<true>(item, result, 0, header, {});
			return result;
		}
		else LANGULUS_ASSERT("Deserializer not implemented");
	}

	/// Serialize a data state																	
	///	@param from - the block to scan for state										
	///	@param to - the serialized state													
	template<class TO>
	void Detail::SerializeStateToText(const Block& from, TO& to) {
		static_assert(IsText<TO>, "TO has to be a Text derivative");
		if (from.IsLeft())
			to += GASM {GASM::PolarizeLeft};
		else if (from.IsRight())
			to += GASM {GASM::PolarizeRight};

		if (from.IsMissing())
			to += GASM {GASM::Missing};
	}

	/// Serialize any block to any string format											
	///	@param from - the block to serialize											
	///	@param to - [out] the serialized block goes here							
	///	@param noscope360 - avoid serializing scope and state						
	///	@return the number of written characters										
	template<class TO>
	pcptr Detail::SerializeBlockToText(const Block& from, TO& to) {
		static_assert(IsText<TO>, "TO has to be a Text derivative");
		const auto initial = to.GetCount();
		SerializeStateToText(from, to);

		const bool scoped = NeedsScope(from);
		if (scoped)
			to += GASM {GASM::OpenScope};

		// Serialize text																	
		if (from.IsUntyped() || from.IsEmpty()) {
			// Do nothing																	
		}
		else if (from.IsDeep()) {
			// Nested serialization, wrap it in content scope					
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				(void) SerializeBlockToText(from.As<Block>(i), to);
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<bool>()) {
			// Contained type is boolean												
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				to += from.As<bool>(i) ? "yes" : "no";
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<GASM>()) {
			// Contained type is code, wrap it in code scope					
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<GASM>(i);
				to += GASM {GASM::OpenCode};
				to += text;
				to += GASM {GASM::CloseCode};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<Text>()) {
			// Contained type is text, wrap it in string scope					
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<Text>(i);
				to += GASM {GASM::OpenString};
				to += text;
				to += GASM {GASM::CloseString};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<char8>()) {
			// Contained type is characters, wrap it in char scope			
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<char8>(i);
				to += GASM {GASM::OpenCharacter};
				to += LiteralText {&text.mValue, 1};
				to += GASM {GASM::CloseCharacter};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<charw>()) {
			// Contained type is characters, wrap it in char scope			
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				auto& text = from.As<charw>(i);
				to += GASM {GASM::OpenCharacter};
				to += LiteralText {
					reinterpret_cast<const char*>(&text.mValue),
					sizeof(text.mValue)
				};
				to += GASM {GASM::CloseCharacter};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else if (from.InterpretsAs<pcbyte>()) {
			// Contained type is raw bytes, wrap it in byte scope				
			auto raw_bytes = from.GetBytes();
			if (!from.IsOr()) {
				to += GASM {GASM::OpenByte};
				for (pcptr i = 0; i < from.GetCount(); ++i)
					to += pcToHex(raw_bytes[i]);
				to += GASM {GASM::CloseByte};
			}
			else for (pcptr i = 0; i < from.GetCount(); ++i) {
				to += GASM {GASM::OpenByte};
				to += pcToHex(raw_bytes[i]);
				to += GASM {GASM::CloseByte};
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}
		else {
			// Serialize all elements one by one using RTTI						
			for (pcptr i = 0; i < from.GetCount(); ++i) {
				auto interpreter = Verb::From<Verbs::Interpret>({}, DataID::Of<TO>);
				auto element = from.GetElementResolved(i);
				if (Verb::DispatchFlat(element, interpreter, false)) {
					if (!interpreter.GetOutput().template Is<TO>()) {
						throw Except::BadSerialization(
							LGLS_VERBOSE_SERIALIZATION(pcLogError
							<< "Can't interpret " << element.GetToken() 
							<< "[" << i << "] to " << DataID::Of<TO>
							<< "; interpreted to " << interpreter.GetOutput().GetToken() 
							<< " instead"));
					}
					to += interpreter.GetOutput().template Get<TO>();
				}
				else throw Except::BadSerialization(
					LGLS_VERBOSE_SERIALIZATION(pcLogError
					<< "Can't interpret " << element.GetToken() 
					<< "[" << i << "] to " << DataID::Of<TO>));

				// Separate each element												
				if (i < from.GetCount() - 1)
					to += Separator(from.IsOr());
			}
		}

		// Close scope																		
		if (scoped)
			to += GASM {GASM::CloseScope};
		return to.GetCount() - initial;
	}

	/// A snippet for serializing a reference to a meta object						
	///	@param from - the member block to serialize									
	///	@param to - [out] the serialized data											
	///	@param member - reflection data about the member							
	template<class META, class TO>
	void Detail::SerializeMetaToText(const Block& from, TO& to, const LinkedMember* member) {
		static_assert(pcHasBase<META, AMeta>, "META has to be an AMeta derivative");
		static_assert(IsText<TO>, "TO has to be a Text derivative");
		auto meta = member->As<META>(from.GetRaw());
		if (meta)	to += meta->GetToken();
		else			to += pcDecay<META>::InternalType::DefaultToken;
	}

	/// Serialize all reflected data members in all bases								
	///	@param from - the member block to serialize									
	///	@param to - the serialized data													
	template<class TO>
	void Detail::SerializeMembersToText(const Block& from, TO& to) {
		static_assert(IsText<TO>, "TO has to be a Text derivative");
		if (from.Is<Block>() || from.Is<Any>()) {
			SerializeBlockToText(from.Get<Block>(), to);
			return;
		}

		// First we serialize all bases' members									
		bool separate = false;
		for (auto& base : from.GetMeta()->GetBaseList()) {
			if (base.mBase->GetStride() > 0) {
				if (separate) {
					to += GASM {GASM::AndSeparator};
					separate = false;
				}

				const auto initial = to.GetCount();
				SerializeMembersToText(from.GetBaseMemory(base.mBase, base), to);
				if (initial < to.GetCount())
					separate = true;
			}
		}

		// Iterate members for each object											
		for (auto& member : from.GetMeta()->GetMemberList()) {
			if (separate)
				to += GASM {GASM::AndSeparator};

			switch (member.mStaticMember.mType.GetHash().GetValue()) {
			case DataID::Switch<DMeta>():
				SerializeMetaToText<DMeta>(from, to, &member);
				break;
			case DataID::Switch<TMeta>():
				SerializeMetaToText<TMeta>(from, to, &member);
				break;
			case DataID::Switch<CMeta>():
				SerializeMetaToText<CMeta>(from, to, &member);
				break;
			case DataID::Switch<VMeta>():
				SerializeMetaToText<VMeta>(from, to, &member);
				break;
			default:
				SerializeBlockToText(from.GetMember(member), to);
			}

			separate = true;
		}
	}

	/// Default header constructor															
	inline Detail::Header::Header() noexcept {
		mAtomSize = sizeof(pcptr);

		// First bit of the flag means the file was written by a big		
		// endian machine																	
		mFlags = Default;
		if constexpr (BigEndianMachine)
			mFlags = BigEndian;
		#if LANGULUS_RTTI_IS(NAMED)
			// Second bit of the flag means the file was written by a		
			// machine using the NAMED internal IDs								
			mFlags |= Portable;
		#endif

		mUnused = 0;
	}

	inline bool Detail::Header::operator == (const Header& rhs) const noexcept {
		return mAtomSize == rhs.mAtomSize && mFlags == rhs.mFlags;
	}

	inline bool Detail::Header::operator != (const Header& rhs) const noexcept {
		return !(operator == (rhs));
	}


	/// Inner binary serialization routine													
	///	@tparam HEADER - true if you want to write a portability header		
	///						  (useful for serializing standalone data)				
	///	@param source - the block to serialize											
	///	@param result - [out] the resulting byte array								
	///	@return the number of written bytes												
	template<bool HEADER>
	void Detail::SerializeBlockToBinary(const Block& source, Bytes& result) {
		if constexpr (HEADER) {
			result += source.GetCount();
			result += source.GetUnconstrainedState();
			// Note that sparseness is retained										
			result += source.GetDataID();
		}

		if (source.IsEmpty() || source.IsUntyped())
			return;

		const bool resolvable = source.GetMeta()->IsResolvable();
		const auto denseStride = source.GetMeta()->GetCTTI().mSize;
		if (!resolvable) {
			if (source.GetMeta()->GetCTTI().mPOD) {
				// If data is POD, optimize by directly memcpying it			
				// This also serializes any InternalID if							
				// LANGULUS_RTTI_IS(HASHED)											
				const auto byteCount = denseStride * source.GetCount();
				result.Allocate(result.GetCount() + byteCount);
				if (source.GetMeta()->IsSparse()) {
					// ... pointer by pointer if sparse								
					auto pointers = source.GetPointers();
					for (pcptr i = 0; i < source.GetCount(); ++i)
						result += Bytes {pointers[i], denseStride};
				}
				else {
					// ... at once if dense												
					result += Bytes {source.GetRaw(), byteCount};
				}

				return;
			}
			else if (source.GetMeta()->GetCTTI().mSize == sizeof(Block) && source.GetMeta()->InterpretsAs<Block>()) {
				// If data is deep, nest each sub-block							
				source.ForEach([&result](const Block& block) {
					SerializeBlockToBinary<true>(block, result);
				});

				return;
			}
			else if (source.GetMeta()->InterpretsAs<InternalID>()) {
				// Serialize internal													
				EitherDoThis
				source.ForEach([&result](const DataID& id) {
					result += id;
				})
				OrThis
				source.ForEach([&result](const VerbID& id) {
					result += id;
				})
				OrThis
				source.ForEach([&result](const TraitID& id) {
					result += id;
				})
				OrThis
				source.ForEach([&result](const ConstID& id) {
					result += id;
				});
				return;
			}
			else if (source.GetMeta()->InterpretsAs<AMeta>()) {
				// Serialize meta															
				if (!source.GetMeta()->IsSparse())
					throw Except::BadSerialization(pcLogFuncError
						<< "Can't bin-serialize dense meta definition " << source.GetToken());

				EitherDoThis
				source.ForEach([&result](const MetaData& meta) {
					result += meta.GetID();
				})
				OrThis
				source.ForEach([&result](const MetaVerb& meta) {
					result += meta.GetID();
				})
				OrThis
				source.ForEach([&result](const MetaTrait& meta) {
					result += meta.GetID();
				})
				OrThis
				source.ForEach([&result](const MetaConst& meta) {
					result += meta.GetID();
				});
				return;
			}
		}
		
		if (source.GetMeta()->IsConstructible()) {
			// Type is statically producible, and has default constructor,	
			// therefore we can serialize it by serializing each reflected	
			// base and member															
			for (pcptr i = 0; i < source.GetCount(); ++i) {
				auto element = source.GetElementResolved(i);
				if (resolvable)
					result += element.GetDataID();

				//TODO attempt calling Bytes operator
				/*auto interpreter = Verb::From<Verbs::Interpret>({}, DataID::Of<Bytes>);
				auto element = from.GetElementResolved(i);
				if (Verb::DispatchFlat(element, interpreter, false)) {
					if (!interpreter.GetOutput().template Is<TO>())
						throw Except::BadSerialization();
					to += interpreter.GetOutput().template Get<TO>();
				}
				else throw Except::BadSerialization();*/

				for (auto& base : element.GetMeta()->GetBaseList()) {
					if (base.mStaticBase.mOr || base.mBase->IsAbstract())
						continue;
					const auto baseBlock = element.GetBaseMemory(base);
					SerializeBlockToBinary<false>(baseBlock, result);
				}

				for (auto& member : element.GetMeta()->GetMemberList()) {
					const auto memberBlock = element.GetMember(member);
					SerializeBlockToBinary<false>(memberBlock, result);
				}
			}

			return;
		}

		throw Except::BadSerialization(pcLogFuncError
			<< "Can't binary serialize " << source.GetCount()
			<< " elements of type " << source.GetToken());
	}

	namespace Detail
	{
		inline void RequestMoreBytes(const Bytes& source, pcptr read, pcptr byteCount, const Loader& loader) {
			if (read >= source.GetCount() || source.GetCount() - read < byteCount) {
				if (!loader)
					throw Except::BadAccess(pcLogError << "Deserializer has no loader");
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
	inline pcptr Detail::DeserializeAtomFromBinary(const Bytes& source, pcptr& result, pcptr read, const Header& header, const Loader& loader) {
		result = 0;
		if (header.mAtomSize == 4) {
			pcu32 count4 = 0;
			RequestMoreBytes(source, read, 4, loader);
			read += pcCopyMemory(source.At(read), &count4, 4);
			result = static_cast<pcptr>(count4);
		}
		else if (header.mAtomSize == 8) {
			pcu64 count8 = 0;
			RequestMoreBytes(source, read, 8, loader);
			read += pcCopyMemory(source.At(read), &count8, 8);
			if (count8 > std::numeric_limits<pcptr>::max()) {
				throw Except::BadSerialization(pcLogFuncError
					<< "Deserialized atom contains a value too powerful for your architecture");
			}
			result = static_cast<pcptr>(count8);
		}
		else TODO();
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
	pcptr Detail::DeserializeBlockFromBinary(const Bytes& source, Block& result, pcptr readOffset, const Header& header, const Loader& loader) {
		pcptr deserializedCount = 0;
		auto read = readOffset;

		if constexpr (HEADER) {
			// Read the header - means we have unpredictable data				
			read = DeserializeAtomFromBinary(source, deserializedCount, read, header, loader);

			DState deserializedState {};
			RequestMoreBytes(source, read, sizeof(DState), loader);
			read += pcCopyMemory(source.At(read), &deserializedState, sizeof(DState));
			result.ToggleState(deserializedState);

			// Finally, read type														
			DataID deserializedType {};
			read = DeserializeInternalFromBinary(
				source, deserializedType, read, header, loader);
			if (!deserializedType)
				return read;

			result.SetDataID(deserializedType, false);
		}
		else {
			// Don't read header - we have a predictable single element,	
			// like a member, a base, or a cast operator sequence				
			// In this case, result should already be allocated and known	
			if (result.IsUntyped() || result.IsEmpty())
				throw Except::BadSerialization("Bad resulting block");
			deserializedCount = result.GetCount();
		}

		if (deserializedCount == 0)
			return read;

		// Fill memory																		
		const bool resolvable = result.GetMeta()->IsResolvable();
		if (!resolvable) {
			if (result.GetMeta()->GetCTTI().mPOD) {
				if (result.IsSparse())
					TODO();

				// If data is POD, optimize by directly memcpying it			
				// This is also used to deserialize InternalIDs if				
				// LANGULUS_RTTI_IS(HASHED)											
				if constexpr (HEADER)
					result.Allocate(deserializedCount, false, true);

				RequestMoreBytes(source, read, result.GetSize(), loader);
				read += pcCopyMemory(source.At(read), result.GetRaw(), result.GetSize());
				return read;
			}
			else if (result.GetMeta()->GetCTTI().mSize == sizeof(Block) && result.InterpretsAs<Block>()) {
				// If data is deep, nest each sub-block							
				if constexpr (HEADER)
					result.Allocate(deserializedCount, true);

				result.ForEach<false>([&](Block& block) {
					read = DeserializeBlockFromBinary<true>(
						source, block, read, header, loader);
				});

				return read;
			}
			else if (result.InterpretsAs<InternalID>()) {
				// Deserialize meta definitions										
				if constexpr (HEADER)
					result.Allocate(deserializedCount, false, true);

				EitherDoThis
					result.ForEach<false>([&](DataID& id) {
						read = DeserializeInternalFromBinary(
							source, id, read, header, loader);
					})
				OrThis
					result.ForEach<false>([&](ConstID& id) {
						read = DeserializeInternalFromBinary(
							source, id, read, header, loader);
					})
				OrThis
					result.ForEach<false>([&](TraitID& id) {
						read = DeserializeInternalFromBinary(
							source, id, read, header, loader);
					})
				OrThis
					result.ForEach<false>([&](VerbID& id) {
						read = DeserializeInternalFromBinary(
							source, id, read, header, loader);
					});

				return read;
			}
			else if (result.InterpretsAs<AMeta>()) {
				SAFETY(if (!result.IsSparse())
					throw Except::BadSerialization(pcLogError << "AMeta not sparse"));

				// Deserialize meta definitions										
				if constexpr (HEADER)
					result.Allocate(deserializedCount, false, true);

				if (result.Is<MetaData>()) {
					auto pointers = const_cast<MetaData const**>(
						reinterpret_cast<MetaData**>(result.GetPointers()));
					for (pcptr i = 0; i < result.GetCount(); ++i)
						read = DeserializeMetaFromBinary<MetaData>(
							source, pointers[i], read, header, loader);
				}
				else if (result.Is<MetaConst>()) {
					auto pointers = const_cast<MetaConst const**>(
						reinterpret_cast<MetaConst**>(result.GetPointers()));
					for (pcptr i = 0; i < result.GetCount(); ++i)
						read = DeserializeMetaFromBinary<MetaConst>(
							source, pointers[i], read, header, loader);
				}
				else if (result.Is<MetaTrait>()) {
					auto pointers = const_cast<MetaTrait const**>(
						reinterpret_cast<MetaTrait**>(result.GetPointers()));
					for (pcptr i = 0; i < result.GetCount(); ++i)
						read = DeserializeMetaFromBinary<MetaTrait>(
							source, pointers[i], read, header, loader);
				}
				else if (result.Is<MetaVerb>()) {
					auto pointers = const_cast<MetaVerb const**>(
						reinterpret_cast<MetaVerb**>(result.GetPointers()));
					for (pcptr i = 0; i < result.GetCount(); ++i)
						read = DeserializeMetaFromBinary<MetaVerb>(
							source, pointers[i], read, header, loader);
				}

				return read;
			}
		}

		if (result.GetMeta()->IsConstructible()) {
			if (result.IsSparse())
				TODO();

			// Type is statically producible, and has default constructor,	
			// therefore we can deserialize it by making a default copy		
			// and then filling in the reflected members and bases			
			for (pcptr i = 0; i < deserializedCount; ++i) {
				auto resolvedType = result.GetDataID();
				if (resolvable)
					read = DeserializeInternalFromBinary(
						source, resolvedType, read, header, loader);

				Any element;
				if constexpr (HEADER) {
					// Create default copy only if not predictable				
					element = Any::From(resolvedType.GetMeta());
					element.Allocate(1, true);
				}
				else {
					// We don't make a default copy if already predictable	
					// It is your responsibility to preallocate and define	
					// the result container												
					element = result.GetElementDense(i);
				}

				for (auto& base : element.GetMeta()->GetBaseList()) {
					if (base.mStaticBase.mOr || base.mBase->IsAbstract())
						continue;
					auto baseBlock = element.GetBaseMemory(base);
					read = DeserializeBlockFromBinary<false>(
						source, baseBlock, read, header, loader);
				}

				for (auto& member : element.GetMeta()->GetMemberList()) {
					auto memberBlock = element.GetMember(member);
					read = DeserializeBlockFromBinary<false>(
						source, memberBlock, read, header, loader);
				}

				if constexpr (HEADER)
					result.InsertBlock(element);
			}

			return read;
		}

		throw Except::BadSerialization(pcLogFuncError
			<< "Can't binary deserialize " << result.GetCount()
			<< " elements of type " << result.GetToken());
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
	pcptr Detail::DeserializeMetaFromBinary(const Bytes& source, META const*& result, pcptr read, const Header& header, const Loader& loader) {
		typename META::InternalType id;
		read = DeserializeInternalFromBinary(source, id, read, header, loader);
		result = id.GetMeta();
		return read;
	}

	/// A snippet for conveniently deserializing an InternalID from binary		
	///	@tparam INTERNAL - type of ID we're deserializing (deducible)			
	///	@param source - the bytes to deserialize										
	///	@param result - [out] the deserialized meta goes here						
	///	@param read - offset into 'from' to start reading from					
	///	@param header - environment header												
	///	@param loader - loader for streaming											
	///	@return number of read bytes														
	template<class INTERNAL>
	pcptr Detail::DeserializeInternalFromBinary(const Bytes& source, INTERNAL& result, pcptr read, const Header& header, const Loader& loader) {
		pcptr count = 0;
		read = DeserializeAtomFromBinary(source, count, read, header, loader);
		if (header.mFlags & Header::Portable) {
			if (count) {
				RequestMoreBytes(source, read, count, loader);
				const auto rawName = reinterpret_cast<const char*>(source.At(read));
				const LiteralText token {rawName, count};
				if constexpr (Same<INTERNAL, DataID>)
					result = PCMEMORY.GetMetaData(token)->GetID();
				else if constexpr (Same<INTERNAL, VerbID>)
					result = PCMEMORY.GetMetaVerb(token)->GetID();
				else if constexpr (Same<INTERNAL, TraitID>)
					result = PCMEMORY.GetMetaTrait(token)->GetID();
				else if constexpr (Same<INTERNAL, ConstID>)
					result = PCMEMORY.GetMetaConst(token)->GetID();
				else LANGULUS_ASSERT("Unsupported internal");
				return read + count;
			}
			else {
				result = {};
				return read;
			}
		}
		else {
			if (header.mAtomSize != sizeof(Hash)) {
				throw Except::BadSerialization(pcLogFuncError
					<< "Binary-incompatible type hash - you should either "
					<< "regenerate the file, or export it as portable");
			}
			static_assert(sizeof(Hash) == sizeof(pcptr), "Size mismatch");
			if (!count) {
				result = {};
				return read;
			}
				
			const Hash h {count};
			if constexpr (Same<INTERNAL, DataID>)
				result = PCMEMORY.GetMetaData(h)->GetID();
			else if constexpr (Same<INTERNAL, VerbID>)
				result = PCMEMORY.GetMetaVerb(h)->GetID();
			else if constexpr (Same<INTERNAL, TraitID>)
				result = PCMEMORY.GetMetaTrait(h)->GetID();
			else if constexpr (Same<INTERNAL, ConstID>)
				result = PCMEMORY.GetMetaConst(h)->GetID();
			else LANGULUS_ASSERT("Unsupported internal");
			return read;
		}
	}

} // namespace PCFW::PCGASM


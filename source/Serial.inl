///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Serial.hpp"
#include "verbs/Do.inl"

#define LGLS_VERBOSE_SERIALIZATION(a)

namespace Langulus::Flow
{

   /// Conversion routine, that is specialized for serialization              
   ///   @tparam TO - the type we're serializing to                           
   ///   @tparam HEADER - ignored when serializing to text formats            
   ///      -  when serializing to binary formats: true if you want to write  
   ///         a portability header for the data (useful for serializing      
   ///         standalone data). The serializer uses this internally when     
   ///         nesting, to reduce redundant writes                            
   ///   @tparam FROM - the type of the item (deducible)                      
   ///   @param item - the item to serialize                                  
   ///   @return the serialized item                                          
   template<CT::Block TO, bool HEADER, CT::Sparse FROM>
   TO Serializer::Serialize(FROM item) {
      if (!item)
         return "<null>";
      return Serialize<TO, HEADER>(*item);
   }

   /// Conversion routine, that is specialized for serialization              
   ///   @tparam TO - the type we're serializing to                           
   ///   @tparam HEADER - ignored when serializing to text formats            
   ///      -  when serializing to binary formats: true if you want to write  
   ///         a portability header for the data (useful for serializing      
   ///         standalone data). The serializer uses this internally when     
   ///         nesting, to reduce redundant writes                            
   ///   @tparam FROM - the type of the item (deducible)                      
   ///   @param item - the item to serialize                                  
   ///   @return the serialized item                                          
   template<CT::Block TO, bool HEADER, CT::Dense FROM>
   TO Serializer::Serialize(const FROM& item) {
      if constexpr (CT::SameAsOneOf<TO, Debug, Text>) {
         ///   DEBUG SERIALIZER                                         
         // Debug serializer doesn't have any restrictions. It is       
         // useful for omitting redundant or irrelevant data, and is    
         // a one-way process. Extensively used by the logger           
         const auto block = Block::From(item);

         try {
            // Attempt converting to debug via reflected converters     
            Debug result;
            (void)SerializeBlock<HEADER>(block, result);
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
            (void)SerializeBlock<HEADER>(block, result);
            return result;
         }
         catch (const Except::Convert&) {}
      }
      else if constexpr (CT::Same<TO, Code>) {
         ///   CODE SERIALIZER                                          
         // Code serializer is strict to allow for deserialization      
         const auto block = Block::From(item);

         try {
            Code result;
            (void)SerializeBlock<HEADER>(block, result);
            return result;
         }
         catch (const Except::Convert&) {}
      }
      else if constexpr (CT::Same<TO, Bytes>) {
         ///   BINARY SERIALIZER                                        
         // Byte serializer is strict to allow for deserialization      
         const auto block = Block::From(item);

         try {
            Bytes result;
            (void)SerializeBlock<HEADER>(block, result);
            return result;
         }
         catch (const Except::Convert&) {}
      }
      else LANGULUS_ERROR("Serializer not implemented");

      // If this is reached, then we weren't able to serialize the item 
      // to the desired type                                            
      throw Except::Convert("Can't serialize");
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Deserialize from Code/Bytes                                            
   ///   @tparam FROM - the type we're deserializing from (deducible)         
   ///   @param item - the item to deserialize                                
   ///   @return the deserialized contents inside a container                 
   template<CT::Block FROM>
   Any Serializer::Deserialize(const FROM& item) {
      if constexpr (CT::Same<FROM, Debug>) {
         LANGULUS_ERROR("You can't deserialize debug containers "
                        " - debug serialization is a one-way process");
      }
      else if constexpr (CT::Same<FROM, Code>) {
         return item.Parse();
      }
      else if constexpr (CT::Same<FROM, Bytes>) {
         Header header;
         Any result;
         (void)DeserializeBlock<true>(item, result, 0, header, {});
         return result;
      }
      else LANGULUS_ERROR("Deserializer not implemented");
   }
#endif

   /// Convert a byte to hexadecimal string, and append it to text container  
   ///   @param from - the byte to convert to hexadecimal                     
   ///   @param to - the container to append to                               
   inline void ToHex(const Byte& from, Text& to) {
      fmt::format_to_n(to.Extend(2).GetRaw(), 2, "{:x}", from.mValue);
   }

   /// Check if a memory block needs a Code scope decorated                   
   ///   @param block - the memory block to check                             
   ///   @return true if a scope is required around the block                 
   inline bool Serializer::NeedsScope(const Block& block) noexcept {
      return block.GetCount() > 1 || block.IsInvalid();
   }

   /// Add a separator                                                        
   ///   @param isOr - OR separator or not                                    
   ///   @return the text equivalent of the separator                         
   inline Text Serializer::Separator(bool isOr) {
      return isOr 
         ? Verbs::Conjunct::CTTI_NegativeOperator 
         : Verbs::Conjunct::CTTI_PositiveOperator;
   }

   /// Serialize any block to any string format                               
   ///   @tparam ENSCOPED - whether or not the block is already enscoped      
   ///   @param from - the block to serialize                                 
   ///   @param to - [out] the serialized block goes here                     
   ///   @return the number of written characters                             
   template<bool ENSCOPED, CT::Text TO>
   Count Serializer::SerializeBlock(const Block& from, TO& to) {
      const auto initial = to.GetCount();
      bool stateWritten = false;
      if (from.IsConstant()) {
         to += Code {Code::Constant};
         stateWritten = true;
      }

      UNUSED() bool scoped;
      if constexpr (!ENSCOPED) {
         scoped = NeedsScope(from);
         if (scoped) {
            if (from.IsPast())
               to += Code {Code::Past};
            else if (from.IsFuture())
               to += Code {Code::Future};

            to += Code {Code::OpenScope};
            stateWritten = false;
         }
      }

      if (!from.IsEmpty()) {
         // Add a bit of spacing                                        
         if (stateWritten)
            to += Text {' '};

         if (from.IsDeep()) {
            // Nested serialization, wrap it in content scope           
            for (Offset i = 0; i < from.GetCount(); ++i) {
               (void) SerializeBlock<false>(from.As<Block>(i), to);
               if (i < from.GetCount() - 1)
                  to += Separator(from.IsOr());
            }
         }
         else if (from.CastsTo<bool>()) {
            // Contained type is boolean                                
            for (Offset i = 0; i < from.GetCount(); ++i) {
               to += from.As<bool>(i) ? TO {"yes"} : TO {"no"};
               if (i < from.GetCount() - 1)
                  to += Separator(from.IsOr());
            }
         }
         else if (from.CastsTo<A::Number>()) {
            // Contained type is some kind of a number                  
            if (from.CastsTo<Float, true>())
               SerializeNumber<Float>(from, to);
            else if (from.CastsTo<Double, true>())
               SerializeNumber<Double>(from, to);
            //else if (from.CastsTo<uint8_t, true>()) //this gets mistaken as letter, so not considered number
            //   SerializeNumber<uint8_t>(from, to);
            else if (from.CastsTo<uint16_t, true>())
               SerializeNumber<uint16_t>(from, to);
            else if (from.CastsTo<uint32_t, true>())
               SerializeNumber<uint32_t>(from, to);
            else if (from.CastsTo<uint64_t, true>())
               SerializeNumber<uint64_t>(from, to);
            else if (from.CastsTo<int8_t, true>())
               SerializeNumber<int8_t>(from, to);
            else if (from.CastsTo<int16_t, true>())
               SerializeNumber<int16_t>(from, to);
            else if (from.CastsTo<int32_t, true>())
               SerializeNumber<int32_t>(from, to);
            else if (from.CastsTo<int64_t, true>())
               SerializeNumber<int64_t>(from, to);
            else {
               Logger::Error("Can't serialize block of ",
                  from.GetToken(), " to ", RTTI::NameOf<TO>(),
                  " - the number type is not implemented");
               LANGULUS_THROW(Convert, "Can't serialize numbers to text");
            }
         }
         else if (from.CastsTo<Letter>()) {
            // Contained type is a character                            
            for (Offset i = 0; i < from.GetCount(); ++i) {
               to += Code {Code::OpenCharacter};
               to += Text {from.As<Letter>(i)};
               to += Code {Code::CloseCharacter};
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
         else if (from.CastsTo<Trait>()) {
            // Contained type is trait, serialize it                    
            for (Offset i = 0; i < from.GetCount(); ++i) {
               auto& trait = from.As<Trait>(i);
               to += trait.GetTrait()
                  ? trait.GetTrait()->mToken
                  : MetaTrait::DefaultToken;
               to += Code {Code::OpenScope};
               (void) SerializeBlock<false>(trait, to);
               to += Code {Code::CloseScope};
               if (i < from.GetCount() - 1)
                  to += Separator(from.IsOr());
            }
         }
         else if (from.CastsTo<RTTI::Meta>()) {
            // Contained type is meta definitions, write the token      
            for (Offset i = 0; i < from.GetCount(); ++i) {
               auto meta = from.As<const RTTI::Meta*>(i);
               to += TO {meta};
               if (i < from.GetCount() - 1)
                  to += Separator(from.IsOr());
            }
         }
         else if (from.CastsTo<Verb>()) {
            // Contained type is verb                                   
            for (Offset i = 0; i < from.GetCount(); ++i) {
               auto& verb = from.As<Verb>(i);
               to += verb.operator TO();
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
            }
            else for (Offset i = 0; i < from.GetCount(); ++i) {
               to += Code {Code::OpenByte};
               ToHex(raw_bytes[i], to);
               if (i < from.GetCount() - 1)
                  to += Separator(from.IsOr());
            }
         }
         else {
            // Serialize all elements one by one using RTTI             
            Verbs::InterpretTo<TO> interpreter;
            interpreter.ShortCircuit(false);

            if (DispatchFlat(from, interpreter)) {
               bool separate = false;
               interpreter.GetOutput().ForEach([&](const Text& r) {
                  if (separate)
                     to += Separator(from.IsOr());
                  separate = true;
                  to += r;
               });
            }
            else {
               Logger::Error("Can't serialize block of type ",
                  from.GetToken(), " to ", RTTI::NameOf<TO>());
               LANGULUS_THROW(Convert, "Can't serialize block to text");
            }
         }
      }

      // Close scope                                                    
      if constexpr (!ENSCOPED) {
         if (scoped)
            to += Code {Code::CloseScope};
         else {
            if (from.IsPast())
               to += Code {Code::Past};
            else if (from.IsFuture())
               to += Code {Code::Future};
         }
      }
      else {
         if (from.IsPast())
            to += Code {Code::Past};
         else if (from.IsFuture())
            to += Code {Code::Future};
      }

      return to.GetCount() - initial;
   }

   /// A snippet for serializing a reference to a meta object                 
   ///   @param from - the member block to serialize                          
   ///   @param to - [out] the serialized data                                
   ///   @param member - reflection data about the member                     
   template<CT::Meta META, CT::Text TO>
   void Serializer::SerializeMeta(const Block& from, TO& to, const RTTI::Member* member) {
      auto meta = member->template As<META>(from.GetRaw());
      if (meta)   to += meta->GetToken();
      else        to += Decay<META>::DefaultToken;
   }

   /// A snippet for serializing a block of numbers                           
   ///   @param from - the number block to serialize                          
   ///   @param to - [out] the serialized data                                
   template<CT::Number T, CT::Text TO>
   void Serializer::SerializeNumber(const Block& from, TO& to) {
      if (from.IsDense()) {
         // Significantly faster routine if dense                       
         auto data = from.template GetRawAs<T>();
         const auto dataEnd = from.template GetRawEndAs<T>();
         while (data != dataEnd) {
            to += TO {*data};
            if (from.GetType()->mSuffix.size())
               to += from.GetType()->mSuffix;
            if (data < dataEnd - 1)
               to += Separator(from.IsOr());
            ++data;
         }
      }
      else {
         for (Offset i = 0; i < from.GetCount(); ++i) {
            to += TO {from.template As<T>(i)};
            to += from.GetType()->mSuffix;
            if (i < from.GetCount() - 1)
               to += Separator(from.IsOr());
         }
      }
   }

   /// Serialize all reflected data members in all bases                      
   ///   @param from - the member block to serialize                          
   ///   @param to - the serialized data                                      
   template<CT::Text TO>
   void Serializer::SerializeMembers(const Block& from, TO& to) {
      if (from.template Is<Block>() || from.template Is<Any>()) {
         SerializeBlock(from.template Get<Block>(), to);
         return;
      }

      // Append a separator?                                            
      bool separate = false;

      // First we serialize all bases' members                          
      for (auto& base : from.GetType()->mBases) {
         if (base.mType->mSize > 0) {
            if (separate) {
               to += Verbs::Conjunct::CTTI_PositiveOperator;
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
            to += Verbs::Conjunct::CTTI_PositiveOperator;

         if (member.template Is<DMeta>())
            SerializeMeta<DMeta>(from, to, &member);
         else if (member.template Is<TMeta>())
            SerializeMeta<TMeta>(from, to, &member);
         else if (member.template Is<VMeta>())
            SerializeMeta<VMeta>(from, to, &member);
         else if (member.template Is<CMeta>())
            SerializeMeta<CMeta>(from, to, &member);
         else
            SerializeBlock(from.GetMember(member), to);

         separate = true;
      }
   }

   /// Default header constructor                                             
   inline Serializer::Header::Header() noexcept {
      mAtomSize = sizeof(Size);

      // First bit of the flag means the file was written by a big      
      // endian machine                                                 
      mFlags = Default;
      if constexpr (BigEndianMachine)
         mFlags = BigEndian;
      mUnused = 0;
   }

   inline bool Serializer::Header::operator == (const Header& rhs) const noexcept {
      return mAtomSize == rhs.mAtomSize && mFlags == rhs.mFlags;
   }

   /// Inner binary serialization routine                                     
   ///   @tparam HEADER - true if you want to write a portability header      
   ///                    (useful for serializing standalone data)            
   ///   @param source - the block to serialize                               
   ///   @param result - [out] the resulting byte array                       
   ///   @return the number of written bytes                                  
   template<bool HEADER>
   void Serializer::SerializeBlock(const Block& source, Bytes& result) {
      if constexpr (HEADER) {
         result += Bytes {source.GetCount()};
         result += Bytes {source.GetUnconstrainedState()};
         result += Bytes {source.GetType()};
      }

      if (source.IsEmpty() || source.IsUntyped())
         return;

      const bool resolvable = source.IsResolvable();
      if (!resolvable) {
         if (source.IsPOD()) {
            // If data is POD, optimize by directly memcpying it        
            const auto denseStride = source.GetStride();
            const auto byteCount = denseStride * source.GetCount();
            result.AllocateMore(result.GetCount() + byteCount);

            if (source.IsSparse()) {
               // ... pointer by pointer if sparse                      
               auto p = source.GetRawSparse();
               const auto pEnd = p + source.GetCount();
               while (p != pEnd)
                  result += Bytes {p++, denseStride};
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
               [&result](DMeta meta) { result += Bytes {meta}; },
               [&result](VMeta meta) { result += Bytes {meta}; },
               [&result](TMeta meta) { result += Bytes {meta}; },
               [&result](CMeta meta) { result += Bytes {meta}; }
            );

            return;
         }
         else if (source.CastsTo<Verb>()) {
            // Serialize verb                                           
            source.ForEach([&result](const Verb& verb) {
               result += Bytes {verb.GetVerb()};
               result += Bytes {verb.GetCharge()};
               result += Bytes {verb.GetVerbState()};
               SerializeBlock<true>(verb.GetSource(), result);
               SerializeBlock<true>(verb.GetArgument(), result);
            });

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
               result += Bytes {element.GetType()};

            // Serialize all reflected bases                            
            for (auto& base : element.GetType()->mBases) {
               // Imposed bases are never serialized                    
               if (base.mImposed || base.mType->mIsAbstract)
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
      LANGULUS_THROW(Convert, "Can't binary serialize");
   }

   inline void Serializer::RequestMoreBytes(const Bytes& source, Offset read, Size byteCount, const Loader& loader) {
      if (read >= source.GetCount() || source.GetCount() - read < byteCount) {
         if (!loader)
            LANGULUS_THROW(Access, "Deserializer has no loader");
         loader(const_cast<Bytes&>(source), byteCount - (source.GetCount() - read));
      }
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Read an atom-sized unsigned integer, based on the provided header      
   ///   @param source - the serialized byte source                           
   ///   @param result - [out] the resulting deserialized number              
   ///   @param read - offset to apply to serialized byte array               
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return the number of read bytes from byte container                 
   inline Size Serializer::DeserializeAtom(const Bytes& source, Offset& result, Offset read, const Header& header, const Loader& loader) {
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
            LANGULUS_THROW(Convert,
               "Deserialized atom contains a value "
               "too powerful for your architecture");
         }
         result = static_cast<Offset>(count8);
      }
      else {
         LANGULUS_THROW(Convert,
            "An unknown atomic size was deserialized "
            "from source - is the source corrupted?");
      }

      return read;
   }

   /// Inner deserialization routine from binary                              
   ///   @tparam HEADER - true if you want to read a portability header       
   ///                    (useful for deserializing standalone data)          
   ///   @param source - bytes to deserialize                                 
   ///   @param result - [out] the resulting deserialized data                
   ///   @param readOffset - offset to apply to serialized byte array         
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return the number of read/peek bytes from byte container            
   template<bool HEADER>
   Size Serializer::DeserializeBlock(const Bytes& source, Block& result, Offset readOffset, const Header& header, const Loader& loader) {
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
         ::std::memcpy(static_cast<void*>(&deserializedState), source.At(read), sizeof(DataState));
         read += sizeof(DataState);
         result.AddState(deserializedState);

         // Finally, read type                                          
         DMeta deserializedType;
         read = DeserializeMeta(
            source, deserializedType, read, header, loader);

         if (!deserializedType)
            return read;

         result.Mutate<false>(deserializedType);
      }
      else {
         // Don't read header - we have a predictable single element,   
         // like a member, a base, or a cast operator sequence          
         // In this case, result should already be allocated and known  
         if (result.IsUntyped() || result.IsEmpty())
            LANGULUS_THROW(Convert, "Bad resulting block");

         deserializedCount = result.GetCount();
      }

      if (deserializedCount == 0)
         return read;

      // Fill memory                                                    
      const bool resolvable = result.IsResolvable();
      if (!resolvable) {
         if (result.IsPOD()) {
            // If data is POD, optimize by directly memcpying it        
            if constexpr (HEADER)
               result.AllocateMore<false, true>(deserializedCount);

            const auto byteSize = result.GetByteSize();
            RequestMoreBytes(source, read, byteSize, loader);

            if (result.IsSparse()) {
               // Allocate a separate block for the elements            
               const auto temporary = Allocator::Allocate(byteSize);
               auto start = temporary->GetBlockStart();
               ::std::memcpy(start, source.At(read), byteSize);
               read += byteSize;
               temporary->Keep(deserializedCount - 1);

               // Write a pointer to each element                       
               auto p = result.template GetHandle<Byte*>(0);
               const auto pEnd = p + result.GetCount();
               const auto size = result.GetType()->mSize;
               while (p != pEnd) {
                  p.New(start, temporary);
                  start += size;
               }
            }
            else {
               // Data is dense, parse it at once                       
               ::std::memcpy(result.GetRaw(), source.At(read), byteSize);
               read += byteSize;
            }

            return read;
         }
         else if (result.IsDeep()) {
            // If data is deep, nest each sub-block                     
            if constexpr (HEADER)
               result.New(deserializedCount);

            result.ForEach([&](Block& block) {
               read = DeserializeBlock<true>(
                  source, block, read, header, loader);
            });

            return read;
         }
         else if (result.CastsTo<RTTI::Meta>()) {
            // Deserialize meta definitions                             
            // The resulting container should be always const & sparse  
            if constexpr (HEADER)
               result.New(deserializedCount);

            auto p = result.template GetHandle<Byte*>(0);
            const auto pEnd = p + result.GetCount();
            if (result.IsExact<DMeta>()) {
               while (p != pEnd) {
                  DMeta ptr;
                  read = DeserializeMeta(source, ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaData*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (result.IsExact<TMeta>()) {
               while (p != pEnd) {
                  TMeta ptr;
                  read = DeserializeMeta(source, ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaTrait*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (result.IsExact<VMeta>()) {
               while (p != pEnd) {
                  VMeta ptr;
                  read = DeserializeMeta(source, ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaVerb*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (result.IsExact<CMeta>()) {
               while (p != pEnd) {
                  CMeta ptr;
                  read = DeserializeMeta(source, ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaConst*>(ptr)),
                     nullptr
                  );
               }
            }
            else LANGULUS_THROW(Convert, "Bad meta container");

            return read;
         }
         else if (result.CastsTo<Verb>()) {
            // If data is verb, deserialize it here                     
            if constexpr (HEADER)
               result.New(deserializedCount);

            result.ForEach([&](Verb& verb) {
               // Deserialize verb type                                 
               VMeta ptr;
               read = DeserializeMeta(source, ptr, read, header, loader);
               verb.SetVerb(ptr);

               // Deserialize the charge                                
               Charge charge;
               RequestMoreBytes(source, read, sizeof(Charge), loader);
               ::std::memcpy(static_cast<void*>(&charge), source.At(read), sizeof(Charge));
               read += sizeof(Charge);
               verb.SetCharge(charge);

               // Deserialize the verb state                            
               VerbState vstate;
               RequestMoreBytes(source, read, sizeof(VerbState), loader);
               ::std::memcpy(static_cast<void*>(&vstate), source.At(read), sizeof(VerbState));
               read += sizeof(VerbState);
               verb.SetVerbState(vstate);

               // Deserialize source                                    
               read = DeserializeBlock<true>(
                  source, verb.GetSource(), read, header, loader);

               // Deserialize argument                                  
               read = DeserializeBlock<true>(
                  source, verb.GetArgument(), read, header, loader);
            });

            return read;
         }
      }

      if (result.IsDefaultable()) {
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
               element = Any::FromMeta(resolvedType);
               element.New(1);
            }
            else {
               // We don't make a default copy if already predictable   
               // It is your responsibility to preallocate and define   
               // the result container                                  
               element = result.GetElementDense(i);
            }

            // Deserialize all reflected bases                          
            for (auto& base : element.GetType()->mBases) {
               if (base.mImposed || base.mType->mIsAbstract)
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
      LANGULUS_THROW(Convert, "Can't binary-deserialize");
   }

   /// A snippet for conveniently deserializing a meta from binary            
   ///   @tparam META - type of meta we're deserializing (deducible)          
   ///   @param source - the bytes to deserialize                             
   ///   @param result - [out] the deserialized meta goes here                
   ///   @param read - byte offset inside 'from'                              
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return number of read bytes                                         
   template<class META>
   Size Serializer::DeserializeMeta(const Bytes& source, META const*& result, Offset read, const Header& header, const Loader& loader) {
      Count count = 0;
      read = DeserializeAtom(source, count, read, header, loader);
      if (count) {
         RequestMoreBytes(source, read, count, loader);
         const Token token {source.GetRawAs<Letter>() + read, count};
         if constexpr (CT::Same<META, MetaData>)
            result = RTTI::Database.GetMetaData(token);
         else if constexpr (CT::Same<META, MetaVerb>)
            result = RTTI::Database.GetMetaVerb(token);
         else if constexpr (CT::Same<META, MetaTrait>)
            result = RTTI::Database.GetMetaTrait(token);
         else if constexpr (CT::Same<META, MetaConst>)
            result = RTTI::Database.GetMetaConstant(token);
         else
            LANGULUS_ERROR("Unsupported meta deserialization");

         return read + count;
      }

      result = {};
      return read;
   }
#endif

} // namespace Langulus::Flow


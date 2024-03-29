///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Serial.hpp"
#include "verbs/Do.inl"
#include "verbs/Conjunct.inl"
#include "verbs/Interpret.inl"

#ifndef LANGULUS_MAX_DEBUGGABLE_ELEMENTS
   #if LANGULUS(DEBUG) or LANGULUS(SAFE)
      #define LANGULUS_MAX_DEBUGGABLE_ELEMENTS 32
   #else
      #define LANGULUS_MAX_DEBUGGABLE_ELEMENTS 8
   #endif
#endif


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
   template<CT::Block TO, bool HEADER, CT::Sparse FROM, CT::Block TO_ORIGINAL>
   TO Serialize(FROM item) {
      if (not item)
         return "<null>";
      return Serialize<TO, HEADER, Deptr<FROM>, TO_ORIGINAL>(*item);
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
   template<CT::Block TO, bool HEADER, CT::Dense FROM, CT::Block TO_ORIGINAL>
   TO Serialize(const FROM& item) {
      Block block = Block::From(const_cast<FROM&>(item));

      if constexpr (CT::SameAsOneOf<TO_ORIGINAL, Debug, Text>) {
         ///   DEBUG SERIALIZER                                         
         // Debug serializer doesn't have any restrictions. It is       
         // useful for omitting redundant or irrelevant data, and is    
         // a one-way process. Extensively used by the logger           
         // Crop it down if block is too big                            
         constexpr auto countCap = LANGULUS_MAX_DEBUGGABLE_ELEMENTS;
         const auto originalCount = block.GetCount();
         if (block.GetCount() > countCap)
            block = block.Crop(0, countCap);

         try {
            // Attempt converting to debug via reflected converters     
            Debug result;
            (void) block.template Serialize<HEADER, Debug, TO_ORIGINAL>(result);
            if (block.GetCount() != originalCount) {
               result += " (";
               result += originalCount - block.GetCount();
               result += " more elements omitted)";
            }
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
            (void) block.template Serialize<HEADER, Code, TO_ORIGINAL>(result);
            if (block.GetCount() != originalCount) {
               result += " (";
               result += originalCount - block.GetCount();
               result += " more elements omitted)";
            }
            return result;
         }
         catch (const Except::Convert&) {}

         // If reached, then no debug serialization was available       
         return "<not debuggable>";
      }
      else if constexpr (CT::Text<TO_ORIGINAL>) {
         ///   CODE SERIALIZER                                          
         // Code serializer is strict to allow for deserialization      
         try {
            TO_ORIGINAL result;
            (void) block.template Serialize<HEADER, TO_ORIGINAL, TO_ORIGINAL>(result);
            return result;
         }
         catch (const Except::Convert&) {}
      }
      else if constexpr (CT::Bytes<TO_ORIGINAL>) {
         ///   BINARY SERIALIZER                                        
         // Byte serializer is strict to allow for deserialization      
         try {
            TO_ORIGINAL result;
            (void) block.template Serialize<HEADER>(result);
            return result;
         }
         catch (const Except::Convert&) {}
      }
      else LANGULUS_ERROR("Serializer not implemented");

      // If this is reached, then we weren't able to serialize the item 
      // to the desired type                                            
      LANGULUS_OOPS(Convert, "Can't serialize ",
         " type `", NameOf<FROM>(), "` as `", NameOf<TO>(),
         "` (final target being `", NameOf<TO_ORIGINAL>(), "`)"
      );
      return {};
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Deserialize from text or binary                                        
   ///   @tparam FROM - the type we're deserializing from (deducible)         
   ///   @param item - the item to deserialize                                
   ///   @return the deserialized contents inside a container                 
   template<CT::Block FROM>
   Any Deserialize(const FROM& from) {
      if constexpr (CT::Text<FROM>) {
         // Assume it's code and attempt to parse it                    
         return Code {from}.Parse();
      }
      else if constexpr (CT::Same<FROM, Bytes>) {
         // Deserialize from binary                                     
         Bytes::Header header;
         Any result;
         (void) from.template Deserialize<true>(result, header);
         return Abandon(result);
      }
      else LANGULUS_ERROR("Deserializer not implemented");
   }
#endif

} // namespace Langulus::Flow


namespace Langulus::Anyness
{
   
   /// Serialize any block to any string format                               
   ///   @tparam ENSCOPED - whether or not the block is already enscoped, if  
   ///           text, or true if you want to write a portability header, if  
   ///           serializing to binary (useful for standalone data)           
   ///   @tparam TO - what are we serializing to (deducible)                  
   ///   @tparam TO_ORIGINAL - keeps track what was the original TO           
   ///   @param from - the block to serialize                                 
   ///   @param to - [out] the serialized block goes here                     
   ///   @return the number of written characters/bytes                       
   template<bool ENSCOPED, class TO, class TO_ORIGINAL>
   Count Block::Serialize(TO& to) const {
      using namespace Flow;
      using namespace Serial;
      const auto initial = to.GetCount();

      if constexpr (CT::Text<TO>) {
         //                                                             
         //  Serialize to text                                          
         //                                                             
         bool stateWritten = false;

         UNUSED() bool scoped;
         if constexpr (not ENSCOPED) {
            if (IsConstant()) {
               to += Code {Code::Constant};
               stateWritten = true;
            }

            scoped = Serial::NeedsScope(*this);
            if (scoped) {
               if (IsPast())
                  to += Code {Code::Past};
               else if (IsFuture())
                  to += Code {Code::Future};

               to += Code {Code::OpenScope};
               stateWritten = false;
            }
         }

         if (not IsEmpty()) {
            // Add a bit of spacing                                     
            if (stateWritten)
               to += Text {' '};

            if (IsDeep()) {
               // Nested serialization, wrap it in content scope        
               for (Offset i = 0; i < GetCount(); ++i) {
                  to += Flow::Serialize<TO, false, Block, TO_ORIGINAL>(As<Block>(i));
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (Is<bool>()) {
               // Contained type is boolean                             
               for (Offset i = 0; i < GetCount(); ++i) {
                  to += As<bool>(i) ? TO {"yes"} : TO {"no"};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (Is<Letter>()) {
               // Contained type is a character                         
               for (Offset i = 0; i < GetCount(); ++i) {
                  to += Code {Code::OpenCharacter};
                  to += Text {As<Letter>(i)};
                  to += Code {Code::CloseCharacter};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (Is<Float>()) {
               SerializeNumber<Float>(*this, to);
            }
            else if (Is<Double>()) {
               SerializeNumber<Double>(*this, to);
            }
            else if (Is<uint8_t>()) {
               SerializeNumber<uint8_t>(*this, to);
            }
            else if (Is<uint16_t>()) {
               SerializeNumber<uint16_t>(*this, to);
            }
            else if (Is<uint32_t>()) {
               SerializeNumber<uint32_t>(*this, to);
            }
            else if (Is<uint64_t>()) {
               SerializeNumber<uint64_t>(*this, to);
            }
            else if (Is<int8_t>()) {
               SerializeNumber<int8_t>(*this, to);
            }
            else if (Is<int16_t>()) {
               SerializeNumber<int16_t>(*this, to);
            }
            else if (Is<int32_t>()) {
               SerializeNumber<int32_t>(*this, to);
            }
            else if (Is<int64_t>()) {
               SerializeNumber<int64_t>(*this, to);
            }
            else if (CastsTo<Code>()) {
               // Contained type is code, wrap it in code scope         
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& text = As<Code>(i);
                  to += Code {Code::OpenCode};
                  to += text;
                  to += Code {Code::CloseCode};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Text>()) {
               // Contained type is text, wrap it in string scope       
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& text = As<Text>(i);
                  to += Code {Code::OpenString};
                  to += text;
                  to += Code {Code::CloseString};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Trait>()) {
               // Contained type is trait, serialize it                 
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& trait = As<Trait>(i);
                  to += trait.GetTrait()
                     ? trait.GetTrait()->mToken
                     : MetaTrait::DefaultToken;
                  to += Code {Code::OpenScope};
                  to += Flow::Serialize<TO, false, Block, TO_ORIGINAL>(trait);
                  to += Code {Code::CloseScope};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<RTTI::Meta>()) {
               // Contained type is meta definitions, write the token   
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto meta = As<const RTTI::Meta*>(i);
                  to += TO {meta};
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Verb>()) {
               // Contained type is verb                                
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& verb = As<Verb>(i);
                  to += verb.operator TO();
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Construct>()) {
               // Contained type is Neat                                
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& construct = As<Construct>(i);
                  to += construct.template SerializeAs<TO_ORIGINAL>();
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Neat>()) {
               // Contained type is Neat                                
               for (Offset i = 0; i < GetCount(); ++i) {
                  auto& neat = As<Neat>(i);
                  to += neat.template SerializeAs<TO_ORIGINAL>();
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (CastsTo<Byte>()) {
               // Contained type is raw bytes, wrap it in byte scope    
               auto raw_bytes = GetRaw();
               if (not IsOr()) {
                  to += Code {Code::OpenByte};
                  for (Offset i = 0; i < GetCount(); ++i)
                     ToHex(raw_bytes[i], to);
               }
               else for (Offset i = 0; i < GetCount(); ++i) {
                  to += Code {Code::OpenByte};
                  ToHex(raw_bytes[i], to);
                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else if (not mType->mNamedValues.empty()) {
               // Contained type has named values, we can use those     
               for (Offset i = 0; i < GetCount(); ++i) {
                  const auto lhs = GetElementDense(i);
                  bool found = false;
                  for (auto cmeta : mType->mNamedValues) {
                     // Compare each element in Block with each constant
                     const Block rhs {{}, cmeta};
                     if (lhs == rhs) {
                        // Match found                                  
                        to += TO {cmeta};
                        found = true;
                        break;
                     }
                  }

                  if (not found) {
                     if constexpr (CT::SameAsOneOf<TO_ORIGINAL, Debug, Text>) {
                        // Don't pollute with messages while serializing
                        // for logging, just return a default name      
                        to += "<unserializable named value>";
                     }
                     else {
                        LANGULUS_OOPS(Convert, "Can't serialize block",
                           " of type `", GetToken(), "` to `", NameOf<TO>(),
                           "` (final target being `", NameOf<TO_ORIGINAL>(), "`)"
                           " because a named value couldn't be found in reflection"
                        );
                     }
                  }

                  if (i < GetCount() - 1)
                     to += Separator(IsOr());
               }
            }
            else {
               // Serialize one by one using Verbs::Interpret           
               // This is the slowest routine                           
               Verbs::InterpretAs<TO> interpreter;
               interpreter.ShortCircuit(false);

               if (DispatchFlat(*this, interpreter)) {
                  bool separate = false;
                  interpreter->ForEach([&](const Text& r) {
                     if (separate)
                        to += Separator(IsOr());
                     separate = true;

                     if (r.IsEmpty())
                        to += "<empty text interpretation>";
                     else
                        to += r;
                  });
               }
               else if constexpr (CT::SameAsOneOf<TO_ORIGINAL, Debug, Text>) {
                  // Don't pollute with messages while serializing      
                  // for logging, just throw                            
                  LANGULUS_THROW(Convert, "Can't serialize block");
               }
               else {
                  LANGULUS_OOPS(Convert, "Can't serialize block",
                     " of type `", GetToken(), "` to `", NameOf<TO>(),
                     "` (final target being `", NameOf<TO_ORIGINAL>(), "`)"
                  );
               }
            }
         }

         // Close scope                                                 
         if constexpr (not ENSCOPED) {
            if (scoped)
               to += Code {Code::CloseScope};
            else {
               if (IsPast())
                  to += Code {Code::Past};
               else if (IsFuture())
                  to += Code {Code::Future};
            }
         }
         else {
            if (IsPast())
               to += Code {Code::Past};
            else if (IsFuture())
               to += Code {Code::Future};
         }

         return to.GetCount() - initial;
      }
      else if constexpr (CT::Same<TO, Bytes>) {
         //                                                             
         //  Serialize to binary                                        
         //                                                             
         if constexpr (ENSCOPED) {
            to += Bytes {GetCount()};
            to += Bytes {GetUnconstrainedState()};
            to += Bytes {GetType()};
         }

         if (IsEmpty() or IsUntyped())
            return to.GetCount() - initial;

         const bool resolvable = IsResolvable();
         if (not resolvable) {
            if (IsPOD()) {
               // If data is POD, optimize by directly memcpying it     
               const auto denseStride = GetStride();
               const auto byteCount = denseStride * GetCount();
               to.AllocateMore(to.GetCount() + byteCount);

               if (IsSparse()) {
                  // ... pointer by pointer if sparse                   
                  auto p = GetRawSparse();
                  const auto pEnd = p + GetCount();
                  while (p != pEnd)
                     to += Bytes {p++, denseStride};
               }
               else {
                  // ... at once if dense                               
                  to += Bytes {GetRaw(), byteCount};
               }

               return to.GetCount() - initial;
            }
            else if (IsDeep()) {
               // If data is deep, nest-serialize each sub-block        
               ForEach([&to](const Block& block) {
                  (void) block.Serialize<true>(to);
               });

               return to.GetCount() - initial;
            }
            else if (CastsTo<RTTI::Meta>()) {
               // Serialize meta                                        
               ForEach(
                  [&to](DMeta meta) { to += Bytes {meta}; },
                  [&to](VMeta meta) { to += Bytes {meta}; },
                  [&to](TMeta meta) { to += Bytes {meta}; },
                  [&to](CMeta meta) { to += Bytes {meta}; }
               );

               return to.GetCount() - initial;
            }
            else if (CastsTo<Verb>()) {
               // Serialize verb                                        
               ForEach([&to](const Verb& verb) {
                  to += Bytes {verb.GetVerb()};
                  to += Bytes {verb.GetCharge()};
                  to += Bytes {verb.GetVerbState()};
                  (void) verb.GetSource().Serialize<true>(to);
                  (void) verb.GetArgument().Serialize<true>(to);
               });

               return to.GetCount() - initial;
            }
         }
      
         if (mType->mDefaultConstructor) {
            // Type is statically creatable, and has default constructor
            // therefore we can serialize it by serializing each        
            // reflected base and member                                
            for (Count i = 0; i < GetCount(); ++i) {
               auto element = GetElementResolved(i);
               if (resolvable)
                  to += Bytes {element.GetType()};

               // Serialize all reflected bases                         
               for (auto& base : element.GetType()->mBases) {
                  // Imposed bases are never serialized                 
                  if (base.mImposed or base.GetType()->mIsAbstract)
                     continue;

                  const auto baseBlock = element.GetBaseMemory(base);
                  (void) baseBlock.Serialize<false>(to);
               }

               // Serialize all reflected members                       
               for (auto& member : element.GetType()->mMembers) {
                  const auto memberBlock = element.GetMember(member);
                  (void) memberBlock.Serialize<false>(to);
               }
            }

            return to.GetCount() - initial;
         }

         // Failure if reached                                          
         LANGULUS_OOPS(Convert, "Can't serialize ",
            " type `", GetToken(), "` as `", NameOf<TO>(),
            "` (final target being `", NameOf<TO_ORIGINAL>(), "`)"
         );
         return 0;
      }
      else LANGULUS_ERROR("Unsupported serialization");
   }

   /// Serialize a Neat container                                             
   ///   @tparam T - text type to serialize as                                
   ///   @return the serialized data                                          
   template<CT::Text T>
   T Neat::SerializeAs() const {
      using Flow::Code;
      T to;
      to += Code {Code::OpenScope};
      bool separator = false;
      for (auto pair : mAnythingElse) {
         for (auto& group : pair.mValue) {
            if (separator)
               to += ", ";

            if (group.IsValid())
               to += Flow::Serialize<T, false>(group);
            else
               to += pair.mKey;
            separator = true;
         }
      }

      for (auto pair : mTraits) {
         for (auto& trait : pair.mValue) {
            if (separator)
               to += ", ";

            if (trait.IsValid())
               to += Flow::Serialize<T, false>(Trait::From(pair.mKey, trait));
            else
               to += pair.mKey;
            separator = true;
         }
      }

      for (auto pair : mConstructs) {
         for (auto& construct : pair.mValue) {
            if (separator)
               to += ", ";

            if (construct.mData.IsValid() or not construct.mCharge.IsDefault())
               to += Flow::Serialize<T, false>(Construct(pair.mKey, construct.mData, construct.mCharge));
            else
               to += pair.mKey;
            separator = true;
         }
      }
      to += Code {Code::CloseScope};
      return to;
   }
   
   /// Serialize a construct to a desired text type                           
   ///   @tparam T - type to serialize as                                     
   ///   @return the serialized container                                     
   template<CT::Text T>
   LANGULUS(INLINED)
   T Construct::SerializeAs() const {
      T to;
      to += GetType();
      to += GetCharge().operator Debug();
      to += GetDescriptor().template SerializeAs<T>();
      return to;
   }

   /// Explicit logging operator for Construct                                
   LANGULUS(INLINED)
   Construct::operator Debug() const {
      return SerializeAs<Debug>();
   }
   
#if LANGULUS_FEATURE(MANAGED_REFLECTION)

   /// Inner deserialization routine from binary                              
   ///   @tparam HEADER - true if you want to read a portability header       
   ///                    (useful for deserializing standalone data)          
   ///   @param source - bytes to deserialize                                 
   ///   @param result - [out] the resulting deserialized data                
   ///   @param readOffset - offset to apply to serialized byte array         
   ///   @param header - environment header                                   
   ///   @param loader - loader for streaming                                 
   ///   @return the number of read/peek bytes from byte container            
   template<bool HEADER, CT::Block TO>
   Size Bytes::Deserialize(TO& to, const Header& header, Offset readOffset, const Loader& loader) const {
      using namespace Flow;
      using namespace Serial;

      Count deserializedCount = 0;
      auto read = readOffset;

      if constexpr (HEADER) {
         // Read the header - means we have unpredictable data          
         // The header contains instructions on how data was serialized 
         read = DeserializeAtom(deserializedCount, read, header, loader);

         // First read the serialized data state                        
         DataState deserializedState {};
         RequestMoreBytes(read, sizeof(DataState), loader);
         ::std::memcpy(static_cast<void*>(&deserializedState), At(read), sizeof(DataState));
         read += sizeof(DataState);
         to.AddState(deserializedState);

         // Finally, read type                                          
         DMeta deserializedType;
         read = DeserializeMeta(deserializedType, read, header, loader);

         if (not deserializedType)
            return read;

         to.template Mutate<false>(deserializedType);
      }
      else {
         // Don't read header - we have a predictable single element,   
         // like a member, a base, or a cast operator sequence          
         // In this case, result should already be allocated and known  
         LANGULUS_ASSERT(to.IsTyped() and to,
            Convert, "Bad serialization output block while deserializing ",
            GetToken(), " as ", to.GetToken()
         );

         deserializedCount = to.GetCount();
      }

      if (not deserializedCount)
         return read;

      // Fill memory                                                    
      const bool resolvable = to.IsResolvable();
      if (not resolvable) {
         if (to.IsPOD()) {
            // If data is POD, optimize by directly memcpying it        
            if constexpr (HEADER)
               to.template AllocateMore<false, true>(deserializedCount);

            const auto byteSize = to.GetBytesize();
            RequestMoreBytes(read, byteSize, loader);

            if (to.IsSparse()) {
               // Allocate a separate block for the elements            
               const auto temporary = Allocator::Allocate(nullptr, byteSize);
               auto start = temporary->GetBlockStart();
               ::std::memcpy(start, At(read), byteSize);
               read += byteSize;
               temporary->Keep(deserializedCount - 1);

               // Write a pointer to each element                       
               auto p = to.template GetHandle<Byte*>(0);
               const auto pEnd = p + to.GetCount();
               const auto size = to.GetType()->mSize;
               while (p != pEnd) {
                  p.New(start, temporary);
                  start += size;
               }
            }
            else {
               // Data is dense, parse it at once                       
               ::std::memcpy(to.GetRaw(), At(read), byteSize);
               read += byteSize;
            }

            return read;
         }
         else if (to.IsDeep()) {
            // If data is deep, nest each sub-block                     
            if constexpr (HEADER)
               to.New(deserializedCount);

            to.ForEach([&](Block& block) {
               read = Deserialize<true>(block, header, read, loader);
            });

            return read;
         }
         else if (to.template CastsTo<RTTI::Meta>()) {
            // Deserialize meta definitions                             
            // The resulting container should be always const & sparse  
            if constexpr (HEADER)
               to.New(deserializedCount);

            auto p = to.template GetHandle<Byte*>(0);
            const auto pEnd = p + to.GetCount();
            if (to.template IsExact<DMeta>()) {
               while (p != pEnd) {
                  DMeta ptr;
                  read = DeserializeMeta(ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaData*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (to.template IsExact<TMeta>()) {
               while (p != pEnd) {
                  TMeta ptr;
                  read = DeserializeMeta(ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaTrait*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (to.template IsExact<VMeta>()) {
               while (p != pEnd) {
                  VMeta ptr;
                  read = DeserializeMeta(ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaVerb*>(ptr)),
                     nullptr
                  );
               }
            }
            else if (to.template IsExact<CMeta>()) {
               while (p != pEnd) {
                  CMeta ptr;
                  read = DeserializeMeta(ptr, read, header, loader);
                  p.New(
                     reinterpret_cast<Byte*>(const_cast<MetaConst*>(ptr)),
                     nullptr
                  );
               }
            }
            else {
               LANGULUS_OOPS(
                  Convert, "Bad meta container while deserializing `",
                  GetToken(), "` as `", to.GetToken(), '`'
               );
            }

            return read;
         }
         else if (to.template CastsTo<Verb>()) {
            // If data is verb, deserialize it here                     
            if constexpr (HEADER)
               to.New(deserializedCount);

            to.ForEach([&](Verb& verb) {
               // Deserialize verb type                                 
               VMeta ptr;
               read = DeserializeMeta(ptr, read, header, loader);
               verb.SetVerb(ptr);

               // Deserialize the charge                                
               Charge charge;
               RequestMoreBytes(read, sizeof(Charge), loader);
               ::std::memcpy(static_cast<void*>(&charge), At(read), sizeof(Charge));
               read += sizeof(Charge);
               verb.SetCharge(charge);

               // Deserialize the verb state                            
               VerbState vstate;
               RequestMoreBytes(read, sizeof(VerbState), loader);
               ::std::memcpy(static_cast<void*>(&vstate), At(read), sizeof(VerbState));
               read += sizeof(VerbState);
               verb.SetVerbState(vstate);

               // Deserialize source                                    
               read = Deserialize<true>(verb.GetSource(), header, read, loader);

               // Deserialize argument                                  
               read = Deserialize<true>(verb.GetArgument(), header, read, loader);
            });

            return read;
         }
      }

      if (to.mType->mDefaultConstructor) {
         // Type is statically producible, and has default constructor, 
         // therefore we can deserialize it by making a default copy    
         // and then filling in the reflected members and bases         
         for (Count i = 0; i < deserializedCount; ++i) {
            auto resolvedType = to.GetType();
            if (resolvable)
               read = DeserializeMeta(resolvedType, read, header, loader);

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
               element = to.GetElementDense(i);
            }

            // Deserialize all reflected bases                          
            for (auto& base : element.GetType()->mBases) {
               if (base.mImposed or base.GetType()->mIsAbstract)
                  continue;

               auto baseBlock = element.GetBaseMemory(base);
               read = Deserialize<false>(baseBlock, header, read, loader);
            }

            // Deserialize all reflected members                        
            for (auto& member : element.GetType()->mMembers) {
               auto memberBlock = element.GetMember(member);
               read = Deserialize<false>(memberBlock, header, read, loader);
            }

            if constexpr (HEADER)
               to.InsertBlock(element);
         }

         return read;
      }

      // Failure if reached                                             
      LANGULUS_OOPS(
         Convert, "Can't deserialize `",
         GetToken(), "` as `", to.GetToken(), '`'
      );
      return 0;
   }

#endif

} // namespace Langulus::Anyness


namespace Langulus::Flow::Serial
{

   /// Convert a byte to hexadecimal string, and append it to text container  
   ///   @param from - the byte to convert to hexadecimal                     
   ///   @param to - the container to append to                               
   LANGULUS(INLINED)
   void ToHex(const Byte& from, Text& to) {
      if (from == 0) {
         // fmt writes single zero instead of two zeroes                
         const auto raw = to.Extend(2).GetRaw();
         *raw = *(raw + 1) = '0';
      }
      else fmt::format_to_n(to.Extend(2).GetRaw(), 2, "{:X}", from.mValue);
   }

   /// Check if a memory block needs a Code scope decorated                   
   ///   @param block - the memory block to check                             
   ///   @return true if a scope is required around the block                 
   LANGULUS(INLINED)
   bool NeedsScope(const Block& block) noexcept {
      return block.GetCount() > 1 || block.IsInvalid();
   }

   /// Add a separator                                                        
   ///   @param isOr - OR separator or not                                    
   ///   @return the text equivalent of the separator                         
   LANGULUS(INLINED)
   Text Separator(bool isOr) {
      return isOr 
         ? Verbs::Conjunct::CTTI_NegativeOperator 
         : Verbs::Conjunct::CTTI_PositiveOperator;
   }

   /// A snippet for serializing a reference to a meta object                 
   ///   @param from - the member block to serialize                          
   ///   @param to - [out] the serialized data                                
   ///   @param member - reflection data about the member                     
   template<CT::Meta META, CT::Text TO>
   LANGULUS(INLINED)
   void SerializeMeta(const Block& from, TO& to, const RTTI::Member* member) {
      auto meta = member->template As<META>(from.GetRaw());
      if (meta)   to += meta->GetToken();
      else        to += Decay<META>::DefaultToken;
   }

   /// A snippet for serializing a block of numbers                           
   ///   @param from - the number block to serialize                          
   ///   @param to - [out] the serialized data                                
   template<class T, CT::Text TO>
   void SerializeNumber(const Block& from, TO& to) {
      if (from.IsDense()) {
         // Significantly faster routine if dense                       
         auto data = from.template GetRawAs<T>();
         const auto dataEnd = from.template GetRawEndAs<T>();
         while (data != dataEnd) {
            if constexpr (CT::Same<T, Letter>) {
               if constexpr (CT::Signed<Letter>)
                  to += TO {static_cast<signed>(*data)};
               else
                  to += TO {static_cast<unsigned>(*data)};
            }
            else to += TO {*data};

            if (from.GetType()->mSuffix.size())
               to += from.GetType()->mSuffix;
            if (data < dataEnd - 1)
               to += Separator(from.IsOr());
            ++data;
         }
      }
      else {
         for (Offset i = 0; i < from.GetCount(); ++i) {
            if constexpr (CT::Same<T, Letter>) {
               if constexpr (CT::Signed<Letter>)
                  to += TO {static_cast<signed>(from.template As<T>(i))};
               else
                  to += TO {static_cast<unsigned>(from.template As<T>(i))};
            }
            else to += TO {from.template As<T>(i)};

            to += from.GetType()->mSuffix;
            if (i < from.GetCount() - 1)
               to += Separator(from.IsOr());
         }
      }
   }

   /// Serialize all reflected data members in all bases                      
   ///   @param from - the member block to serialize                          
   ///   @param to - the serialized data                                      
   template<CT::Text TO, CT::Block TO_ORIGINAL>
   void SerializeMembers(const Block& from, TO& to) {
      if (from.IsDeep()) {
         to += Serialize<TO, true, Block, TO_ORIGINAL>(from.template Get<Block>());
         return;
      }

      // Append a separator?                                            
      bool separate = false;

      // First we serialize all bases' members                          
      for (auto& base : from.GetType()->mBases) {
         if (base.GetType()->mSize > 0) {
            if (separate) {
               to += Verbs::Conjunct::CTTI_PositiveOperator;
               separate = false;
            }

            const auto initial = to.GetCount();
            SerializeMembers<TO, TO_ORIGINAL>(
               from.GetBaseMemory(base.GetType(), base), to
            );

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
            to += Serialize<TO, true, Block, TO_ORIGINAL>(from.GetMember(member));

         separate = true;
      }
   }

} // namespace Langulus::Flow::Serial


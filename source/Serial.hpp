///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Code.hpp"

namespace Langulus::Flow
{
   struct Serializer {
   public:
      template<CT::Block TO, bool HEADER = true, CT::Dense FROM>
      NOD() static TO Serialize(const FROM&);
      template<CT::Block TO, bool HEADER = true, CT::Sparse FROM>
      NOD() static TO Serialize(FROM);

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<CT::Block FROM>
         NOD() static Any Deserialize(const FROM&);
      #endif

   private:
      ///                                                                     
      ///   General Code/Debug serializer tools                               
      ///                                                                     
      NOD() static bool NeedsScope(const Block& block) noexcept;
      NOD() static Text Separator(bool isOr);

      template<bool ENSCOPE, CT::Text TO>
      NOD() static Count SerializeBlock(const Block&, TO&);

      template<CT::Number T, CT::Text TO>
      static void SerializeNumber(const Block&, TO&);

      template<CT::Meta META, CT::Text TO>
      static void SerializeMeta(const Block&, TO&, const RTTI::Member*);

      template<CT::Text TO>
      static void SerializeMembers(const Block&, TO&);


      ///                                                                     
      ///   General binary serializer tools                                   
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
      static void SerializeBlock(const Block&, Bytes&);

      using Loader = TFunctor<void(Bytes&, Size)>;

      static void RequestMoreBytes(const Bytes&, Offset, Size, const Loader&);

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         NOD() static Size DeserializeAtom(const Bytes&, Offset&, Offset, const Header&, const Loader&);

         template<bool HEADER>
         NOD() static Size DeserializeBlock(const Bytes&, Block&, Offset, const Header&, const Loader&);

         template<class META>
         NOD() static Size DeserializeMeta(const Bytes&, META const*&, Offset, const Header&, const Loader&);
      #endif
   };

} // namespace Langulus::Flow

#include "Serial.inl"
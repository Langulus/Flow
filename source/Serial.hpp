///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Code.hpp"


namespace Langulus::Flow
{

   ///                                                                        
   ///   Serializer interface                                                 
   ///                                                                        
   template<CT::Block TO, bool HEADER = true, CT::Dense FROM, CT::Block TO_ORIGINAL = TO>
   NOD() TO Serialize(const FROM&);
   template<CT::Block TO, bool HEADER = true, CT::Sparse FROM, CT::Block TO_ORIGINAL = TO>
   NOD() TO Serialize(FROM);

   #if LANGULUS_FEATURE(MANAGED_REFLECTION)
      template<CT::Block FROM>
      NOD() Any Deserialize(const FROM&);
   #endif

   namespace Serial
   {

      ///                                                                     
      ///   General Code/Debug serializer tools                               
      ///                                                                     
      void ToHex(const Byte&, Text&);
      NOD() bool NeedsScope(const Block&) noexcept;
      NOD() Text Separator(bool isOr);

      template<class T, CT::Text TO>
      void SerializeNumber(const Block&, TO&);

      template<CT::Meta META, CT::Text TO>
      void SerializeMeta(const Block&, TO&, const RTTI::Member*);

      template<CT::Text TO, CT::Block TO_ORIGINAL>
      void SerializeMembers(const Block&, TO&);

   } // namespace Langulus::Flow::Serial

} // namespace Langulus::Flow
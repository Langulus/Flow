///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"
#include <Anyness/TUnorderedSet.hpp>
#include <Anyness/TUnorderedMap.hpp>

namespace Langulus::Flow
{

   ///                                                                        
   ///   Normalized data container                                            
   ///                                                                        
   /// Turns messy descriptors into neatly and consistently ordered container 
   /// that is very fast on compare/search/insert/remove, but is a bit too    
   /// large to be used all over the place                                    
   ///                                                                        
   struct Normalized {
      // Verbs will always be ordered in the order they appear          
      // Their contents will be normalized all the way through          
      TAny<Verb> mVerbs;
      // Traits are ordered first by their trait type, then by their    
      // order of appearance. Duplicate trait types are allowed         
      // Trait contents are also normalized all the way through         
      TUnorderedMap<TMeta, TAny<Any>> mTraits;
      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TAny<Construct>> mConstructs;
      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // All sub-blocks are normalized all the way through              
      TUnorderedMap<DMeta, TAny<Any>> mAnythingElse;

      mutable Hash mHash;

   public:
      Normalized(const Normalized&) = default;
      Normalized(Normalized&&) noexcept = default;
      template<CT::Semantic S>
      Normalized(S&&);

      Normalized(const Any&);

      Normalized& operator = (const Normalized&) = default;
      Normalized& operator = (Normalized&&) noexcept = default;
      template<CT::Semantic S>
      Normalized& operator = (S&&);

      NOD() Hash GetHash() const;

      bool operator == (const Normalized&) const;

      void Merge(const Normalized&);

      template<CT::Trait T>
      const TAny<Trait>* GetTraits();
      template<CT::Trait T>
      const TAny<Trait>* GetTraits() const;

      template<CT::Trait T, CT::Data D>
      void SetDefaultTrait(D&&);

      template<CT::Trait T, CT::Data D>
      void OverwriteTrait(D&&);
   };

} // namespace Langulus::Flow

#include "Normalized.inl"
///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Normalized.hpp"

namespace Langulus::Flow
{

   /// Semantic initialization from another normalized descriptor             
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - the descriptor to use                                 
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Normalized::Normalized(S&& other)
      : mVerbs {S::Nest(other->mVerbs)}
      , mTraits {S::Nest(other->mTraits)}
      , mMetaDatas {S::Nest(other->mMetaDatas)}
      , mMetaTraits {S::Nest(other->mMetaTraits)}
      , mMetaConstants {S::Nest(other->mMetaConstants)}
      , mMetaVerbs {S::Nest(other->mMetaVerbs)}
      , mConstructs {S::Nest(other->mConstructs)}
      , mAnythingElse {S::Nest(other->mAnythingElse)}
      , mHash {other->mHash} {
      static_assert(CT::Exact<TypeOf<S>, Normalized>,
         "S type must be Normalized");

      // Reset remote hash if moving                                    
      if constexpr (S::Move)
         other->mHash = {};
   }

   /// Compile a descriptor, by removing Traits::Parent, and grouping elements
   /// in predictable ways, ensuring further comparisons are fast & orderless 
   ///   @param messy - the messy descriptor to normalize                     
   LANGULUS(INLINED)
   Normalized::Normalized(const Any& messy) {
      messy.ForEachDeep([this](const Any& group) {
         if (group.IsOr())
            TODO();

         if (group.ForEach(
            [this](const Verb& verb) {
               // Never modify verb sequences, but make sure their      
               // contents are normalized                               
               Verb normalizedVerb {verb.PartialCopy()};
               normalizedVerb.SetSource(Normalized {verb.GetSource()});
               normalizedVerb.SetArgument(Normalized {verb.GetArgument()});
               mVerbs << Abandon(normalizedVerb);
            },
            [this](const Trait& trait) {
               // Always skip parent traits                             
               if (trait.TraitIs<Traits::Parent>())
                  return;
               
               // Normalize trait contents and push sort it by its      
               // trait type                                            
               mTraits[trait.GetTrait()] << Trait::From(
                  trait.GetTrait(), Normalized {trait}
               );
            },
            [this](const MetaData* type) {
               mMetaDatas << type;
            },
            [this](const MetaTrait* type) {
               mMetaTraits << type;
            },
            [this](const MetaConst* type) {
               mMetaConstants << type;
            },
            [this](const MetaVerb* type) {
               mMetaVerbs << type;
            },
            [this](const Construct& construct) {
               // Normalize contents and push sort it by type           
               mConstructs[construct.GetType()] << Construct {
                  construct.GetType(), 
                  Normalized {construct}
               };
            }
         )) return;

         // If reached, just propagate the block without changing it    
         // But still sort it by block type                             
         mAnythingElse[group.GetType()] << group;
      });
   }

   /// Semantic assignment with another normalized descriptor                 
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param other - normalized descriptor to assign                       
   ///   @return a reference to this descriptor                               
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Normalized& Normalized::operator = (S&& other) {
      static_assert(CT::Exact<TypeOf<S>, Normalized>,
         "S type must be Normalized");

      mVerbs = S::Nest(other->mVerbs);
      mTraits = S::Nest(other->mTraits);
      mMetaDatas = S::Nest(other->mMetaDatas);
      mMetaTraits = S::Nest(other->mMetaTraits);
      mMetaConstants = S::Nest(other->mMetaConstants);
      mMetaVerbs = S::Nest(other->mMetaVerbs);
      mConstructs = S::Nest(other->mConstructs);
      mAnythingElse = S::Nest(other->mAnythingElse);
      mHash = other->mHash;

      // Reset remote hash if moving                                    
      if constexpr (S::Move)
         other->mHash = {};
      return *this;
   }

   /// Get the hash of a normalized descriptor (cached)                       
   ///   @return the hash                                                     
   LANGULUS(INLINED)
   Hash Normalized::GetHash() const {
      if (mHash)
         return mHash;

      // Cache hash so we don't recompute it all the time               
      mHash = HashOf(
         mVerbs,
         mTraits,
         mMetaDatas,
         mMetaTraits,
         mMetaConstants,
         mMetaVerbs,
         mConstructs,
         mAnythingElse
      );
      return mHash;
   }

   /// Compare normalized descriptors                                         
   ///   @param rhs - the container to compare with                           
   ///   @return true if descriptors match                                    
   LANGULUS(INLINED)
   bool Normalized::operator == (const Normalized& rhs) const {
      if (GetHash() != rhs.GetHash())
         return false;

      return mVerbs == rhs.mVerbs
         && mTraits == rhs.mTraits
         && mMetaDatas == rhs.mMetaDatas
         && mMetaTraits == rhs.mMetaTraits
         && mMetaConstants == rhs.mMetaConstants
         && mMetaVerbs == rhs.mMetaVerbs
         && mConstructs == rhs.mConstructs
         && mAnythingElse == rhs.mAnythingElse;
   }

} // namespace Langulus::Flow

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
               mTraits[trait.GetTrait()] << Normalized {trait};
            },
            [this](const MetaData* type) {
               // Insert an empty Construct to signify solo type ID     
               mConstructs[type] << Construct {type};
            },
            [this](const MetaTrait* type) {
               // Insert an empty Any to signify trait without content  
               mTraits[type] << Any {};
            },
            [this](const MetaConst* type) {
               // Expand the constant, then normalize, and merge it     
               Any wrapped = Block {
                  DataState::Constrained, 
                  type->mValueType, 1,
                  type->mPtrToValue, nullptr
               };

               // Clone it, so that we take authority over the data     
               Any cloned = Clone(wrapped);
               Merge(Normalized {cloned});
            },
            [this](const MetaVerb* type) {
               // Insert an empty verb to signify solo verb ID          
               mVerbs << Verb::FromMeta(type);
            },
            [this](const Construct& construct) {
               // Normalize contents and push sort it by type           
               mConstructs[construct.GetType()] << Construct {
                  construct.GetType(), Normalized {construct}
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
      mHash = HashOf(mVerbs, mTraits, mConstructs, mAnythingElse);
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
         && mConstructs == rhs.mConstructs
         && mAnythingElse == rhs.mAnythingElse;
   }

   /// Merge two normalized descriptors                                       
   ///   @param rhs - the descriptor to merge                                 
   LANGULUS(INLINED)
   void Normalized::Merge(const Normalized& rhs) {
      mVerbs += rhs.mVerbs;
      mTraits += rhs.mTraits;
      mConstructs += rhs.mConstructs;
      mAnythingElse += rhs.mAnythingElse;

      // Rehash                                                         
      mHash = HashOf(mVerbs, mTraits, mConstructs, mAnythingElse);
   }

   /// Get list of traits, corresponding to a type                            
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   const TAny<Any>* Normalized::GetTraits() {
      auto found = mTraits.FindKeyIndex(MetaTrait::Of<T>());
      if (!found)
         return nullptr;
      else return &mTraits.GetValue(found);
   }

   /// Get list of traits, corresponding to a type (const)                    
   ///   @tparam T - trait type to search for                                 
   ///   @return the trait list, or nullptr if no such list exists            
   ///   @attention the list can be empty, if trait was provided with no      
   ///              contents                                                  
   template<CT::Trait T>
   LANGULUS(INLINED)
   const TAny<Any>* Normalized::GetTraits() const {
      return const_cast<Normalized*>(this)->template GetTraits<T>();
   }
   
   /// Get list of data, corresponding to a type                              
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Any>* Normalized::GetData() {
      auto found = mAnythingElse.FindKeyIndex(MetaData::Of<T>());
      if (!found)
         return nullptr;
      else return &mAnythingElse.GetValue(found);
   }

   /// Get list of data, corresponding to a type (const)                      
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Any>* Normalized::GetData() const {
      return const_cast<Normalized*>(this)->template GetData<T>();
   }

   /// Set a default trait, if such wasn't already set                        
   ///   @tparam T - trait to set                                             
   ///   @tparam D - type of data to set it to (deducible)                    
   ///   @param value - the value to assign                                   
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   void Normalized::SetDefaultTrait(D&& value) {
      auto found = GetTraits<T>();
      if (found && !found->IsEmpty())
         return;
      *found = Forward<D>(value);
   }

   /// Overwrite trait, or add a new one, if not already set                  
   ///   @tparam T - trait to set                                             
   ///   @tparam D - type of data to set it to (deducible)                    
   ///   @param value - the value to assign                                   
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   void Normalized::OverwriteTrait(D&& value) {
      // Trait was found, overwrite it                                  
      auto meta = MetaTrait::Of<T>();
      mTraits[meta] = Forward<D>(value);
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam T - the trait we're searching for                            
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   bool Normalized::ExtractTrait(D&& value) {
      auto found = GetTraits<T>();
      if (found) {
         int reverseIdx = -1;
         const int endIdx = -static_cast<int>(found->GetCount());
         while (reverseIdx != endIdx) {
            try {
               value = (*found)[reverseIdx].AsCast<D>();
               return true;
            }
            catch (...) {}
         }
         --reverseIdx;
      }

      return false;
   }
   
   /// Extract data                                                           
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Data D>
   LANGULUS(INLINED)
   bool Normalized::ExtractData(D&& value) {
      auto found = GetData<D>();
      if (found) {
         value = found->Last().template Get<D>();
         return true;
      }

      return false;
   }

} // namespace Langulus::Flow

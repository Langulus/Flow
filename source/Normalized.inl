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
   /// Nested contents are normalized only if deep                            
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
               if (verb.GetSource().IsDeep())
                  normalizedVerb.SetSource(Normalized {verb.GetSource()});
               else
                  normalizedVerb.SetSource(verb.GetSource());

               if (verb.GetArgument().IsDeep())
                  normalizedVerb.SetArgument(Normalized {verb.GetArgument()});
               else
                  normalizedVerb.SetArgument(verb.GetArgument());

               mVerbs << Abandon(normalizedVerb);
            },
            [this](const Trait& trait) {
               // Always skip parent traits                             
               if (trait.TraitIs<Traits::Parent>())
                  return;
               
               // Normalize trait contents and push sort it by its      
               // trait type                                            
               if (trait.IsDeep())
                  mTraits[trait.GetTrait()] << Normalized {trait};
               else
                  mTraits[trait.GetTrait()] << static_cast<const Any&>(trait);
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
               Any wrapped = Block {{}, type};

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
               if (construct.IsDeep()) {
                  mConstructs[construct.GetType()] << Construct {
                     construct.GetType(), Normalized {construct}
                  };
               }
               else mConstructs[construct.GetType()] << construct;
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
   TAny<Any>* Normalized::GetTraits() {
      auto found = mTraits.Find(T::GetTrait());
      if (!found)
         return nullptr;
      
      return &mTraits.GetValue(found);
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
   TAny<Any>* Normalized::GetData() {
      auto found = mAnythingElse.Find(MetaOf<T>());
      if (!found)
         return nullptr;
      
      return &mAnythingElse.GetValue(found);
   }

   /// Get list of data, corresponding to a type (const)                      
   ///   @tparam T - type to search for                                       
   ///   @return the data list, or nullptr if no such list exists             
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Any>* Normalized::GetData() const {
      return const_cast<Normalized*>(this)->template GetData<T>();
   }

   /// Get list of constructs, corresponding to a type                        
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   TAny<Construct>* Normalized::GetConstructs() {
      auto found = mConstructs.Find(MetaOf<T>());
      if (!found)
         return nullptr;
      
      return &mConstructs.GetValue(found);
   }

   /// Get list of constructs, corresponding to a type (const)                
   ///   @tparam T - type to search for                                       
   ///   @return the construct list, or nullptr if no such list exists        
   template<CT::Data T>
   LANGULUS(INLINED)
   const TAny<Construct>* Normalized::GetConstructs() const {
      return const_cast<Normalized*>(this)->template GetConstructs<T>();
   }

   /// Set a default trait, if such wasn't already set                        
   ///   @tparam T - trait to set                                             
   ///   @tparam D - type of data to set it to (deducible)                    
   ///   @param value - the value to assign                                   
   template<CT::Trait T, CT::Data D>
   LANGULUS(INLINED)
   void Normalized::SetDefaultTrait(D&& value) {
      auto found = GetTraits<T>();
      if (found && *found)
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
      mTraits[T::GetTrait()] = Forward<D>(value);
   }

   /// Extract a trait from the descriptor                                    
   ///   @tparam T - the trait we're searching for                            
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param values - [out] where to save the value, if found              
   ///   @return true if value changed                                        
   template<CT::Trait T, CT::Data... D>
   LANGULUS(INLINED)
   bool Normalized::ExtractTrait(D&... values) {
      auto found = GetTraits<T>();
      if (found) {
         return ExtractTraitInner(
            *found,
            ::std::make_integer_sequence<Offset, sizeof...(D)> {},
            values...
         );
      }
      return false;
   }

   ///                                                                        
   template<CT::Data... D, Offset... IDX>
   bool Normalized::ExtractTraitInner(
      TAny<Any>& found, 
      ::std::integer_sequence<Offset, IDX...>, 
      D&... values
   ) {
      return (ExtractTraitInnerInner<IDX, D>(found, values) || ...);
   }
   
   ///                                                                        
   template<Offset IDX, CT::Data D>
   bool Normalized::ExtractTraitInnerInner(TAny<Any>& found, D& value) {
      if (IDX >= found.GetCount())
         return false;

      if constexpr (CT::Deep<D>) {
         value = found[IDX];
         return true;
      }
      else try {
         value = found[IDX].AsCast<D>();
         return true;
      }
      catch (...) {}
      return false;
   }
   
   /// Extract data of an exact type                                          
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Data D>
   LANGULUS(INLINED)
   bool Normalized::ExtractData(D& value) {
      auto found = GetData<D>();
      if (found) {
         value = found->Last().template Get<D>();
         return true;
      }

      return false;
   }
   
   /// Extract any data, convertible to D                                     
   ///   @tparam D - the type of the data we're extracting (deducible)        
   ///   @param value - [out] where to save the value, if found               
   ///   @return true if value changed                                        
   template<CT::Data D>
   LANGULUS(INLINED)
   bool Normalized::ExtractDataAs(D& value) {
      for (auto pair : mAnythingElse) {
         for (auto& group : pair.mValue) {
            try {
               value = group.AsCast<D>();
               return true;
            }
            catch (...) {}
         }
      }

      return false;
   }

} // namespace Langulus::Flow

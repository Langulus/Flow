///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TFactory.hpp"

#define TEMPLATE() template<class T, FactoryUsage USAGE>
#define FACTORY() TFactory<T, USAGE>

namespace Langulus::Flow
{

   /// The only allowed element constructor                                   
   ///   @param factory - the factory who owns the T instance                 
   ///   @param hash - precomputed descriptor hash (optimization)             
   ///   @param descriptor - the element descriptor, used for hashing         
   TEMPLATE()
   FACTORY()::Element::Element(TFactory* factory, Hash hash, const Any& descriptor)
      : mFactory {factory}
      , mHash {hash}
      , mDescriptor {descriptor}
      , mData {descriptor} {}

   /// Construction of a factory                                              
   ///   @param owner - the factory owner                                     
   TEMPLATE()
   FACTORY()::TFactory(Producer* owner)
      : mFactoryOwner {owner} {}

   /// Move-assignment remaps all elements to the new instance owner          
   ///   @attention notice how mFactoryOwner never changes on both sides      
   ///   @param other - the factory to move                                   
   TEMPLATE()
   FACTORY()& FACTORY()::operator = (TFactory&& other) noexcept {
      mData = Move(other.mData);
      mHashmap = Move(other.mHashmap);
      mReusable = other.mReusable;
      other.mReusable = nullptr;
      for (auto& item : mData)
         item.mFactory = this;
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE()
   void FACTORY()::Reset() {
      mData.Reset();
      mHashmap.Reset();
      mReusable = nullptr;
   }

   /// Find an element with the provided hash and descriptor                  
   ///   @param hash - precomputed descriptor hash (optimization)             
   ///   @param descriptor - the full descriptor for the element              
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE()
   typename FACTORY()::Element* FACTORY()::Find(Hash hash, const Any& descriptor) const {
      const auto found = mHashmap.FindKeyIndex(hash);
      if (found) {
         for (auto candidate : mHashmap.GetValue(found)) {
            if (candidate->mDescriptor != descriptor) //TODO orderless comparison instead
               continue;

            // Found                                                    
            return candidate;
         }
      }
      
      // Not found                                                      
      return nullptr;
   }


   /// Create/Destroy element(s) inside the factory                           
   ///   @param verb - the creation verb                                      
   ///   @param arguments... - additional arguments for element constructor,  
   ///                         usually provided by factory owner              
   TEMPLATE()
   void FACTORY()::Create(Verb& verb) {
      verb.ForEachDeep(
         [&](const Construct& construct) {
            // For each construct...                                    
            if (!construct.GetType()->template HasDerivation<T>())
               return;
            
            auto count = static_cast<int>(
               ::std::floor(construct.GetCharge().mMass * verb.GetMass())
            );
            CreateInner(verb, count, construct);
         },
         [&](const MetaData* type) {
            // For each type...                                         
            if (!type || !type->template HasDerivation<T>())
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );
            const Any descriptor {};
            CreateInner(verb, count, descriptor);
         }
      );
   }
   
   /// Inner creation/destruction verb                                        
   ///   @param verb - [in/out] the creation/destruction verb                 
   ///   @param count - the number of items to create (or destroy if negative)
   ///   @param descriptor - the element descriptor                           
   ///   @param arguments... - additional arguments for element construction  
   TEMPLATE()
   void FACTORY()::CreateInner(Verb& verb, int count, const Any& descriptor) {
      const auto hash = descriptor.GetHash();
      if (count > 0) {
         // Produce amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(hash, descriptor);
            if (found) {
               // The unique construct was found, just return it.       
               // Mass will be ignored, it makes no sense to            
               // create multiple instances if unique                   
               verb << &found->mData;
               return;
            }

            // If reached, nothing was found                            
            // Produce exactly one element with this descriptor         
            // Mass will be ignored, it makes no sense to create        
            // multiple instances if unique                             
            verb << Produce(hash, descriptor);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               verb << Produce(hash, descriptor);
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(hash, descriptor);
            if (found) {
               // The unique construct was found, destroy it            
               // Mass is ignored, there should be exactly one          
               Destroy(found);
               return;
            }
         }
         else {
            // Destroy the required amount of matching items            
            do {
               const auto found = Find(hash, descriptor);
               if (!found)
                  break;
               Destroy(found);
               ++count;
            }
            while (count < 0);
         }

         verb.Done();
      }
   }

   /// Select/Deselect element(s) inside the factory                          
   ///   @param verb - the selection verb                                     
   TEMPLATE()
   void FACTORY()::Select(Verb& verb) {
      // For each construct or meta compatible with the factory         
      verb.ForEachDeep(
         [&](const Construct& construct) {
            if (!construct.GetType()->template HasDerivation<T>())
               return;
         },
         [&](const MetaData* type) {
            if (!type || !type->template HasDerivation<T>())
               return;
         }
      );
   }

   /// Produce a single T with the given descriptor and arguments             
   ///   @param hash - precomputed descriptor hash (optimization)             
   ///   @param descriptor - the element descriptor                           
   ///   @param arguments... - additional arguments for element constructor,  
   ///                         usually provided by factory owner              
   TEMPLATE()
   T* FACTORY()::Produce(Hash hash, const Any& descriptor) {
      if (mReusable) {
         // Reuse a slot                                                
         auto memory = mReusable;
         mReusable = mReusable->mNextFreeElement;
         auto result = new (memory) Element {this, hash, descriptor};
         mHashmap[hash] << result;
         return &result->mData;
      }

      // If this is reached, then a reallocation is required            
      mData.Emplace(this, hash, descriptor);
      mHashmap[hash] << &mData.Last();
      return &mData.Last().mData;
   }

   /// Destroys an element inside factory                                     
   ///   @attention assumes item is a valid pointer, owned by the factory     
   ///   @attention item pointer is no longer valid after this call           
   ///   @param item - element to destroy                                     
   TEMPLATE()
   void FACTORY()::Destroy(Element* item) {
      LANGULUS_ASSUME(DevAssumes, item != nullptr,
         "Pointer is not valid");
      LANGULUS_ASSUME(DevAssumes, mData.Owns(item),
         "Pointer is not owned by factory");

      // Remove from hashmap                                            
      auto& list = mHashmap[item->mHash];
      list.template RemoveValue<false, true>(item);
      if (list.IsEmpty())
         mHashmap.RemoveKey(item->mHash);

      // Destroy the element                                            
      item->~Element();
      item->mNextFreeElement = mReusable;
      mReusable = item;
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE()
   typename FACTORY()::Iterator FACTORY()::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE()
   typename FACTORY()::Iterator FACTORY()::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE()
   typename FACTORY()::Iterator FACTORY()::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE()
   typename FACTORY()::ConstIterator FACTORY()::begin() const noexcept {
      if (mData.IsEmpty())
         return end();

      // Seek first valid slot, or hit sentinel at the end              
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();
      while (raw != rawEnd && 0 == raw->mData.GetReferences())
         ++raw;

      return {raw, rawEnd};
   }

   /// Get iterator to end                                                    
   ///   @return a constant iterator to the end element                       
   TEMPLATE()
   typename FACTORY()::ConstIterator FACTORY()::end() const noexcept {
      const auto ender = mData.GetRawEnd();
      return {ender, ender};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE()
   typename FACTORY()::ConstIterator FACTORY()::last() const noexcept {
      if (mData.IsEmpty())
         return end();

      // Seek first valid slot, or hit sentinel at the end              
      auto raw = mData.GetRawEnd() - 1;
      const auto rawEnd = mData.GetRaw() - 1;
      while (raw != rawEnd && 0 == raw->mData.GetReferences())
         --raw;

      return {raw != rawEnd ? raw : mData.GetRawEnd()};
   }

   

   
   ///                                                                        
   ///   TFactory iterator                                                    
   ///                                                                        
   #define ITERATOR() TFactory<T, USAGE>::template TIterator<MUTABLE>

   /// Construct an iterator                                                  
   ///   @param element - the current element                                 
   ///   @param sentinel - the sentinel (equivalent to factory::end())        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   TFactory<T, USAGE>::TIterator<MUTABLE>::TIterator(const Element* element, const Element* sentinel) noexcept
      : mElement {element}
      , mSentinel {sentinel} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename ITERATOR()& TFactory<T, USAGE>::TIterator<MUTABLE>::operator ++ () noexcept {
      ++mElement;
      // Skip all invalid entries, until a valid one/sentinel is hit    
      while (mElement != mSentinel && 0 == mElement->mData.GetReferences())
         ++mElement;
      return *this;
   }

   /// Suffix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the previous value of the iterator                           
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   typename ITERATOR() TFactory<T, USAGE>::TIterator<MUTABLE>::operator ++ (int) noexcept {
      const auto backup = *this;
      operator ++ ();
      return backup;
   }

   /// Compare iterators                                                      
   ///   @param rhs - the other iterator                                      
   ///   @return true if entries match                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   bool TFactory<T, USAGE>::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mElement == rhs.mElement;
   }
      
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(ALWAYSINLINE)
   const T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef FACTORY
#undef ITERATOR
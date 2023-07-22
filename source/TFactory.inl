///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "TFactory.hpp"
#include "verbs/Interpret.inl"

#define TEMPLATE()   template<class T, FactoryUsage USAGE>
#define FACTORY()    TFactory<T, USAGE>
#define ITERATOR()   TFactory<T, USAGE>::template TIterator<MUTABLE>

namespace Langulus::Flow
{

   /// Move a produced item                                                   
   ///   @param other - the item to move                                      
   template<class T>
   ProducedFrom<T>::ProducedFrom(ProducedFrom&& other)
      // mProducer intentionally not overwritten                        
      : mDescriptor {Move(other.mDescriptor)} {}

   /// Construct a produced item                                              
   ///   @param producer - the item's producer                                
   ///   @param descriptor - the item's messy descriptor                      
   template<class T>
   ProducedFrom<T>::ProducedFrom(T* producer, const Any& descriptor)
      : mDescriptor {descriptor}
      , mProducer {producer} {
      LANGULUS_ASSUME(DevAssumes, producer != nullptr, "Invalid producer");
   }

   /// Get the normalized descriptor of the produced item                     
   ///   @return the normalized descriptor                                    
   template<class T>
   const Normalized& ProducedFrom<T>::GetDescriptor() const noexcept {
      return mDescriptor;
   }

   /// Get the hash of the normalized descriptor (cached and efficient)       
   ///   @return the hash                                                     
   template<class T>
   Hash ProducedFrom<T>::GetHash() const noexcept {
      return mDescriptor.GetHash();
   }

   /// Return the producer of the item (a.k.a. the owner of the factory)      
   ///   @return a pointer to the producer instance                           
   template<class T>
   T* ProducedFrom<T>::GetProducer() const noexcept {
      return mProducer;
   }


   /// Constructor for descriptor-constructible element                       
   ///   @param factory - the factory who owns the T instance                 
   ///   @param descriptor - the messy element descriptor, used               
   ///                       to construct the element                         
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::Element::Element(TFactory* factory, const Any& descriptor)
      : mFactory {factory}
      , mData {factory->mFactoryOwner, descriptor} {}
   
   /// Destructor for descriptor-constructible element                        
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::Element::~Element() {
      /*#ifdef LANGULUS_ENABLE_SAFE_MODE
         if (mData.GetReferences() != 1) {
            Logger::Error("Unable to destroy ", mData,
               ", it has ", mData.GetReferences(), " uses instead of 1"
            );
            LANGULUS_THROW(Destruct, "Unable to destroy factory element");
         }
      #endif*/
   }


   /// Construction of a factory                                              
   ///   @param owner - the factory owner                                     
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::TFactory(Producer* owner)
      : mFactoryOwner {owner} {}

   /// Factory destructor                                                     
   /// Checks if all elements are referenced exactly once before destruction  
   /// if safe mode is enabled                                                
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::~TFactory() SAFETY_NOEXCEPT() {
      Reset();
   }

   /// Move-assignment remaps all elements to the new instance owner          
   ///   @attention notice how mFactoryOwner never changes on both sides      
   ///   @param other - the factory to move                                   
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()& FACTORY()::operator = (TFactory&& other) noexcept {
      mData = Move(other.mData);
      mHashmap = Move(other.mHashmap);
      mReusable = other.mReusable;
      mCount = other.mCount;
      other.mCount = 0;
      other.mReusable = nullptr;
      for (auto& item : mData)
         item.mFactory = this;
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   void FACTORY()::Reset() {
      if (!mData.IsAllocated())
         return;

      mHashmap.Reset();

      // Destroy only elements that have a single reference             
      // Some of the elements might be used in other modules, and their 
      // destruction, as well as the destruction of the mData.mEntry    
      // will commence automatically after their use have ceased.       
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();
      //Count count = 0;
      while (raw != rawEnd) {
         if (raw->mData.GetReferences() != 1) {
            /*Logger::Error("Unable to destroy ", raw->mData, ", it has ",
               raw->mData.GetReferences(), " uses instead of 1");
            LANGULUS_THROW(Destruct, "Unable to destroy factory");*/
            continue;
         }

         // Destroy the element                                         
         raw->~Element();

         //++count;
         ++raw;
      }

      /*LANGULUS_ASSUME(DevAssumes, count == mCount, 
         "Factory element(s) unaccounted for");
      LANGULUS_ASSUME(DevAssumes, mData.GetUses() == 1, 
         "Factory bank has ", mData.GetUses(), " uses instead of 1");*/

      // Make sure no more destructors are called upon Any::Reset()     
      mData.mCount = 0;
      mData.Reset();
      mReusable = nullptr;
      mCount = 0;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   bool FACTORY()::IsEmpty() const noexcept {
      return mCount == 0;
   }

   /// Explicit bool cast operator, for use in if statements                  
   ///   @return true if block contains at least one valid element            
   TEMPLATE() LANGULUS(INLINED)
   constexpr FACTORY()::operator bool() const noexcept {
      return !IsEmpty();
   }

#ifdef LANGULUS_ENABLE_SAFE_MODE
   /// Dump the factory to the log                                            
   TEMPLATE()
   void FACTORY()::Dump() const {
      const auto scope = Logger::Special("--------- FACTORY DUMP FOR ", 
         MetaOf<FACTORY()>(), " (", mData.GetUses(), " references): ",
         Logger::Tabs {}
      );

      Count counter {};
      auto raw = mData.GetRaw();
      const auto rawEnd = mData.GetRawEnd();
      while (raw != rawEnd) {
         if (!raw->mData.GetReferences())
            continue;

         Logger::Info(counter, "] ", raw->mData,
            ", ", raw->mData.GetReferences(), " references");
         ++raw;
      }
   }
#endif

   /// Find an element with the provided hash and descriptor                  
   ///   @param descriptor - the normalized descriptor for the element        
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Element* FACTORY()::Find(const Normalized& descriptor) const {
      const auto hash = descriptor.GetHash();
      const auto found = mHashmap.Find(hash);
      if (found) {
         for (auto candidate : mHashmap.GetValue(found)) {
            if (candidate->mData.GetDescriptor() != descriptor)
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
   TEMPLATE()
   void FACTORY()::Create(Verb& verb) {
      verb.ForEachDeep(
         [&](const Construct& construct) {
            // For each construct...                                    
            if (!MetaOf<T>()->CastsTo(construct.GetType()))
               return;
            
            auto count = static_cast<int>(
               ::std::floor(construct.GetCharge().mMass * verb.GetMass())
            );

            try { CreateInner(verb, count, construct); }
            catch (const Exception& e) {
               Logger::Error("Unable to create ", construct, " due to exception: ", e);
               return;
            }
         },
         [&](const MetaData* type) {
            // For each type...                                         
            if (!type || !MetaOf<T>()->CastsTo(type))
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );

            try { CreateInner(verb, count); }
            catch (const Exception& e) {
               Logger::Error("Unable to create ", type, " due to exception: ", e);
               return;
            }
         }
      );
   }

   /// Inner creation/destruction verb                                        
   ///   @param verb - [in/out] the creation/destruction verb                 
   ///   @param count - the number of items to create (or destroy if negative)
   ///   @param messyDescriptor - uncompiled messy element descriptor         
   TEMPLATE()
   void FACTORY()::CreateInner(Verb& verb, int count, const Any& messyDescriptor) {
      Normalized descriptor {messyDescriptor};
      if (count > 0) {
         // Produce amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(descriptor);
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
            verb << Produce(messyDescriptor);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               auto produced = Produce(messyDescriptor);
               verb << produced;
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(descriptor);
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
               const auto found = Find(descriptor);
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
            // For each construct...                                    
            if (!MetaOf<T>()->CastsTo(construct.GetType()))
               return;

            TODO();
         },
         [&](const MetaData* type) {
            // For each type...                                         
            if (!type || !MetaOf<T>()->CastsTo(type))
               return;

            TODO();
         }
      );
   }

   /// Produce a single T with the given descriptor and arguments             
   ///   @param descriptor - the original, messy element descriptor           
   ///   @return the produced instance                                        
   TEMPLATE()
   T* FACTORY()::Produce(const Any& descriptor) {
      if (mReusable) {
         // Reuse a slot                                                
         auto memory = mReusable;
         mReusable = mReusable->mNextFreeElement;
         auto result = new (memory) Element {this, descriptor};
         mHashmap[result->mData.GetHash()] << result;
         ++mCount;
         return &result->mData;
      }
      else {
         // If this is reached, then a reallocation is required         
         mData.Emplace(this, descriptor);
         auto result = &mData.Last();
         mHashmap[result->mData.GetHash()] << result;
         ++mCount;
         return &result->mData;
      }
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
      const auto hash = item->mData.GetHash();
      auto& list = mHashmap[hash];
      list.Remove(item);
      if (!list)
         mHashmap.RemoveKey(hash);

      // Destroy the element                                            
      item->~Element();
      item->mNextFreeElement = mReusable;
      mReusable = item;
      --mCount;
   }


   ///                                                                        
   ///   Iteration                                                            
   ///                                                                        

   /// Get iterator to first element                                          
   ///   @return an iterator to the first element, or end if empty            
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::begin() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->begin();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to end                                                    
   ///   @return an iterator to the end element                               
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::end() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->end();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to the last element                                       
   ///   @return an iterator to the last element, or end if empty             
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Iterator FACTORY()::last() noexcept {
      static_assert(sizeof(Iterator) == sizeof(ConstIterator),
         "Size mismatch - types must be binary-compatible");
      const auto constant = const_cast<const FACTORY()*>(this)->last();
      return reinterpret_cast<const Iterator&>(constant);
   }

   /// Get iterator to first element                                          
   ///   @return a constant iterator to the first element, or end if empty    
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::begin() const noexcept {
      if (IsEmpty())
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
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::end() const noexcept {
      const auto ender = mData.GetRawEnd();
      return {ender, ender};
   }

   /// Get iterator to the last valid element                                 
   ///   @return a constant iterator to the last element, or end if empty     
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::ConstIterator FACTORY()::last() const noexcept {
      if (IsEmpty())
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

   /// Construct an iterator                                                  
   ///   @param element - the current element                                 
   ///   @param sentinel - the sentinel (equivalent to factory::end())        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   TFactory<T, USAGE>::TIterator<MUTABLE>::TIterator(const Element* element, const Element* sentinel) noexcept
      : mElement {element}
      , mSentinel {sentinel} {}

   /// Prefix increment operator                                              
   ///   @attention assumes iterator points to a valid element                
   ///   @return the modified iterator                                        
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
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
   LANGULUS(INLINED)
   bool TFactory<T, USAGE>::TIterator<MUTABLE>::operator == (const TIterator& rhs) const noexcept {
      return mElement == rhs.mElement;
   }
      
   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator * () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   const T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator * () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator -> () const noexcept requires (MUTABLE) {
      return const_cast<T&>(mElement->mData);
   }

   /// Iterator access operator                                               
   ///   @return a reference to the element at the current iterator position  
   TEMPLATE()
   template<bool MUTABLE>
   LANGULUS(INLINED)
   const T& TFactory<T, USAGE>::TIterator<MUTABLE>::operator -> () const noexcept requires (!MUTABLE) {
      return mElement->mData;
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef FACTORY
#undef ITERATOR
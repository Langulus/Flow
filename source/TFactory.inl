///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "TFactory.hpp"

#define TEMPLATE()   template<class T, FactoryUsage USAGE>
#define FACTORY()    TFactory<T, USAGE>

#if 0
   #define VERBOSE_FACTORY(...) Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_FACTORY(...) LANGULUS(NOOP)
#endif


namespace Langulus::Flow
{

   /// Factory destructor                                                     
   /// Checks if all elements are referenced exactly once before destruction  
   /// if safe mode is enabled                                                
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()::~TFactory() {
      static_assert(CT::Producible<T>, "T must have a producer");
      Reset();
   }

   /// Move-assignment remaps all elements to the new instance owner          
   ///   @attention notice how mFactoryOwner never changes on both sides      
   ///   @param other - the factory to move                                   
   TEMPLATE() LANGULUS(INLINED)
   FACTORY()& FACTORY()::operator = (TFactory&& other) noexcept {
      Base::operator = (Forward<Base>(other));
      mHashmap = Move(other.mHashmap);
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE() LANGULUS(INLINED)
   void FACTORY()::Reset() {
      mHashmap.Reset();
      Base::Reset();
   }

#if LANGULUS(SAFE)
   /// Dump the factory to the log                                            
   TEMPLATE()
   void FACTORY()::Dump() const {
      const auto scope = Logger::SpecialTab("--------- FACTORY DUMP FOR ", 
         MetaDataOf<TFactory>(), " (", Base::mCount, " of ", Base::mReserved,
         " cells used in ", Base::mFrames.GetCount(), " frames): "
      );

      Count counter = 0;
      ForEach([&](const T& item) {
         Logger::Info(counter, "] ", item, ", ",
            item.GetReferences(), " references");
      });
   }
#endif

   /// Find an element with the provided descriptor                           
   ///   @param descriptor - the normalized descriptor for the element        
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE() LANGULUS(INLINED)
   typename FACTORY()::Cell* FACTORY()::FindInner(Describe descriptor) const {
      VERBOSE_FACTORY(NameOf<FACTORY()>(), " seeking for ", descriptor);
      const auto hash = descriptor->GetHash();
      const auto found = mHashmap.FindIt(hash);
      if (found) {
         for (auto cell : found.GetValue()) {
            if (cell->mData.GetDescriptor() != descriptor)
               continue;
            return cell;
         }
      }
      
      return nullptr;
   }

   /// Create/Destroy element(s) inside the factory                           
   ///   @param producer - the producer                                       
   ///   @param verb - the creation verb                                      
   TEMPLATE()
   void FACTORY()::Create(auto* producer, Verb& verb) {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      verb.ForEachDeep(
         [&](const Construct& construct) {
            // For each construct...                                    
            if (not MetaOf<T>()->CastsTo(construct.GetType()))
               return;
            
            auto count = static_cast<int>(
               ::std::floor(construct.GetCharge().mMass * verb.GetMass())
            );

            CreateInner(producer, verb, count, construct.GetDescriptor());
         },
         [&](const DMeta& type) {
            // For each type...                                         
            if (not type or not MetaOf<T>()->CastsTo(type))
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );

            CreateInner(producer, verb, count);
         }
      );
   }
   
   /// Create (or reuse) a single element                                     
   ///   @param producer - the producer                                       
   ///   @param descriptor - the descriptor                                   
   ///   @return the new (or reused) instance                                 
   TEMPLATE()
   auto FACTORY()::CreateOne(auto* producer, Describe descriptor) -> T* {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      // Produce amount of compatible constructs                        
      if constexpr (IsUnique) {
         // Check if descriptor matches any of the available            
         const auto found = FindInner(descriptor);
         if (found)
            return &found->mData;
      }

      // If reached, nothing was found                                  
      // Produce exactly one element with this descriptor               
      return Produce(producer, descriptor);
   }

   /// Inner creation/destruction verb                                        
   ///   @param producer - the producer                                       
   ///   @param verb - [in/out] the creation/destruction verb                 
   ///   @param count - the number of items to create (or destroy if negative)
   ///   @param descriptor - element descriptor                               
   TEMPLATE()
   void FACTORY()::CreateInner(
      auto* producer, Verb& verb, int count, Describe descriptor
   ) {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      if (count > 0) {
         // Produce amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = FindInner(descriptor);
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
            verb << Produce(producer, descriptor);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               auto produced = Produce(producer, descriptor);
               verb << produced;
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = FindInner(descriptor);
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
               const auto found = FindInner(descriptor);
               if (not found)
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
            if (not MetaDataOf<T>()->CastsTo(construct.GetType()))
               return;

            TODO();
         },
         [&](const DMeta& type) {
            // For each type...                                         
            if (not type or not MetaDataOf<T>()->CastsTo(type))
               return;

            TODO();
         }
      );
   }

   /// External interface for finding an entry in the factory                 
   ///   @param descriptor - descriptor to match exactly                      
   ///   @return a valid pointer if element was found                         
   TEMPLATE()
   auto FACTORY()::Find(Describe descriptor) const -> const T* {
      const auto found = FindInner(descriptor);
      if (found)
         return &found->mData;
      return nullptr;
   }

   /// Produce a single T with the given descriptor                           
   ///   @param producer - the producer                                       
   ///   @param descriptor - element descriptor                               
   ///   @return the produced instance                                        
   TEMPLATE()
   T* FACTORY()::Produce(auto* producer, Describe descriptor) {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      // Register entry in the hashmap, for fast search by descriptor   
      VERBOSE_FACTORY(NameOf<FACTORY()>(), " producing: ", descriptor);
      auto result = Base::NewInner(producer, descriptor);
      if (not result)
         return nullptr;

      const auto hash = result->mData.GetHash();
      const auto found = mHashmap.FindIt(hash);
      if (found)
         found.GetValue() << result;
      else
         mHashmap.Insert(hash, result);

      return &result->mData;
   }

   /// Destroys an element inside factory                                     
   ///   @attention assumes item is a valid pointer, owned by the factory     
   ///   @attention item pointer is no longer valid after this call           
   ///   @param item - element to destroy                                     
   TEMPLATE()
   void FACTORY()::Destroy(Cell* item) {
      // Remove from hashmap                                            
      const auto hash = item->mData.GetHash();
      auto& list = mHashmap[hash];
      list.Remove(item);
      if (not list)
         mHashmap.RemoveKey(hash);

      // Destroy the element                                            
      Base::Destroy(item);
   }
   



   /// Move a produced item                                                   
   ///   @param other - the item to move                                      
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(ProducedFrom&& other)
      : ProducedFrom {Move(other)} {}

   /// Generic construction                                                   
   ///   @param other - intent and element to initialize with                 
   template<class T> template<template<class> class S> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(S<ProducedFrom>&& other)
   requires CT::Intent<S<ProducedFrom>>
      // mProducer intentionally not overwritten                        
      : mDescriptor {other.Nest(other->mDescriptor)} {}

   /// Construct a produced item                                              
   ///   @param producer - the item's producer                                
   ///   @param descriptor - the item's neat descriptor                       
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(T* producer, Describe descriptor)
      : mDescriptor {descriptor}
      , mProducer   {producer} {}

   /// Reset the descriptor to remove circular dependencies                   
   template<class T> LANGULUS(INLINED)
   void ProducedFrom<T>::Detach() {
      return mDescriptor.Reset();
   }
   
   /// Get the normalized descriptor of the produced item                     
   ///   @return the normalized descriptor                                    
   template<class T> LANGULUS(INLINED)
   auto ProducedFrom<T>::GetDescriptor() const noexcept -> const Many& {
      return mDescriptor;
   }

   /// Get the hash of the normalized descriptor (cached and efficient)       
   ///   @return the hash                                                     
   template<class T> LANGULUS(INLINED)
   Hash ProducedFrom<T>::GetHash() const noexcept {
      return mDescriptor.GetHash();
   }

   /// Return the producer of the item (a.k.a. the owner of the factory)      
   ///   @return a pointer to the producer instance                           
   template<class T> LANGULUS(INLINED)
   auto ProducedFrom<T>::GetProducer() const noexcept -> T* {
      return mProducer;
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef FACTORY

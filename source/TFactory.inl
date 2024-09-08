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
   typename FACTORY()::Cell* FACTORY()::Find(const Neat& descriptor) const {
      VERBOSE_FACTORY(NameOf<FACTORY()>(), " seeking for ", descriptor);
      const auto hash = descriptor.GetHash();
      const auto found = mHashmap.FindIt(hash);
      if (found) {
         for (auto cell : found.GetValue()) {
            if (cell->mData.GetNeat() != descriptor)
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

   /// Inner creation/destruction verb                                        
   ///   @param producer - the producer                                       
   ///   @param verb - [in/out] the creation/destruction verb                 
   ///   @param count - the number of items to create (or destroy if negative)
   ///   @param neat - element descriptor                                     
   TEMPLATE()
   void FACTORY()::CreateInner(auto* producer, Verb& verb, int count, const Neat& neat) {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      if (count > 0) {
         // Produce amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(neat);
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
            verb << Produce(producer, neat);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               auto produced = Produce(producer, neat);
               verb << produced;
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible constructs                     
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = Find(neat);
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
               const auto found = Find(neat);
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

   /// Produce a single T with the given descriptor                           
   ///   @param producer - the producer                                       
   ///   @param neat - element descriptor                                     
   ///   @return the produced instance                                        
   TEMPLATE()
   T* FACTORY()::Produce(auto* producer, const Neat& neat) {
      static_assert(CT::Related<ProducerOf<T>, decltype(producer)>,
         "Producer isn't related to the reflected one");

      // Register entry in the hashmap, for fast search by descriptor   
      VERBOSE_FACTORY(NameOf<FACTORY()>(), " producing: ", neat);
      auto result = Base::NewInner(producer, neat);
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
   template<class T> template<template<class> class S>
   requires CT::Intent<S<Neat>> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(S<ProducedFrom<T>>&& other)
      // mProducer intentionally not overwritten                        
      : mDescriptor {S<Neat> {other->mDescriptor}} {}

   /// Construct a produced item                                              
   ///   @param producer - the item's producer                                
   ///   @param neat - the item's neat descriptor                             
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(T* producer, const Neat& neat)
      : mDescriptor {neat}
      , mProducer   {producer} {}

   /// Reset the descriptor to remove circular dependencies                   
   template<class T> LANGULUS(INLINED)
   void ProducedFrom<T>::Detach() {
      return mDescriptor.Reset();
   }
   
   /// Get the normalized descriptor of the produced item                     
   ///   @return the normalized descriptor                                    
   template<class T> LANGULUS(INLINED)
   const Neat& ProducedFrom<T>::GetNeat() const noexcept {
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
   T* ProducedFrom<T>::GetProducer() const noexcept {
      return mProducer;
   }

} // namespace Langulus::Flow

#undef TEMPLATE
#undef FACTORY

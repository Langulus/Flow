///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Factory.hpp"
#include <Langulus/Recipe.hpp>
#include <Langulus/CT/Derived.hpp>
#include <vector>
#include <ranges>

#if 0
   #define VERBOSE_FACTORY(...) Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_FACTORY(...) LANGULUS(NOOP)
#endif


namespace Langulus::Flow
{
   /// Factory destructor.                                                    
   /// Checks if all elements are referenced exactly once before destruction  
   /// if safe mode is enabled.                                               
   TEMPLATE()
   FACTORY()::~TFactory() {
      static_assert(CT::Producible<T>, "T must have a producer");

      // First stage destruction that severs all connections            
      // If TFactory was on the stack, this will be the first time it's 
      // called. If it was on the managed heap, it would've been called 
      // from the Reference routine                                     
      Teardown();
   }

   /// Move-assignment remaps all elements to the new instance owner          
   ///   @attention notice how mFactoryOwner never changes on both sides      
   ///   @param other the factory to move                                     
   TEMPLATE()
   auto FACTORY()::operator = (TFactory&& other) noexcept -> TFactory& {
      Base::operator = (::std::forward<Base>(other));
      mIndexedByDescriptor = Move(other.mIndexedByDescriptor);
      return *this;
   }

   /// Reset the factory                                                      
   TEMPLATE()
   void FACTORY()::Reset() {
      mIndexedByDescriptor.Reset();
      Base::Reset();
   }
   
   ///   @attention                                                           
   /// Factories can have their own hierarchies going on, and tearing them    
   /// down to account of circular references is mandatory. Here's an example:
   /// 1. The Physics module has a Euclidean::World factory.                  
   /// 2. Each Euclidean::World has a factory for Euclidean::Instances.       
   /// 3. Each Instance refers to its World producer.                         
   /// 4. In order to destroy a World, you have to first visit all Instances  
   ///    and sever their connection with their World, so that the proper     
   ///    order of destruction happens later. Otherwise the World will be     
   ///    destroyed from the last Instance::mProducer handle instead of on    
   ///    World factory destruction.                                          
   //TODO is this really true though? The world factory should hold the last references. 
   //TODO maybe this was done to cope with bad consistency?
   //TODO maybe make a weak pointer variant of TRef and use that as mProducer? just use TOwn?
   //TODO but then descriptors are deep containers that can reference whatever they want... can't remedy that with weak pointers...
   TEMPLATE()
   void FACTORY()::Teardown() {
      mIndexedByDescriptor.Reset();

      if constexpr (requires (T item) { item.Teardown(); }) {
         for (T& item : *this)
            item.Teardown();
      }
   }

   /// Find element(s) with the provided descriptor                           
   ///   @param descriptor the normalized descriptor for the element          
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE()
   auto FACTORY()::FindByDescription(Neat const& descriptor) const -> Query {
      VERBOSE_FACTORY(NameOf<TFactory>(), " seeking for ", descriptor);
      const auto found = mIndexedByDescriptor.Find(descriptor);
      if (found)
         return found.GetVal();      
      return nullptr;
   }

   /// Create/Destroy element(s) inside the factory.                          
   /// This considers all verb requests and mass. All produced T (if any)     
   /// will go to the verb's output container.                                
   ///   @param producer the producer                                         
   ///   @param verb the creation verb                                        
   TEMPLATE() template<class P>
   void FACTORY()::Create(P* producer, Verb& verb) {
      constexpr ProducersOf<T> producers;
      static_assert(Expand(producers, []<class...C> {
         return (CT::Related<C, P> or ...);
      }),
         "None of the producers in " + CppNameOf<decltype(producers)>() + 
         " is related to " + CppNameOf<P>()
      );

      const auto requested = MetaDataOf<T>();
      verb.ForEachDeep(
         [this,&requested,&producer,&verb](Recipe const& recipe) {
            // For each compatible recipe...                            
            if (not requested.CastsTo(recipe.GetTarget()))
               return;
            
            auto count = static_cast<int>(
               ::std::floor(recipe.GetMass() * verb.GetMass())
            );

            CreateOrDestroyInner(producer, verb, count, recipe.GetDescriptor());
         },
         [this,&requested,&producer,&verb](RTTI::DMeta const& type) {
            // For each compatible type...                              
            if (not type or not requested.CastsTo(type))
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );

            CreateOrDestroyInner(producer, verb, count);
         }
      );
   }

   /// Select/Deselect element(s) inside the factory                          
   ///   @param verb the selection verb                                       
   TEMPLATE()
   void FACTORY()::Select(Verb& verb) {
      // For each recipe or meta compatible with the factory            
      verb.ForEachDeep(
         [&](Recipe const& recipe) {
            // For each recipe...                                       
            if (not MetaDataOf<T>().CastsTo(recipe.GetTarget()))
               return;

            TODO();
         },
         [&](RTTI::DMeta const& type) {
            // For each type...                                         
            if (not type or not MetaDataOf<T>().CastsTo(type))
               return;

            TODO();
         }
      );
   }

   /// Produce a single T with the given descriptor, or reuse one if factory  
   /// IsUnique, and there is already item with the same descriptor available.
   ///   @param producer the producer                                         
   ///   @param descriptor element descriptor                                 
   ///   @return the found/produced instance                                  
   TEMPLATE() template<class P>
   auto FACTORY()::Produce(P* producer, Neat const& descriptor) -> T* {
      // Check if producer is correct at compile-time                   
      constexpr ProducersOf<T> producers;
      static_assert(Expand(producers, []<class...C> {
         return (CT::Related<C, P> or ...);
      }),
         "None of the producers in " + CppNameOf<decltype(producers)>() + 
         " is related to " + CppNameOf<P>()
      );

      if constexpr (IsUnique) {
         // Check if descriptor matches any of the available            
         const auto found = FindByDescription(descriptor);
         if (found)
            return const_cast<T*>(found);
      }

      // Allocate the new item                                          
      VERBOSE_FACTORY(NameOf<TFactory>(), " producing: ", descriptor);
      auto result = Base::Emplace(producer, descriptor);
      if (not result)
         return nullptr;

      // Register item in the hashmap, for fast search by descriptor    
      T* new_pointer = &*result;
      if constexpr (IsUnique)
         mIndexedByDescriptor.Merge(descriptor, new_pointer);
      else {
         auto found = mIndexedByDescriptor.Find(descriptor);
         if (found)
            found.GetVal() << new_pointer;
         else
            mIndexedByDescriptor.Merge(descriptor, new_pointer);
      }
      return new_pointer;
   }

   /// Emplace a non-unique element manually with custom constructor          
   ///   @param arguments... arguments to forwards towards contructor         
   ///   @return a pointer to the newly created element                       
   /*TEMPLATE() template<class...ARG>
   auto FACTORY()::Emplace(ARG&&...arguments) -> T* requires (not IsUnique) {
      auto result = Base::NewInner(LglsFwd(arguments)...);
      if (not result)
         return nullptr;

      const auto hash = result->mData.GetHash();
      const auto found = mIndexedByDescriptor.FindIt(hash);
      if (found)
         found.GetVal() << result;
      else
         mIndexedByDescriptor.Insert(hash, result);

      return &result->mData;
   }*/

   /// Destroys an element inside factory                                     
   ///   @param descriptor element to destroy                                 
   ///   @param count number of element to destroy (ignored if IsUnique)      
   TEMPLATE()
   void FACTORY()::EraseByDescription(Neat const& descriptor, int count) {
      LglsAssumeDevAndOptimize(count > 0);

      if constexpr (IsUnique) {
         const auto pair = mIndexedByDescriptor.Find(descriptor);
         T const* const item = pair.GetVal();
         mIndexedByDescriptor.Erase(pair);
         Base::Erase(item);
      }
      else {
         const auto pair = mIndexedByDescriptor.Find(descriptor);

         if (pair.GetVal().GetCount() <= static_cast<size_t>(count)) {
            // Keep it in a non-owning list, so that we don't lose it   
            ::std::vector<T const*> list;
            list.reserve(pair.GetVal().GetCount());
            for (T const* item : pair.GetVal())
               list.emplace_back(item);

            // Erase from registry                                      
            mIndexedByDescriptor.Erase(pair);

            // Erase from Hive                                          
            for (T const* item : list | ::std::views::reverse)
               Base::Erase(item);
         }
         else {
            CT::Handle auto item_handle = pair.GetVal()[Index::Last];
            while(count) {
               T* item = *item_handle;

               // Erase from registry                                   
               pair.GetVal().Erase(item_handle);
               --item_handle;

               // Erase from Hive                                       
               Base::Erase(item);
               --count;
            }
         }
      }
   }

#if LANGULUS(SAFE)
   /// Dump the factory to the log                                            
   TEMPLATE()
   void FACTORY()::Dump() const {
      auto _ = Logger::SpecialScoped("--------- FACTORY DUMP FOR ", 
         MetaDataOf<TFactory>(), " (", Base::mCount, " of ", Base::mReserved,
         " cells used in ", Base::mFrames.GetCount(), " frames): "
      );

      size_t counter = 0;
      for (auto& item : *this) {
         Logger::Info(counter++, "] ", item, ", ",
            item.GetReferences(), " references");
      }
   }
#endif

   /// Inner creation/destruction verb                                        
   ///   @param producer the producer                                         
   ///   @param verb [in/out] the creation/destruction verb                   
   ///   @param count the number of items to create (or destroy if negative)  
   ///   @param descriptor element descriptor                                 
   TEMPLATE() template<class P>
   void FACTORY()::CreateOrDestroyInner(
      P* producer, Verb& verb, int count, Neat const& descriptor
   ) {
      if (count > 0) {
         // Produce amount of compatible recipes                        
         if constexpr (IsUnique) {
            // We can ignore count if only one unique T can exist       
            verb << Produce(producer, descriptor);
         }
         else {
            // Satisfy the required count                               
            while (count >= 1) {
               verb << Produce(producer, descriptor);
               --count;
            }
         }
      }
      else if (count < 0) {
         // Destroy amount of compatible recipes                        
         if constexpr (IsUnique) {
            // We can ignore count if only one unique T can exist       
            EraseByDescription(descriptor);
         }
         else {
            // Destroy the desired amount of elements, in the reverse   
            // order of creation.                                       
            EraseByDescription(descriptor, -count);
         }
      }

      // Note: can also be reached by 0 count, which can be used to     
      // probe capability of Create verb.                               
      verb.Done();
   }
}


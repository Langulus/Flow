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

      for (auto& item : *this) {
         item.TeardownInner();

         // Propagate Teardown routine                                  
         if constexpr (requires { item.Teardown(); })
            item.Teardown();
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

   /// Find element(s) with the provided descriptor                           
   ///   @param descriptor the normalized descriptor for the element          
   ///   @return the found element, or nullptr if not found                   
   TEMPLATE()
   auto FACTORY()::FindInner(Neat const& descriptor) const -> Query {
      VERBOSE_FACTORY(NameOf<TFactory>(), " seeking for ", descriptor);
      const auto found = mIndexedByDescriptor.Find(descriptor);
      if (found)
         return found.GetVal();      
      return nullptr;
   }

   /// Create/Destroy element(s) inside the factory                           
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

      const auto produced = MetaDataOf<T>();
      verb.ForEachDeep(
         [this,&produced,&producer,&verb](Recipe const& recipe) {
            // For each compatible recipe...                            
            if (not produced.CastsTo(recipe.GetTarget()))
               return;
            
            auto count = static_cast<int>(
               ::std::floor(recipe.GetMass() * verb.GetMass())
            );

            CreateInner(producer, verb, count, recipe.GetDescriptor());
         },
         [this,&produced,&producer,&verb](RTTI::DMeta const& type) {
            // For each compatible type...                              
            if (not type or not produced.CastsTo(type))
               return;

            auto count = static_cast<int>(
               ::std::floor(verb.GetMass())
            );

            CreateInner(producer, verb, count);
         }
      );
   }
   
   /// Create (or reuse) a single element                                     
   ///   @param producer the producer                                         
   ///   @param descriptor the descriptor                                     
   ///   @return the new (or reused) instance                                 
   TEMPLATE() template<class P>
   auto FACTORY()::CreateOne(P* producer, Neat const& descriptor) -> T* {
      constexpr ProducersOf<T> producers;
      static_assert(Expand(producers, []<class...C> {
         return (CT::Related<C, P> or ...);
      }),
         "None of the producers in " + CppNameOf<decltype(producers)>() + 
         " is related to " + CppNameOf<P>()
      );

      if constexpr (IsUnique) {
         // Check if descriptor matches any of the available            
         const auto found = FindInner(descriptor);
         if (found)
            return found;
      }

      // If reached, nothing was found                                  
      // Produce exactly one element with this descriptor               
      return Produce(producer, descriptor);
   }

   /// Emplace a non-unique element manually with custom constructor          
   ///   @param arguments... arguments to forwards towards contructor         
   ///   @return a pointer to the newly created element                       
   TEMPLATE() template<class...ARG>
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
   }

   /// Inner creation/destruction verb                                        
   ///   @param producer the producer                                         
   ///   @param verb [in/out] the creation/destruction verb                   
   ///   @param count the number of items to create (or destroy if negative)  
   ///   @param descriptor element descriptor                                 
   TEMPLATE() template<class P>
   void FACTORY()::CreateInner(
      P* producer, Verb& verb, int count, Neat const& descriptor
   ) {
      if (count > 0) {
         // Produce amount of compatible recipes                        
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = FindInner(descriptor);
            if (found) {
               // The unique recipe was found, just return it.          
               // Mass will be ignored, it makes no sense to            
               // create multiple instances if unique                   
               verb << found;
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
         // Destroy amount of compatible recipes                        
         if constexpr (IsUnique) {
            // Check if descriptor matches any of the available         
            const auto found = FindInner(descriptor);
            if (found) {
               // The unique recipe was found, destroy it               
               // Mass is ignored, there should be exactly one          
               Destroy(const_cast<T*>(found));
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

   /// External interface for finding an entry in the factory                 
   ///   @param descriptor descriptor to match exactly                        
   ///   @return a valid pointer if element was found                         
   TEMPLATE()
   auto FACTORY()::Find(Neat const& descriptor) const -> Query {
      return FindInner(descriptor);
   }

   /// Produce a single T with the given descriptor                           
   ///   @param producer the producer                                         
   ///   @param descriptor element descriptor                                 
   ///   @return the produced instance                                        
   TEMPLATE() template<class P>
   auto FACTORY()::Produce(P* producer, Neat const& descriptor) -> T* {
      // Register entry in the hashmap, for fast search by descriptor   
      VERBOSE_FACTORY(NameOf<TFactory>(), " producing: ", descriptor);
      auto result = Base::Emplace(producer, descriptor);
      if (not result)
         return nullptr;

      if constexpr (IsUnique)
         mIndexedByDescriptor.Merge(descriptor, result);
      else {
         const auto found = mIndexedByDescriptor.Find(descriptor);
         if (found)
            found.GetVal() << result;
         else
            mIndexedByDescriptor.Merge(descriptor, result);
      }
      return result;
   }

   /// Destroys an element inside factory                                     
   ///   @attention assumes item is a valid pointer, owned by the factory     
   ///   @attention item pointer is no longer valid after this call           
   ///   @param item element to destroy                                       
   TEMPLATE()
   void FACTORY()::Destroy(T* item) {
      // Remove from hashmap                                            
      if constexpr (IsUnique)
         mIndexedByDescriptor.Erase(item->GetDescriptor());
      else {
         auto found = mIndexedByDescriptor.Find(item->GetDescriptor());
         IF_SAFE(if (not found) return;)

         if (found.GetVal()->Erase(item)) {
            if (found.GetVal()->IsEmpty())
               mIndexedByDescriptor.Erase(found);
         }
      }

      // Remove from hive                                               
      Base::Erase(item);
   }
}


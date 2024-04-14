///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"
#include <Anyness/Neat.hpp>
#include <Anyness/THive.hpp>


namespace Langulus::Flow
{

   /// Usage styles for TFactory                                              
   enum class FactoryUsage {
      Default,		// Default factories aggregate duplicated items       
      Unique		// Unique factories never duplicate items (a set)     
   };

   /// Concept for determining if a type is producible from a factory         
   /// The type must have a producer defined, not be abstract, be dense, and  
   /// be referencable                                                        
   template<class...T>
   concept FactoryProducible = ((CT::Producible<T>
         and not CT::Abstract<T> and CT::Dense<T> and CT::Referencable<T>)
      and ...);


   ///                                                                        
   ///   Factory container                                                    
   ///                                                                        
   ///   Basically a templated container used to contain, produce, but most   
   /// importantly reuse memory. The factory can contain only reference-      
   /// counted types, because elements are forbidden to move, and are reused  
   /// in-place. Additionally, the factory also internally utilizes a hashmap 
   /// to quickly find relevant elements. Items are laid out serially, so     
   /// that iteration is as fast and cache-friendly as possible.              
   ///                                                                        
   ///	By specifying a FactoryUsage::Unique usage, you're essentially       
   /// making a set of the produced resources, never duplicating elements     
   /// with the same descriptor twice.                                        
   ///                                                                        
   template<class T, FactoryUsage USAGE = FactoryUsage::Default>
   class TFactory : public Anyness::THive<T> {
   public:
      static_assert(CT::Producible<T>, "T must have a producer");

      LANGULUS(TYPED) T;
      using Base = Anyness::THive<T>;
      using Producer = CT::ProducerOf<T>;
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;

   protected: IF_LANGULUS_TESTING(public:)
      using typename Base::Cell;

      // Each factory is bound to a producer instance                   
      // Every produced T will also be bound to that instance           
      // If factory moved, all contents will be remapped to the new     
      // instance                                                       
      Producer* mFactoryOwner {};

      // A hash map for fast retrieval of elements                      
      TUnorderedMap<Hash, TAny<Cell*>> mHashmap;

      NOD() T* Produce(const Neat&);
      void CreateInner(Verb&, int, const Neat& = {});
      void Destroy(Cell*);
      NOD() Cell* Find(const Neat&) const;

   public:
      /// Factories can't be default-, move- or copy-constructed              
      /// We must guarantee that mFactoryOwner is always valid, and move is   
      /// allowed only via assignment, on a previously initialized factory    
      /// This is needed, because elements must be remapped to a new valid    
      /// owner upon move                                                     
      TFactory() = delete;
      TFactory(const TFactory&) = delete;
      TFactory(TFactory&&) = delete;

      TFactory(Producer*);
      TFactory& operator = (TFactory&&) noexcept;
      ~TFactory();

   public:
      void Reset();

      void Create(Verb&);
      void Select(Verb&);

      IF_SAFE(void Dump() const);
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;


   ///                                                                        
   ///   An element, that is factory produced (used as CRTP)                  
   ///                                                                        
   /// Saves the descriptor by which the item was made with, in order to      
   /// compare creation requests                                              
   ///   @attention mDerscriptor is known to cause circular dependencies, so  
   ///      it will be reset the moment owning TFactory is destroyed          
   ///                                                                        
   template<class T>
   class ProducedFrom {
      LANGULUS(PRODUCER) T;

   protected:
      // The descriptor used for hashing, and element identification    
      Neat mDescriptor;
      // The producer of the element                                    
      T* const mProducer {};

   public:
      ProducedFrom(const ProducedFrom&) = delete;
      ProducedFrom(ProducedFrom&&);
      ProducedFrom(T* = nullptr, const Neat& = {});

      template<template<class> class S>
      requires CT::Semantic<S<Neat>>
      ProducedFrom(S<ProducedFrom>&&);

      void Detach();
      const Neat& GetNeat() const noexcept;
      Hash GetHash() const noexcept;
      T* GetProducer() const noexcept;
   };

} // namespace Langulus::Flow

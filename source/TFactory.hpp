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
   template<class T>
   concept FactoryProducible = CT::Producible<T>
                       and not CT::Abstract<T>
                       and     CT::Dense<T>
                       and     CT::Referencable<T>;


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
   class TFactory {
   public:
      static_assert(CT::Complete<T>, "T must be a complete type");
      static_assert(CT::Dense<T>, "T must be a dense type");
      static_assert(CT::Data<T>, "T can't be void");
      static_assert(CT::Referencable<T>, "T must be referencable");
      static_assert(CT::Producible<T>, "T must have a producer");
      static_assert(not CT::Abstract<T>, "T can't be abstract");

      LANGULUS(TYPED) T;
      using Producer = CT::ProducerOf<T>;
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;

   protected: IF_LANGULUS_TESTING(public:)
      struct Element;

      // Each factory is bound to a producer instance                   
      // Every produced T will also be bound to that instance           
      // If factory moved, all contents will be remapped to the new     
      // instance                                                       
      Producer* mFactoryOwner {};
      // Elements are allocated here, so they are cache-friendly and    
      // iterated fast, rarely ever moving                              
      TAny<Element> mData;
      // The start of the reusable chain                                
      Element* mReusable {};
      // A hash map for fast retrieval of elements                      
      TUnorderedMap<Hash, TAny<Element*>> mHashmap;
      // Number of initialized elements                                 
      Count mCount {};

      NOD() T* Produce(const Neat&);
      void CreateInner(Verb&, int, const Neat& = {});
      void Destroy(Element*);
      NOD() Element* Find(const Neat&) const;

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

      NOD() bool IsEmpty() const noexcept;

      NOD() constexpr explicit operator bool() const noexcept;

      void Create(Verb&);
      void Select(Verb&);

      ///                                                                     
      ///   Iteration                                                         
      ///                                                                     
      template<bool MUTABLE>
      struct TIterator;

      using Iterator = TIterator<true>;
      using ConstIterator = TIterator<false>;

      NOD() Iterator begin() noexcept;
      NOD() Iterator end() noexcept;
      NOD() Iterator last() noexcept;
      NOD() ConstIterator begin() const noexcept;
      NOD() ConstIterator end() const noexcept;
      NOD() ConstIterator last() const noexcept;

      IF_SAFE(void Dump() const);
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;


   ///                                                                        
   ///   An element, that is factory produced (used as CRTP)                  
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
      ProducedFrom() = delete;
      ProducedFrom(const ProducedFrom&) = delete;
      ProducedFrom(ProducedFrom&&);
      ProducedFrom(T*, const Neat&);

      template<template<class> class S>
      requires CT::Semantic<S<Neat>>
      ProducedFrom(S<ProducedFrom>&&);

      const Neat& GetNeat() const noexcept;
      Hash GetHash() const noexcept;
      T* GetProducer() const noexcept;
   };


   ///                                                                        
   ///   Factory element (for internal usage)                                 
   ///                                                                        
   template<class T, FactoryUsage USAGE>
   struct TFactory<T, USAGE>::Element {
   protected: IF_LANGULUS_TESTING(public:)
      friend class TFactory<T, USAGE>;
      union {
         // When element is in use, this pointer points to the          
         // factory who produced, and owns the element                  
         TFactory* mFactory;
         // When element is not in use, this pointer points to the      
         // next free entry in the factory                              
         Element* mNextFreeElement;
      };

      // Counts the uses of a factory element, because T should be      
      // referencable. If references are zero, element is unused, and   
      // mNextFreeElement is set. Still in use after destruction!       
      // Referencable's destructor should make sure references are zero 
      // in memory, after destruction                                   
      T mData;

   public:
      Element() = delete;

      Element(TFactory*, const Neat&);

      template<template<class> class S>
      requires CT::SemanticMakableAlt<S<T>>
      Element(S<Element>&&);
   };


   ///                                                                        
   ///   Factory iterator                                                     
   ///                                                                        
   template<class T, FactoryUsage USAGE>
   template<bool MUTABLE>
   struct TFactory<T, USAGE>::TIterator {
   protected:
      friend class TFactory<T, USAGE>;

      const Element* mElement;
      const Element* mSentinel;

      TIterator(const Element*, const Element*) noexcept;

   public:
      NOD() bool operator == (const TIterator&) const noexcept;

      NOD() T& operator * () const noexcept requires (MUTABLE);
      NOD() const T& operator * () const noexcept requires (not MUTABLE);

      NOD() T& operator -> () const noexcept requires (MUTABLE);
      NOD() const T& operator -> () const noexcept requires (not MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Flow

///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"

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
   concept FactoryProducible = CT::Producible<T> && !CT::Abstract<T> && CT::Dense<T> && CT::Referencable<T>;


   ///                                                                        
   /// Normalized data container                                              
   ///                                                                        
   class Normalized {
   private:
      // Verbs will always be ordered in the order they appear          
      // Their contents will be normalized all the way through          
      TAny<Verb> mVerbs;
      // Traits are ordered first by their trait type, then by their    
      // order of appearance. Duplicate trait types are allowed         
      // Trait contents are also normalized all the way through         
      TUnorderedMap<TMeta, TAny<Trait>> mTraits;
      // Metas are ordered by their hash, duplicates are discarded      
      TUnorderedSet<DMeta> mMetaDatas;
      TUnorderedSet<TMeta> mMetaTraits;
      TUnorderedSet<CMeta> mMetaConstants;
      TUnorderedSet<VMeta> mMetaVerbs;
      // Subconstructs are sorted first by the construct type, and then 
      // by their order of appearance. Their contents are also          
      // nest-normalized all the way through                            
      TUnorderedMap<DMeta, TAny<Construct>> mConstructs;
      // Any other block type that doesn't fit in the above is sorted   
      // first by the block type, then by the order of appearance       
      // All sub-blocks are normalized all the way through              
      TUnorderedMap<DMeta, TAny<Any>> mAnythingElse;

      mutable Hash mHash;

   public:
      Normalized(const Any&);

      NOD() Hash GetHash() const;

      bool operator == (const Normalized&) const;
   };


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
   /// making a set of the produced resources, never duplicating same         
   /// creations twice. It is highly recommended in such cases, to make the   
   /// produced item hashable and implement a satisfyingly fast compare       
   /// operators, to avoid	huge overheads.                                    
   ///                                                                        
   template<class T, FactoryUsage USAGE = FactoryUsage::Default>
   class TFactory {
   public:
      static_assert(CT::Dense<T>, "T must be a dense type");
      static_assert(CT::Data<T>, "T can't be void");
      static_assert(CT::Referencable<T>, "T must be referencable");
      static_assert(CT::Producible<T>, "T must have a producer");
      static_assert(CT::DescriptorMakable<T>, "T must have a descriptor-constructor");
      static_assert(!CT::Abstract<T>, "T can't be abstract");

      /// Makes the factory CT::Typed                                         
      using MemberType = T;
      using Producer = CT::ProducerOf<T>;
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;

   protected:
   TESTING(public:)
      struct Element;

      // Each factory is bound to a producer instance                   
      // Every produced T will also be bound to that instance           
      // If factory moved, all contents will be remapped to the new     
      // instance                                                       
      Producer* const mFactoryOwner;
      
      // Elements are allocated here, so they are cache-friendly and    
      // iterated fast, rarely ever moving                              
      TAny<Element> mData;

      // The start of the reusable chain                                
      Element* mReusable {};

      // A hash map for fast retrieval of elements                      
      TUnorderedMap<Hash, TAny<Element*>> mHashmap;

      NOD() T* Produce(Hash, const Any&, const Normalized&);
      void CreateInner(Verb&, int, const Any& = {});
      void Destroy(Element*);
      NOD() Element* Find(Hash, const Normalized&) const;

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

   public:
      void Reset();

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
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;


   ///                                                                        
   ///   Factory element (for internal usage)                                 
   ///                                                                        
   template<class T, FactoryUsage USAGE>
   struct TFactory<T, USAGE>::Element {
   protected:
   TESTING(public:)
      friend class TFactory<T, USAGE>;
      union {
         // When element is in use, this pointer points to the          
         // factory who produced, and owns the element                  
         TFactory* mFactory;

         // When element is not in use, this pointer points to the      
         // next free entry in the factory                              
         Element* mNextFreeElement;
      };

      // Precomputed hash of the descriptor                             
      Hash mHash;

      // The descriptor used for hashing, and element identification    
      // Not valid if mReferences is zero                               
      Normalized mDescriptor;

      // Counts the uses of a factory element, because T should be      
      // referencable. If references are zero, element is unused, and   
      // mNextFreeElement is set. Still in use after destruction!       
      // Referencable's destructor should make sure references are zero 
      // in memory, after destruction                                   
      T mData;

   public:
      Element() = delete;
      Element(TFactory*, Hash, const Any&, const Normalized&);
      Element(Element&&) = default;
      ~Element() noexcept = default;
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
      NOD() const T& operator * () const noexcept requires (!MUTABLE);

      NOD() T& operator -> () const noexcept requires (MUTABLE);
      NOD() const T& operator -> () const noexcept requires (!MUTABLE);

      // Prefix operator                                                
      TIterator& operator ++ () noexcept;

      // Suffix operator                                                
      NOD() TIterator operator ++ (int) noexcept;
   };

} // namespace Langulus::Flow

#include "TFactory.inl"
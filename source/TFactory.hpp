///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"
#include <Langulus/Anyness/THive.hpp>


namespace Langulus::Flow
{

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
      LANGULUS(TYPED) T;
      using Base = Anyness::THive<T>;
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;
      static constexpr bool IsNotUnique = not IsUnique;

   protected:
      using typename Base::Cell;

      // A hash map for fast retrieval of elements                      
      TUnorderedMap<Hash, TMany<Cell*>> mHashmap;

      auto Produce(auto*, const Many&) -> T*;
      void CreateInner(auto*, Verb&, int, const Many& = {});
      void Destroy(Cell*);
      auto FindInner(const Many&) const -> Cell*;

   public:
      /// Factories can't be default-, move- or copy-constructed              
      /// We must guarantee that mFactoryOwner is always valid, and move is   
      /// allowed only via assignment, on a previously initialized factory    
      /// This is needed, because elements must be remapped to a new valid    
      /// owner upon move                                                     
      TFactory() = default;
      TFactory(const TFactory&) = delete;
      TFactory(TFactory&&) = delete;
     ~TFactory();

      auto operator = (TFactory&&) noexcept -> TFactory&;

      void Reset();
      void Create(auto*, Verb&);
      auto CreateOne(auto*, const Many&) -> T*;
      template<class...ARG>
      auto Emplace(ARG&&...) -> T* requires IsNotUnique;
      void Select(Verb&);
      auto Find(const Many&) const -> const T*;
      void Teardown();

      IF_SAFE(void Dump() const);

      #if LANGULUS(TESTING)
         auto& GetHashmap() const { return mHashmap; }
      #endif
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;

} // namespace Langulus::Flow

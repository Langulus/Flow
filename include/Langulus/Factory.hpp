///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/THive.hpp>
#include <Langulus/TMap.hpp>
#include <Langulus/TMany.hpp>
#include "Producible.hpp"


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
   struct TFactory : Annies::THive<T> {
      using CTTI_ReflectAs    = TFactory;
      using Base              = Annies::THive<T>;
      
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;
      static constexpr bool IsNotUnique = not IsUnique;

   protected:
      using typename Base::Cell;

      // A hash map for fast retrieval of elements                      
      TMapUnsorted<Hash, TMany<Cell*>> mHashmap;

      auto Produce(auto*, Many const&) -> T*;
      void CreateInner(auto*, Verb&, int, Many const& = {});
      void Destroy(Cell*);
      auto FindInner(Many const&) const -> Cell*;

   public:
      /// Factories can't be default-, move- or copy-constructed              
      /// We must guarantee that mFactoryOwner is always valid, and move is   
      /// allowed only via assignment, on a previously initialized factory    
      /// This is needed, because elements must be remapped to a new valid    
      /// owner upon move                                                     
      TFactory() = default;
      TFactory(const TFactory&) = delete("Factory elements are bound to the "
                                         "factory that produced them, "
                                         "and can't be copied to another");
      TFactory(TFactory&&)      = delete("Factory elements are bound to the "
                                         "factory that produced them, "
                                         "and can't be moved to another");
     ~TFactory();

      auto operator = (TFactory&&) noexcept -> TFactory&;

      void Reset();
      void Create(auto*, Verb&);
      auto CreateOne(auto*, Many const&) -> T*;
      template<class...ARG>
      auto Emplace(ARG&&...) -> T* requires IsNotUnique;
      void Select(Verb&);
      auto Find(Many const&) const -> const T*;
      void Teardown();

      IF_SAFE(void Dump() const);

      #if LANGULUS(TESTING)
         auto& GetHashmap() const { return mHashmap; }
      #endif
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;
}

#include "Factory.inl"
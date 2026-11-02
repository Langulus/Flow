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
#include <Langulus/TVerb.hpp>
#include <Langulus/Neat.hpp>
#include "Producible.hpp"

#define TEMPLATE()   template<class T, FactoryUsage USAGE>
#define FACTORY()    TFactory<T, USAGE>


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
   ///	By specifying a FactoryUsage::Unique usage, you're essentially       
   /// making a set of the produced resources, never duplicating elements     
   /// with the same descriptor twice.                                        
   template<class T, FactoryUsage USAGE = FactoryUsage::Default>
   struct TFactory : Annies::THive<T> {
      using CTTI_ReflectAs    = TFactory;
      using Base              = Annies::THive<T>;
      
      static constexpr bool IsUnique = USAGE == FactoryUsage::Unique;
      using Query = Tif<IsUnique, T const*, TMany<T const*> const*>;

   protected:
      // Elements indexed by their descriptor                           
      TMapUnsorted<Neat, Tif<IsUnique, T const*, TMany<T const*>>> mIndexedByDescriptor;

      template<class P>
      auto Produce(P*, Neat const&) -> T*;

      template<class P>
      void CreateInner(P*, Verb&, int, Neat const& = {});

      void Destroy(T*);

      auto FindInner(Neat const&) const -> Query;

   public:
      /// Factories can't be default-, move- or copy-constructed.             
      /// We must guarantee that mFactoryOwner is always valid, and move is   
      /// allowed only via assignment, on a previously initialized factory.   
      /// This is needed, because elements must be remapped to a new valid    
      /// owner upon move.                                                    
      TFactory() = default;
      TFactory(TFactory const&) = delete("Factory elements are bound to the "
                                         "factory that produced them, "
                                         "and can't be copied to another");
      TFactory(TFactory&&)      = delete("Factory elements are bound to the "
                                         "factory that produced them, "
                                         "and can't be moved to another");
     ~TFactory();

      auto operator = (TFactory&&) noexcept -> TFactory&;

      void Reset();

      template<class P>
      void Create(P*, Verb&);

      template<class P>
      auto CreateOne(P*, Neat const&) -> T*;

      template<class...ARG>
      auto Emplace(ARG&&...) -> T* requires (not IsUnique);
      
      void Select(Verb&);
      auto Find(Neat const&) const -> Query;
      void Teardown();

      IF_SAFE(void Dump() const);
   };

   template<class T>
   using TFactoryUnique = TFactory<T, FactoryUsage::Unique>;
}

#include "Factory.inl"

#undef TEMPLATE
#undef FACTORY

namespace Langulus
{
   using Flow::TFactory;
   using Flow::TFactoryUnique;
}
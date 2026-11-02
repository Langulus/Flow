///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Neat.hpp>
#include <Langulus/TRef.hpp>


namespace Langulus::Flow
{
   /// Usage styles for TFactory                                              
   enum class FactoryUsage {
      Default,		// Default factories aggregate duplicated items       
      Unique		// Unique factories never duplicate items (a set)     
   };

   template<class, FactoryUsage>
   struct TFactory;


   ///                                                                        
   ///   An element, that is factory produced                                 
   ///                                                                        
   /// Saves the recipe by which the item was made with, in order to compare  
   /// creation requests.                                                     
   template<class T>
   struct ProducedFrom {
      using CTTI_Producer = T;

   protected:
      template<class, FactoryUsage>
      friend struct TFactory;

      // The descriptor used for hashing and element identification     
      /// @attention mDescriptor can contain anything (including Thing  //TODO think about adding Teardown to deep ownership components to handle that?
      ///    references) and is known to cause circular dependencies.   
      ///    That's why ProducedFrom::Teardown has to be called as a    
      ///    first-stage destruction, usually in a custom Reference(int)
      ///    routine.                                                   
      Neat mDescriptor;//TODO maybe make this optional by letting the produced item cache it if they want to?
      // The producer of the element                                    
      Ref<T> mProducer;

   public:
      ProducedFrom(ProducedFrom const&) = delete("Factory elements are bound to the "
                                                 "factory that produced them, "
                                                 "and can't be copied to another");

                                                 //TODO make make these protected?
      /// Move a produced item                                                
      ///   @param other the item to move                                     
      ProducedFrom(ProducedFrom&& other)
         : ProducedFrom {Move(other)} {}

      /// Construct a produced item                                           
      ///   @param producer the item's producer                               
      ///   @param descriptor the item's neat descriptor                      
      ProducedFrom(T* producer = nullptr, Neat const& descriptor = {})
         : mDescriptor {descriptor}
         , mProducer   {producer} {}

      /// Generic construction                                                
      ///   @param other intent and element to initialize with                
      ///   @attention mProducer intentionally not overwritten                
      template<template<class> class S>
      ProducedFrom(S<ProducedFrom>&& other) requires CT::Intent<S<ProducedFrom>>
         : mDescriptor {other.Nest(other->mDescriptor)} {}

      /// Get the normalized descriptor of the produced item                  
      constexpr auto GetDescriptor() const noexcept -> Neat const& {
         return mDescriptor;
      }

      /// Get the hash of the normalized descriptor                           
      constexpr auto GetHash() const noexcept -> Hash {
         return mDescriptor.GetHash();
      }

      /// Return the producer of the item (a.k.a. the owner of the factory)   
      auto GetProducer() const noexcept -> Ref<T> const& {
         return mProducer;
      }  

      /// First stage destruction if needed                                   
      void Teardown() {
         mDescriptor.Reset();
         mProducer.Reset();
      }
   };
}

namespace Langulus
{
   using Flow::ProducedFrom;
}
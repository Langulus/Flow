///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Producible.hpp"


namespace Langulus::Flow
{

   /// Move a produced item                                                   
   ///   @param other - the item to move                                      
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(ProducedFrom&& other)
      : ProducedFrom {Move(other)} {}

   /// Generic construction                                                   
   ///   @param other - intent and element to initialize with                 
   template<class T> template<template<class> class S> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(S<ProducedFrom>&& other)
   requires CT::Intent<S<ProducedFrom>>
      // mProducer intentionally not overwritten                        
      : mDescriptor {other.Nest(other->mDescriptor)} {}

   /// Construct a produced item                                              
   ///   @param producer - the item's producer                                
   ///   @param descriptor - the item's neat descriptor                       
   template<class T> LANGULUS(INLINED)
   ProducedFrom<T>::ProducedFrom(T* producer, const Many& descriptor)
      : mDescriptor {descriptor}
      , mProducer   {producer} {}

   /// Get the normalized descriptor of the produced item                     
   ///   @return the normalized descriptor                                    
   template<class T> LANGULUS(INLINED)
   auto ProducedFrom<T>::GetDescriptor() const noexcept -> const Many& {
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
   auto ProducedFrom<T>::GetProducer() const noexcept -> const Ref<T>& {
      return mProducer;
   }
   
   /// Return the producer of the item (a.k.a. the owner of the factory)      
   ///   @return a pointer to the producer instance                           
   template<class T> LANGULUS(INLINED)
   void ProducedFrom<T>::TeardownInner() {
      mDescriptor.Reset();
      mProducer.Reset();
   }

} // namespace Langulus::Flow
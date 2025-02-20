///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"
#include <Langulus/Anyness/Many.hpp>
#include <Langulus/Anyness/Ref.hpp>


namespace Langulus::Flow
{

   ///                                                                        
   ///   An element, that is factory produced (used as CRTP)                  
   ///                                                                        
   /// Saves the descriptor by which the item was made with, in order to      
   /// compare creation requests                                              
   ///   @attention mDescriptor can contain anything (including Thing         
   ///      references) and is known to cause circular dependencies. That's   
   ///      why ProducedFrom::Teardown has to be called as a first-stage      
   ///      destruction, usually in a custom Reference(int) routine.          
   ///                                                                        
   template<class T>
   class ProducedFrom {
      LANGULUS(PRODUCER) T;

   protected:
      template<class, FactoryUsage>
      friend class TFactory;

      // The descriptor used for hashing and element identification     
      Many mDescriptor;
      // The producer of the element                                    
      Ref<T> mProducer;

   public:
      ProducedFrom(const ProducedFrom&) = delete;
      ProducedFrom(ProducedFrom&&);
      ProducedFrom(T* = nullptr, const Many& = {});

      template<template<class> class S>
      ProducedFrom(S<ProducedFrom>&&) requires CT::Intent<S<ProducedFrom>>;

      auto GetDescriptor() const noexcept -> const Many&;
      Hash GetHash() const noexcept;
      auto GetProducer() const noexcept -> const Ref<T>&;
      void TeardownInner();
   };

} // namespace Langulus::Flow

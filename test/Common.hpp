///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Logger.hpp>
#include <Langulus/Testing/CommonTypes.hpp>
#include <Langulus/Downcastable.hpp>
#include <Langulus/Factory.hpp>
#include <Langulus/Producible.hpp>

using namespace Flow;


/// A mockup of Langulus::Thing, for testing purposes                         
struct Thing : Downcastable, Referenced {
   using CTTI_Producer  = Thing;
   using CTTI_Bases     = Types<Downcastable, Referenced>;

   Thing() : Downcastable {this} {}

   template<class T>
   Thing(const T* c) : Downcastable {c} {}

   virtual ~Thing() {
      Reference(-1);
   }

   virtual void Update() {}

   int mMember = 666;

   using CTTI_Members = Members<&Thing::mMember>;
};

/// A mockup of more concrete Langulus::Thing, for testing purposes           
struct Thing2 : Thing {
   using CTTI_Bases = Types<Thing>;

   Thing2() : Thing {this} {}

   void Update() final {}

   int mMember = 777;

   using CTTI_Members = Members<&Thing2::mMember>;
};

/// A mockup of a universe component, for testing purposes                    
struct Universe : Downcastable {
   using CTTI_Producer = Thing;
   using CTTI_Bases = Downcastable;
   
   Universe() : Downcastable {this} {}
};

/// A mockup of a window component, for testing purposes                      
struct Window : Downcastable {
   using CTTI_Producer = Thing;
   using CTTI_Bases = Downcastable;

   Window() : Downcastable {this} {}
};

/// A mockup of a user component, for testing purposes                        
struct User : Downcastable {
   using CTTI_Producer = Thing;
   using CTTI_Bases = Downcastable;

   User() : Downcastable {this} {}
};

/// A mockup of a session component, for testing purposes                     
struct Session : Downcastable {
   using CTTI_Producer = Thing;
   using CTTI_Bases = Downcastable;

   Session() : Downcastable {this} {}
};

/// A mockup of a fraction                                                    
/*struct Fraction : Downcastable {
   using CTTI_Abstract = No;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);
   Fraction() : Downcastable(MetaOf<Fraction>()) {}
};*/

struct Producer;

/// A mockup of a producible                                                  
struct Producible : Referenced, ProducedFrom<Producer> {
   Producible(Producer const* producer, Neat const& desc = {})
      : ProducedFrom {producer, desc} {}

   ~Producible() {
      Logger::Special("Destroying Producible");
   }

   bool operator == (Producible const& rhs) const {
      return mDescriptor == rhs.mDescriptor;
   }

   /*operator Text () const {
      return "Producible";
   }*/
};

LANGULUS_MORPHISM_CUSTOM(Producible, {
   (void) from;
   return "Producible";
}, Annies::Text);


struct DeepProducer;
struct ShallowProducer;

LANGULUS_MORPHISM_CUSTOM(DeepProducer, {
   (void) from;
   return "DeepProducer";
}, Annies::Text);

LANGULUS_MORPHISM_CUSTOM(ShallowProducer, {
   (void) from;
   return "ShallowProducer";
}, Annies::Text);

/// A mockup of a producible                                                  
struct TheProducible : Referenced, ProducedFrom<ShallowProducer> {
   //TODO should ProducedFrom inherit virtual Referenced directly?,
   // and so no need for ProducedFrom::Teardown method at all????
   TheProducible(ShallowProducer const* producer, Neat const& desc = {})
      : ProducedFrom {producer, desc} {}
};

LANGULUS_MORPHISM_CUSTOM(TheProducible, {
   (void) from;
   return "TheProducible";
}, Annies::Text);

/// A mockup of a producer                                                    
struct Producer : Referenced {
   TFactory<Producible>       factory1;
   TFactoryUnique<Producible> factory2;

   Producer() = default;

   // This constructor is required, because factories are not movable   
   // by default. We have to change all producers of all producibles    
   // in order for it to be a valid move. After the move, producibles   
   // should never change memory addresses, just be updated with new    
   // producer.                                                         
   Producer(Abandon<Producer>&& other) {
      factory1.TransferFrom(this, LglsMov(other->factory1));
      factory2.TransferFrom(this, LglsMov(other->factory2));
   }

   /*Producer(Abandon<Producer>&& other)
      : factory1 {Abandon {other->factory1}}
      , factory2 {Abandon {other->factory2}} {}*/

   void Teardown() {
      factory1.Teardown();
      factory2.Teardown();
   }
};

/// A mockup of a producible                                                  
struct ShallowProducer : Referenced, ProducedFrom<DeepProducer> {
   TFactory<TheProducible> factory;

   ShallowProducer() = default;

   // This constructor is required, because factories are not movable   
   // by default. We have to change all producers of all producibles    
   // in order for it to be a valid move. After the move, producibles   
   // should never change memory addresses, just be updated with new    
   // producer.                                                         
   ShallowProducer(Abandon<ShallowProducer>&& other) {
      factory.TransferFrom(this, LglsMov(other->factory));
   }

   ShallowProducer(DeepProducer const* producer, Neat const& desc = {})
      : ProducedFrom {producer, desc} {}

   void Teardown() {
      factory.Teardown();
      ProducedFrom<DeepProducer>::Teardown();
   }
};

/// A producer of a producer                                                  
struct DeepProducer : Referenced {
   TFactory<ShallowProducer> factory;

   DeepProducer() = default;

   // This constructor is required, because factories are not movable   
   // by default. We have to change all producers of all producibles    
   // in order for it to be a valid move. After the move, producibles   
   // should never change memory addresses, just be updated with new    
   // producer.                                                         
   DeepProducer(Abandon<DeepProducer>&& other) {
      factory.TransferFrom(this, LglsMov(other->factory));
   }

   void Teardown() {
      factory.Teardown();
   }
};



///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Flow/Resolvable.hpp>
#include <Flow/Factory.hpp>

using namespace Langulus;
using namespace Langulus::Flow;


/// A mockup of Langulus::Thing, for testing purposes                         
struct Thing : Resolvable, Referenced {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);

   Thing() : Resolvable {this} {}

   template<class T>
   Thing(const T* c) : Resolvable {c} {}

   virtual ~Thing() {
      Reference(-1);
   }

   virtual void Update() {}

   int mMember = 666;
};

/// A mockup of more concrete Langulus::Thing, for testing purposes           
struct Thing2 : Thing {
   LANGULUS_BASES(Thing);
   Thing2() : Thing {this} {}

   void Update() final {}

   int mMember = 777;
};

/// A mockup of a universe component, for testing purposes                    
struct Universe : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Universe() : Resolvable {this} {}
};

/// A mockup of a window component, for testing purposes                      
struct Window : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Window() : Resolvable {this} {}
};

/// A mockup of a user component, for testing purposes                        
struct User : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   User() : Resolvable {this} {}
};

/// A mockup of a session component, for testing purposes                     
struct Session : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Session() : Resolvable {this} {}
};

/// A mockup of a fraction                                                    
/*struct Fraction : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);
   Fraction() : Resolvable(MetaOf<Fraction>()) {}
};*/

struct Producible;

/// A mockup of a producer                                                    
struct Producer : Referenced {
   TFactory<Producible>       factory1;
   TFactoryUnique<Producible> factory2;
};

/// A mockup of a producible                                                  
struct Producible : Referenced, ProducedFrom<Producer> {
   Producible(Producer* producer, const Many& desc = {})
      : ProducedFrom {producer, desc} {}

   ~Producible() {
      Logger::Special("Destroying Producible");
   }

   Count Reference(int x) {
      if (Referenced::Reference(x) == 1) {
         // First-stage destruction must occur at the point where only  
         // the producer (or stack) has the only single reference left  
         ProducedFrom::Teardown();
      }

      return GetReferences();
   }

   bool operator == (const Producible& rhs) const {
      return mDescriptor == rhs.mDescriptor;
   }

   operator Text () const {
      return "Producible";
   }
};

struct ShallowProducer;

/// A producer of a producer                                                  
struct DeepProducer : Referenced {
   TFactory<ShallowProducer> factory;

   Count Reference(int x) {
      if (Referenced::Reference(x) == 1) {
         // First-stage destruction must occur at the point where only  
         // the producer (or stack) has the only single reference left  
         factory.Teardown();
      }

      return GetReferences();
   }
};

struct TheProducible;

/// A mockup of a producible                                                  
struct ShallowProducer : Referenced, ProducedFrom<DeepProducer> {
   TFactory<TheProducible> factory;

   ShallowProducer(DeepProducer* producer, const Many& desc = {})
      : ProducedFrom {producer, desc} {}

   Count Reference(int x) {
      if (Referenced::Reference(x) == 1) {
         // First-stage destruction must occur at the point where only  
         // the producer (or stack) has the only single reference left  
         factory.Teardown();
         ProducedFrom::Teardown();
      }

      return GetReferences();
   }
};

/// A mockup of a producible                                                  
struct TheProducible : Referenced, ProducedFrom<ShallowProducer> {
   //TODO should ProducedFrom inherit virtual Referenced directly?,
   // and so no need for ProducedFrom::Teardown method at all????
   TheProducible(ShallowProducer* producer, const Many& desc = {})
      : ProducedFrom {producer, desc} {}

   Count Reference(int x) {
      if (Referenced::Reference(x) == 1) {
         // First-stage destruction must occur at the point where only  
         // the producer (or stack) has the only single reference left  
         ProducedFrom::Teardown();
      }

      return GetReferences();
   }
};

/// Dump parse results and requirements                                       
template<class INPUT, class OUTPUT, class REQUIRED>
void DumpResults(const INPUT& in, const OUTPUT& out, const REQUIRED& required) {
   Logger::Special("-------------");
   Logger::Special("Script:   ", in);
   Logger::Special("Parsed:   ", out);
   Logger::Special("Required: ", required);
   Logger::Special("-------------");
}
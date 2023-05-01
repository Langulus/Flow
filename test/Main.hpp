#pragma once
#include <Flow/Resolvable.hpp>
#include <Flow/Factory.hpp>

using namespace Langulus;
using namespace Langulus::Flow;

#define CATCH_CONFIG_ENABLE_BENCHMARKING

/// A mockup of Langulus::Thing, for testing purposes                         
struct Thing : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Thing() : Resolvable(MetaData::Of<Thing>()) {}
   Thing(DMeta type) : Resolvable(type) {}
   virtual ~Thing() = default;

   virtual void Update() {}

   int mMember = 666;
};

/// A mockup of more concrete Langulus::Thing, for testing purposes           
struct Thing2 : public Thing {
   LANGULUS_BASES(Thing);
   Thing2() : Thing {MetaData::Of<Thing2>()} {}

   void Update() final {}

   int mMember = 777;
};

/// A mockup of a universe component, for testing purposes                    
struct Universe : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Universe() : Resolvable(MetaData::Of<Universe>()) {}
};

/// A mockup of a window component, for testing purposes                      
struct Window : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Window() : Resolvable(MetaData::Of<Window>()) {}
};

/// A mockup of a user component, for testing purposes                        
struct User : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   User() : Resolvable(MetaData::Of<User>()) {}
};

/// A mockup of a session component, for testing purposes                     
struct Session : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Session() : Resolvable(MetaData::Of<Session>()) {}
};

/// A mockup of a fraction                                                    
struct Fraction : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);
   Fraction() : Resolvable(MetaData::Of<Fraction>()) {}
};

/// A mockup of a producer                                                    
struct Producer {};

/// A mockup of a producible                                                  
struct Producible : Referenced, ProducedFrom<Producer> {
   Producible(Producer* producer, const Any& d = {})
      : ProducedFrom {producer, d} {}

   Any mDescriptor;

   bool operator == (const Producible& rhs) const {
      return mDescriptor == rhs.mDescriptor;
   }

   operator Debug () const {
      return "Producible";
   }
};
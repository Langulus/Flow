///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Flow/Resolvable.hpp>
#include <Flow/Factory.hpp>

using namespace Langulus;
using namespace Langulus::Flow;

#define CATCH_CONFIG_ENABLE_BENCHMARKING


/// A mockup of Langulus::Thing, for testing purposes                         
struct Thing : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Thing() : Resolvable(MetaOf<Thing>()) {}
   Thing(DMeta type) : Resolvable(type) {}
   virtual ~Thing() = default;

   virtual void Update() {}

   int mMember = 666;
};

/// A mockup of more concrete Langulus::Thing, for testing purposes           
struct Thing2 : Thing {
   LANGULUS_BASES(Thing);
   Thing2() : Thing {MetaOf<Thing2>()} {}

   void Update() final {}

   int mMember = 777;
};

/// A mockup of a universe component, for testing purposes                    
struct Universe : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Universe() : Resolvable(MetaOf<Universe>()) {}
};

/// A mockup of a window component, for testing purposes                      
struct Window : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Window() : Resolvable(MetaOf<Window>()) {}
};

/// A mockup of a user component, for testing purposes                        
struct User : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   User() : Resolvable(MetaOf<User>()) {}
};

/// A mockup of a session component, for testing purposes                     
struct Session : Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS(PRODUCER) Thing;
   LANGULUS_BASES(Resolvable);
   Session() : Resolvable(MetaOf<Session>()) {}
};

/// A mockup of a fraction                                                    
/*struct Fraction : public Resolvable {
   LANGULUS(ABSTRACT) false;
   LANGULUS(UNINSERTABLE) false;
   LANGULUS_BASES(Resolvable);
   Fraction() : Resolvable(MetaOf<Fraction>()) {}
};*/

/// A mockup of a producer                                                    
struct Producer {};

/// A mockup of a producible                                                  
struct Producible : Referenced, ProducedFrom<Producer> {
   Producible(Producer* producer, const Neat& neat = {})
      : ProducedFrom {producer, neat} {}

   bool operator == (const Producible& rhs) const {
      return mDescriptor == rhs.mDescriptor;
   }

   operator Debug () const {
      return "Producible";
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

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md        
#define LANGULUS_EXCEPTION_HANDLER \
   CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) { \
      const Text serialized {ex}; \
      return ::std::string {Token {serialized}}; \
   }

#include "Main.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

LANGULUS_RTTI_BOUNDARY("Test")

int main(int argc, char* argv[]) {
   static_assert(CT::Convertible<Thing, Debug>,
      "Thing must be convertible to Debug string");
   static_assert(CT::Convertible<Verb, Debug>,
      "Verb must be convertible to Debug string");
   static_assert(CT::Convertible<Verb, Code>,
      "Verb must be convertible to Code string");
   static_assert(CT::Convertible<Construct, Debug>,
      "Construct must be convertible to Debug string");
   static_assert(CT::Convertible<Construct, Code>,
      "Construct must be convertible to Code string");

   static_assert(CT::Debuggable<Thing>,
      "Thing must be convertible to Debug string");
   static_assert(CT::Debuggable<Verb>,
      "Verb must be convertible to Debug string");
   static_assert(CT::Debuggable<Construct>,
      "Construct must be convertible to Debug string");

   Catch::Session session;
   return session.run(argc, argv);
}

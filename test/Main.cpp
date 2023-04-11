#include "Main.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

LANGULUS_RTTI_BOUNDARY("Test")

int main(int argc, char* argv[]) {
   // For reflection of all verbs                                       
   (void)MetaData::Of<Index>();
   (void)MetaData::Of<Thing>();
   (void)MetaData::Of<Temporal>();
   (void)MetaData::Of<Universe>();
   (void)MetaData::Of<Window>();
   (void)MetaData::Of<User>();
   (void)MetaData::Of<Fraction>();
   (void)MetaData::Of<Construct>();

   // For reflection of all verbs                                       
   (void)MetaVerb::Of<Verbs::Add>();
   (void)MetaVerb::Of<Verbs::Associate>();
   (void)MetaVerb::Of<Verbs::Catenate>();
   (void)MetaVerb::Of<Verbs::Conjunct>();
   (void)MetaVerb::Of<Verbs::Create>();
   (void)MetaVerb::Of<Verbs::Do>();
   (void)MetaVerb::Of<Verbs::Exponent>();
   (void)MetaVerb::Of<Verbs::Interpret>();
   (void)MetaVerb::Of<Verbs::Multiply>();
   (void)MetaVerb::Of<Verbs::Select>();

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

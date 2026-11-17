///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"
#include <Langulus/SerializeText.hpp>
#include <Langulus/Code.hpp>
#include <Langulus/Verbs/Select.hpp>
#include <Langulus/Verbs/Do.hpp>


SCENARIO("Text capsulation in verbs") {
   static MemoryState memoryState;

   /*static_assert(CT::Stringifiable<Verb>,
      "Verb must be convertible to Text");
   static_assert(CT::Convertible<Verb, Code>,
      "Verb must be convertible to Code");

   static_assert(CT::Stringifiable<Thing>,
      "Thing must be convertible to Text");
   static_assert(CT::Convertible<Thing, Text>,
      "Thing must be convertible to Text");*/

   GIVEN("A templated utf8 text container") {
      Text text = "tests";

      WHEN("Wrapped inside a verb's output") {
         Verb wrapper = Verbs::Do().SetOutput(&text);
         Verb wrapper2 = wrapper;
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");

         wrapper.Reset();
         wrapper2.Reset();
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");
      }

      WHEN("Wrapped inside a verb's argument") {
         Verbs::Do wrapper {&text};
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");

         wrapper.Reset();
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");
      }

      WHEN("Wrapped inside a verb's source") {
         Verb wrapper = Verbs::Do().In(&text);
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");

         wrapper.Reset();
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");
      }

      WHEN("Wrapped everywhere inside a verb") {
         Verb wrapper = Verbs::Do(&text).In(&text).SetOutput(&text);
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");

         wrapper.Reset();
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");
      }
   }

   GIVEN("A default-initialized select verb") {
      Verbs::Select test;

      WHEN("Stringified") {
         Text toText;
         REQUIRE(Langulus::Serialize(test, toText));

         Flow::Code toCode;
         REQUIRE(Langulus::Serialize(test, toCode));

         REQUIRE(toText == toCode);
         REQUIRE(toText == ".");
      }
   }

   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}

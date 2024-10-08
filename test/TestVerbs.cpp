///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"
#include <Flow/Verbs/Select.hpp>


SCENARIO("Text capsulation in verbs", "[verbs]") {
   static Allocator::State memoryState;

   GIVEN("A templated utf8 text container") {
      Text text = "tests";

      REQUIRE(!text.IsStatic());
      REQUIRE(text.GetUses() == 1);

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
         Verb wrapper = Verbs::Do().SetSource(&text);
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");

         wrapper.Reset();
         REQUIRE(text.GetUses() == 1);
         REQUIRE(text == "tests");
      }

      WHEN("Wrapped everywhere inside a verb") {
         Verb wrapper = Verbs::Do(&text).SetSource(&text).SetOutput(&text);
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
         const auto toText = static_cast<Text>(test);
         const auto toCode = static_cast<Code>(test);

         REQUIRE(toText == toCode);
         REQUIRE(toText == ".");
      }
   }

   REQUIRE(memoryState.Assert());
}

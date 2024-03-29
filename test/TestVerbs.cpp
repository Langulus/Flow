///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Flow/Verbs/Select.hpp>
#include <catch2/catch.hpp>

SCENARIO("Text capsulation in verbs", "[verbs]") {
   GIVEN("A templated utf8 text container") {
      Text text = "tests";

      REQUIRE(!text.IsStatic());
      REQUIRE(text.GetUses() == 1);

      WHEN("Wrapped inside a verb's output") {
         Verb wrapper = Verbs::Do().SetOutput(&text);
         Verb wrapper2 = wrapper;
         THEN("The block's reference count must increase") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }

         wrapper.Reset();
         wrapper2.Reset();
         THEN("The block's reference count must decrease") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }
      }

      WHEN("Wrapped inside a verb's argument") {
         Verbs::Do wrapper {&text};
         THEN("The block's reference count must increase") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }

         wrapper.Reset();
         THEN("The block's reference count must decrease") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }
      }

      WHEN("Wrapped inside a verb's source") {
         Verb wrapper = Verbs::Do().SetSource(&text);
         THEN("The block's reference count must increase") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }

         wrapper.Reset();
         THEN("The block's reference count must decrease") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }
      }

      WHEN("Wrapped everywhere inside a verb") {
         Verb wrapper = Verbs::Do(&text).SetSource(&text).SetOutput(&text);
         THEN("The block's reference count must increase") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }

         wrapper.Reset();
         THEN("The block's reference count must decrease") {
            REQUIRE(text.GetUses() == 1);
            REQUIRE(text == "tests");
         }
      }
   }

   GIVEN("A default-initialized select verb") {
      Verbs::Select test;

      WHEN("Stringified") {
         // Calling static_cast here produces errors due to MSVC bug    
         const auto toDebug = test.operator Debug();
         const auto toCode = test.operator Code();

         THEN("The block's reference count must increase") {
            REQUIRE(toDebug == toCode);
            REQUIRE(toDebug == ".");
         }
      }
   }
}

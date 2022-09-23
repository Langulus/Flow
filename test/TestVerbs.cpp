#include "Main.hpp"
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
			Verbs::Do wrapper(&text);
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
}

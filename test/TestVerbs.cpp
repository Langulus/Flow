#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Text capsulation in verbs", "[text]") {
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
			Verb wrapper = Verbs::Do(&text);
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

SCENARIO("Serializing verbs", "[verbs]") {
	GIVEN("A container with various kinds of data") {
		auto stuff = Any::Wrap(
			"some text"_text,
			10, 5, 20.0f, 40.0,
			Verbs::Do(10).SetSource(5),
			Verbs::Do(10).SetSource("some other text"_text),
			Verbs::Do("even more text"_text).SetSource(Verbs::Do(10).SetSource(5))
		);

		WHEN("Serialize and then deserialize container in binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(stuff);
			auto deserialized = Verbs::Interpret::To<Any>(serialized);

			THEN("Deserialized data must match original stuff completely") {
				REQUIRE(stuff == deserialized);
			}
		}

		/*WHEN("Serialize and then deserialize container in Code") {
			auto serialized = pcSerialize<Code>(stuff);
			auto deserialized = pcDeserialize(serialized);

			THEN("Deserialized data must match original stuff completely") {
				REQUIRE(stuff == deserialized);
			}
		}*/
	}
}
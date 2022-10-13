#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Serialization", "[serialization]") {
	GIVEN("An empty Any instance") {
		Any pack;

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);
			THEN("The serialized binary pack must match some criteria") {
				REQUIRE(serialized.Is<Byte>());
			}

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("An Any instance containing other Any instances") {
		Any pack;
		pack << Any {1} << Any {2} << Any {3};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("A TAny instance containing other Any instances") {
		TAny<Any> pack;
		pack << Any {1} << Any {2} << Any {3};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("An Any instance containing Text") {
		Any pack;
		pack
			<< "hello"_text
			<< "i love you"_text
			<< "won't you tell me your name"_text;

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("An Any instance containing Trait") {
		Any pack;
		pack
			<< Traits::Name("hello"_text)
			<< Traits::Name("i love you"_text)
			<< Traits::Name("won't you tell me your name"_text);

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("An Any instance containing a verb") {
		Any pack = Verbs::Do(10).SetSource(5);

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("An Any instance containing various kinds of numbers") {
		auto pack = Any::Wrap(10, 5, 20.0f, 40.0);

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);
				THEN("The deserialized binary pack must be completely identical with the original") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}

	GIVEN("A complex pack with various kinds of data") {
		auto pack = Any::Wrap(
			"some text"_text,
			10, 5, 20.0f, 40.0,
			Verbs::Do(10).SetSource(5),
			Verbs::Do(10).SetSource("some other text"_text),
			Verbs::Do("even more text"_text).SetSource(Verbs::Do(10).SetSource(5))
		);

		WHEN("Serialize and then deserialize container in binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Any>(serialized);

				THEN("Deserialized data must match original stuff completely") {
					REQUIRE(deserialized == pack);
				}
			#endif
		}
	}
}
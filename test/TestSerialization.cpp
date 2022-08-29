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

			auto deserialized = Verbs::Interpret::To<Any>(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("An Any instance containing other Any instances") {
		Any pack;
		pack << Any {1} << Any {2} << Any {3};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);
			auto deserialized = Verbs::Interpret::To<Any>(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("A TAny instance containing other Any instances") {
		TAny<Any> pack;
		pack << Any {1} << Any {2} << Any {3};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);
			auto deserialized = Verbs::Interpret::To<Any>(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
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
			auto deserialized = Verbs::Interpret::To<Any>(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
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
			auto deserialized = Verbs::Interpret::To<Any>(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}
}
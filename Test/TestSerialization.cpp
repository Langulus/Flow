#include "TestMain.hpp"
#include <catch2/catch.hpp>

SCENARIO("Serialization", "[serialization]") {
	GIVEN("An empty Any instance") {
		Any pack;

		WHEN("Pack is serialized as binary") {
			auto serialized = pcSerialize<Bytes, true>(pack);
			THEN("The serialized binary pack must match some criteria") {
				REQUIRE(serialized.Is<pcbyte>());
			}

			auto deserialized = pcDeserialize(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("An Any instance containing other Any instances") {
		Any pack;
		pack << Any{ 1 } << Any{ 2 } << Any{ 3 };

		WHEN("Pack is serialized as binary") {
			auto serialized = pcSerialize<Bytes, true>(pack);
			auto deserialized = pcDeserialize(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("A TAny instance containing other Any instances") {
		TAny<Any> pack;
		pack << Any{ 1 } << Any{ 2 } << Any{ 3 };

		WHEN("Pack is serialized as binary") {
			auto serialized = pcSerialize<Bytes, true>(pack);
			auto deserialized = pcDeserialize(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("An Any instance containing Text") {
		Any pack;
		pack << Text{ "hello" } << Text{ "i love you" } << Text{ "won't you tell me your name" };

		WHEN("Pack is serialized as binary") {
			auto serialized = pcSerialize<Bytes, true>(pack);
			auto deserialized = pcDeserialize(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}

	GIVEN("An Any instance containing Trait") {
		Any pack; pack 
			<< Trait::From<Traits::Name>(Text{ "hello" })
			<< Trait::From<Traits::Name>(Text{ "i love you" })
			<< Trait::From<Traits::Name>(Text{ "won't you tell me your name" });

		WHEN("Pack is serialized as binary") {
			auto serialized = pcSerialize<Bytes, true>(pack);
			auto deserialized = pcDeserialize(serialized);
			THEN("The deserialized binary pack must be completely identical with the original") {
				REQUIRE(deserialized == pack);
			}
		}
	}
}
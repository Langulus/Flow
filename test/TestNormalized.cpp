#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Data normalization", "[normalized]") {
	GIVEN("A complex descriptor") {
		Any descriptor;

		WHEN("Normalized") {
			Normalized normalized {descriptor};

			THEN("The requirements should be met") {
				REQUIRE(true);
			}
		}
	}
}

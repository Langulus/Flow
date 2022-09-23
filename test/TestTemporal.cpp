#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Temporal flow", "[temporal]") {
	///																								
	/// The following tests rely on this ontology sequence							
	/*const auto HI = "? create Entity(User)"_code.Parse();
	const auto COMMA = "(number? >< future number?) or (? Conjunct!4 future?)"_code.Parse();
	const auto DOT = "(number? + Fraction(future number?)) or (? Conjunct!8 future?)"_code.Parse();
	const auto MY = "?.Entity(User).future?"_code.Parse();
	const auto NAME = "Traits::Name"_code.Parse();
	const auto IS = "? = future?"_code.Parse();
	const auto APOSTROPHE_S = "(? = future?) or (?.Entity(Session or User)) or (?.future?)"_code.Parse();
	const auto MAKE = "? create future?"_code.Parse();
	const auto AA = "single"_code.Parse();
	const auto GAME = "Entity(Universe, Window, Temporal)"_code.Parse();
	const auto CALLED = "? create Name(future text?)"_code.Parse();

	GIVEN("An empty temporal flow") {
		Temporal flow;

		WHEN("Code is pushed to the flow") {
			flow.Push(HI);
			flow.Push(COMMA);
			flow.Push(" ");
			flow.Push(MY);
			flow.Push(" ");
			flow.Push(NAME);
			flow.Push(" ");
			flow.Push(IS);
			flow.Push(" Dimo");
			flow.Push(DOT);
			flow.Push("let");
			flow.Push(APOSTROPHE_S);
			flow.Push(" ");
			flow.Push(MAKE);
			flow.Push(" ");
			flow.Push(AA);
			flow.Push(" ");
			flow.Push(GAME);
			flow.Push(" ");
			flow.Push(CALLED);
			flow.Push(" t");
			flow.Push("e");
			flow.Push("st");
			flow.Push(" game");
			flow.Push(GAME);

			THEN("The generated temporal flow should be correct") {

			}
		}
	}*/
}

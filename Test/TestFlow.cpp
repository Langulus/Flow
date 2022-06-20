#include "TestMain.hpp"
#include <catch2/catch.hpp>

#define DUMP_STUFF \
	pcLogSpecial << "-------------"; \
	pcLogSpecial << "Script: " << gasm; \
	pcLogSpecial << "Parsed: " << parsed; \
	pcLogSpecial << "Requir: " << required; \
	pcLogSpecial << "-------------";


SCENARIO("Parsing GASM", "[gasm]") {
	GIVEN("1) The GASM script: associate(`plural` > iMany)") {
		const GASM gasm = "associate(`plural` > iMany)";
		TAny<Any> package = Any::Wrap(Text("plural"), uiMany);
		package[0].MakeLeft();
		package[1].MakeRight();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("2) The GASM script: associate(`thing` > [Scope!-1(Entity: ? > ?)])") {
		const GASM gasm = "associate(`thing` > [Scope!-1(Entity: ? > ?)])";
		TAny<Any> package = Any::Wrap(Text("thing"), GASM("Scope!-1(Entity: ? > ?)"));
		package[0].MakeLeft();
		package[1].MakeRight();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("3) The GASM script: associate(`things` > (\"thing\", `plural`))") {
		const GASM gasm = "associate(`things` > (\"thing\", `plural`))";
		TAny<Any> package = Any::Wrap(Text("things"), Any::WrapOne(Text("thing"), Text("plural")));
		package[0].MakeLeft();
		package[1].MakeRight();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("4) The GASM script: associate(',' > ([Catenate(<ANumber?: >ANumber?)] or iSingle or \"and\"))") {
		const GASM gasm = "associate(',' > ([Catenate(<ANumber?: >ANumber?)] or iSingle or \"and\"))";
		TAny<Any> package = Any::Wrap(char8(','), Any::Wrap(GASM("Catenate(<ANumber?: >ANumber?)"), uiSingle, Text("and")));
		package[0].MakeLeft();
		package[1].MakeRight().MakeOr();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("5) The GASM script: Catenate   (   <ANumber?	:	>ANumber?	)   ") {
		const GASM gasm = "Catenate   (   <ANumber?	:	>ANumber?	)   ";
		TAny<Any> package = Any::Wrap(ANumber::ID, ANumber::ID);
		package[0].MakeLeft().MakeMissing();
		package[1].MakeRight().MakeMissing();
		const Any required = Verb::From<Verbs::Catenate>(package[0], package[1]);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("6) The GASM script: Scope!-1(Verb: ? > ?)") {
		const GASM gasm = "Scope!-1(Verb: ? > ?)";
		TAny<Any> package = Any::Wrap(Any(), Any());
		package[0].MakeLeft().MakeMissing();
		package[1].MakeRight().MakeMissing();
		const Any required = Verb::From<Verbs::Scope>(DataID::Of<Verb>, package).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("7) The GASM script: associate(`is` > (<? = ?>))") {
		const GASM gasm = "associate(`is` > (<? = ?>))";
		TAny<Any> package = Any::Wrap(
			Text("is"),
			Verb::From<Verbs::Associate>(
				Any().MakeLeft().MakeMissing(),
				Any().MakeRight().MakeMissing()
			).SetPriority(2)
		);
		package[0].MakeLeft();
		package[1].MakeRight();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("8) The GASM script: .Verb.(>? as Context)") {
		const GASM gasm = ".Verb.(>? as Context)";
		Any required = Verb::From<Verbs::Select>(
			Verb::From<Verbs::Select>({}, DataID::Of<Verb>),
			Any().MakeMissing().MakeRight(),
			Traits::Context::ID
		);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("9) The GASM script: Scope!-1(Verb: <?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct))") {
		const GASM gasm = "Scope!-1(Verb: <?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct))";
		Any required = Verb::From<Verbs::Scope>(
			DataID::Of<Verb>,
			Any::WrapOne(
				Any::WrapOne(ANumber::ID, DataID::Of<DataID>, DataID::Of<Construct>).MakeMissing().MakeLeft(),
				Any::WrapOne(ANumber::ID, DataID::Of<DataID>, DataID::Of<Construct>).MakeMissing().MakeRight()
			)
		).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("10) The GASM script: Create^1(Count(1)).Add^3(2)") {
		const GASM gasm = "Create^1(Count(1)).Add^3(2)";
		Any required = Verb::From<Verbs::Add>(
			Verb::From<Verbs::Create>(
				{}, 
				Any{ Trait::From<Traits::Count>(real(1)) }
			).SetFrequency(1),
			real(2)
		).SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("11) The GASM script: Create^1(Count(1)).Add^2(2).Multiply^3(4)") {
		const GASM gasm = "Create^1(Count(1)).Add^2(2).Multiply^3(4)";
		Any required = Verb::From<Verbs::Multiply>(
			Verb::From<Verbs::Add>(
				Verb::From<Verbs::Create>(
					{}, 
					Any{ Trait::From<Traits::Count>(real(1)) }
				).SetFrequency(1),
				real(2)
			).SetFrequency(2),
			real(4)
		).SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("12) The GASM script: -(2 * 8.75 - 14 ^ 2)") {
		const GASM gasm = "-(2 * 8.75 - 14 ^ 2)";

		WHEN("Parsed without optimization") {
			Any required = Verb::From<Verbs::Add>(
				Verb::From<Verbs::Add>(
					Verb::From<Verbs::Multiply>(real(2), real(8.75)),
					Verb::From<Verbs::Exponent>(real(14), real(2))
				).SetMass(-1)
			).SetMass(-1);

			const auto parsed = gasm.Parse(false);
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
		WHEN("Parsed with optimization") {
			Any required = real(178.5);
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}
}
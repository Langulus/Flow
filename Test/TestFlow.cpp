#include "Main.hpp"
#include <catch2/catch.hpp>

#define DUMP_STUFF \
	Logger::Special() << "-------------"; \
	Logger::Special() << "Script: " << gasm; \
	Logger::Special() << "Parsed: " << parsed; \
	Logger::Special() << "Requir: " << required; \
	Logger::Special() << "-------------";


SCENARIO("Parsing Code", "[gasm]") {
	GIVEN("1) The Code script: associate(`plural` > iMany)") {
		const Code gasm = "associate(`plural` > iMany)";
		TAny<Any> package = Any::Wrap(Text("plural"), Index::Many);
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("2) The Code script: associate(`thing` > [Scope!-1(Entity: ? > ?)])") {
		const Code gasm = "associate(`thing` > [Scope!-1(Entity: ? > ?)])";
		TAny<Any> package = Any::Wrap(Text("thing"), Code("Scope!-1(Entity: ? > ?)"));
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("3) The Code script: associate(`things` > (\"thing\", `plural`))") {
		const Code gasm = "associate(`things` > (\"thing\", `plural`))";
		TAny<Any> package = Any::Wrap(Text("things"), Any::WrapOne(Text("thing"), Text("plural")));
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("4) The Code script: associate(',' > ([Catenate(<ANumber?: >ANumber?)] or iSingle or \"and\"))") {
		const Code gasm = "associate(',' > ([Catenate(<ANumber?: >ANumber?)] or iSingle or \"and\"))";
		TAny<Any> package = Any::Wrap(char8(','), Any::Wrap(Code("Catenate(<ANumber?: >ANumber?)"), uiSingle, Text("and")));
		package[0].MakePast();
		package[1].MakeFuture();
		package[1].MakeOr();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("5) The Code script: Catenate   (   <ANumber?	:	>ANumber?	)   ") {
		const Code gasm = "Catenate   (   <ANumber?	:	>ANumber?	)   ";
		TAny<Any> package = Any::Wrap(A::Number::ID, ANumber::ID);
		package[0].MakePast();
		package[0].MakeMissing();
		package[1].MakeFuture();
		package[1].MakeMissing();
		const Any required = Verb::From<Verbs::Catenate>(package[0], package[1]);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("6) The Code script: Scope!-1(Verb: ? > ?)") {
		const Code gasm = "Scope!-1(Verb: ? > ?)";
		TAny<Any> package = Any::Wrap(Any(), Any());
		package[0].MakePast();
		package[0].MakeMissing();
		package[1].MakeFuture();
		package[1].MakeMissing();
		const Any required = Verb::From<Verbs::Scope>(DataID::Of<Verb>, package).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("7) The Code script: associate(`is` > (<? = ?>))") {
		const Code gasm = "associate(`is` > (<? = ?>))";
		TAny<Any> package = Any::Wrap(
			Text("is"),
			Verb::From<Verbs::Associate>(
				Any().MakePast().MakeMissing(),
				Any().MakeFuture().MakeMissing()
			).SetPriority(2)
		);
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verb::From<Verbs::Associate>({}, package);

		WHEN("Parsed") {
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("8) The Code script: .Verb.(>? as Context)") {
		const Code gasm = ".Verb.(>? as Context)";
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

	GIVEN("9) The Code script: Scope!-1(Verb: <?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct))") {
		const Code gasm = "Scope!-1(Verb: <?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct))";
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

	GIVEN("10) The Code script: Create^1(Count(1)).Add^3(2)") {
		const Code gasm = "Create^1(Count(1)).Add^3(2)";
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

	GIVEN("11) The Code script: Create^1(Count(1)).Add^2(2).Multiply^3(4)") {
		const Code gasm = "Create^1(Count(1)).Add^2(2).Multiply^3(4)";
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

	GIVEN("12) The Code script: -(2 * 8.75 - 14 ^ 2)") {
		const Code gasm = "-(2 * 8.75 - 14 ^ 2)";

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
			Any required = Real(178.5);
			const auto parsed = gasm.Parse();
			DUMP_STUFF;
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}
}
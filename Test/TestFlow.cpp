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
		const Any required = Verbs::Associate({}, package);

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
		const Any required = Verbs::Associate({}, package);

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
		TAny<Any> package = Any::Wrap(Text("things"), Any::WrapCommon(Text("thing"), Text("plural")));
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verbs::Associate({}, package);

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
		TAny<Any> package = Any::Wrap(',', Any::Wrap(Code("Catenate(<ANumber?: >ANumber?)"), Index::Single, Text("and")));
		package[0].MakePast();
		package[1].MakeFuture();
		package[1].MakeOr();
		const Any required = Verbs::Associate({}, package);

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
		TAny<Any> package = Any::Wrap(MetaData::Of<A::Number>(), MetaData::Of<A::Number>());
		package[0].MakePast();
		package[0].MakeMissing();
		package[1].MakeFuture();
		package[1].MakeMissing();
		const Any required = Verbs::Catenate(package[0], package[1]);

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
		const Any required = Verbs::Scope(MetaData::Of<Verb>(), package).SetPriority(-1);

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
			Verbs::Associate(
				Any().MakePast().MakeMissing(),
				Any().MakeFuture().MakeMissing()
			).SetPriority(2)
		);
		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verbs::Associate({}, package);

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
		Any required = Verbs::Select(
			Verbs::Select({}, MetaData::Of<Verb>()),
			Any().MakeMissing().MakeRight(),
			Traits::Context()
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
		Any required = Verbs::Scope(
			MetaData::Of<Verb>(),
			Any::WrapCommon(
				Any::WrapCommon(MetaData::Of<A::Number>(), MetaData::Of<MetaData>(), MetaData::Of<Construct>()).MakeMissing().MakeLeft(),
				Any::WrapCommon(MetaData::Of<A::Number>(), MetaData::Of<MetaData>(), MetaData::Of<Construct>()).MakeMissing().MakeRight()
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
		Any required = Verbs::Add(
			Verbs::Create({}, Any {Traits::Count(Real(1))}).SetFrequency(1),
			Real(2)
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
		Any required = Verbs::Multiply(
			Verbs::Add(
				Verbs::Create({}, Any {Traits::Count(Real(1))}).SetFrequency(1),
				Real(2)
			).SetFrequency(2),
			Real(4)
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
			Any required = Verbs::Add(
				Verbs::Add(
					Verbs::Multiply(Real(2), Real(8.75)),
					Verbs::Exponent(Real(14), Real(2))
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
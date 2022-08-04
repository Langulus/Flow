#include "Main.hpp"
#include <catch2/catch.hpp>


template<class INPUT, class OUTPUT, class REQUIRED>
void DumpResults(const INPUT& in, const OUTPUT& out, const REQUIRED& required) {
	Logger::Special() << "-------------";
	Logger::Special() << "Script:   " << in;
	Logger::Special() << "Parsed:   " << out;
	Logger::Special() << "Required: " << required;
	Logger::Special() << "-------------";
}

SCENARIO("Parsing scripts with corner cases", "[code]") {
	GIVEN("The script: `plural` associate many") {
		const auto code = "`plural` associate many"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `plural` associate(many)") {
		const auto code = "`plural` associate(many)"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (`plural`) associate (many)") {
		const auto code = "(`plural`) associate (many)"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (`plural`) associate (many)") {
		const auto code = "(`plural`) associate many"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `plural` = many") {
		const auto code = "`plural` = many"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `plural` = (many)") {
		const auto code = "`plural` = (many)"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (`plural`) = (many)") {
		const auto code = "(`plural`) = (many)"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (`plural`) = many") {
		const auto code = "(`plural`) = many"_code;
		const Any required = Verbs::Associate(IndexMany)
			.SetSource("plural"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `thing` associate [Entity(past?, future?)]") {
		const auto code = "`thing` associate [Entity(past?, future?)]"_code;
		const Any required = Verbs::Associate("Entity(past?, future?)"_code)
			.SetSource("thing"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `things` associate(\"thing\", `plural`)") {
		const auto code = "`things` associate(\"thing\", `plural`)"_code;
		const Any required = Verbs::Associate(Any::WrapCommon("thing"_text, "plural"_text))
			.SetSource("things"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: `things` = (\"thing\", `plural`)") {
		const auto code = "`things` = (\"thing\", `plural`)"_code;
		const Any required = Verbs::Associate(Any::WrapCommon("thing"_text, "plural"_text))
			.SetSource("things"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: ',' = ([past number? >< future number?] or single or \"and\")") {
		const Code code = "',' = ([past number? >< future number?] or single or \"and\")";
		auto argument = Any::Wrap("past number? >< future number?"_code, IndexSingle, "and"_text);
		argument.MakeOr();
		const Any required = Verbs::Associate(argument)
			.SetSource(","_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: past number    ?       &   future     number ? ") {
		const Code code = "past number    ?       &   future     number ? ";
		Any source = MetaData::Of<A::Number>();
		source.MakePast();
		source.MakeMissing();
		Any argument = MetaData::Of<A::Number>();
		argument.MakeFuture();
		argument.MakeMissing();
		const Any required = Verbs::Catenate(argument)
			.SetSource(source);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("6) The Code script: Create!-1(Verb(? > ?))") {
		const Code code = "Create!-1(Verb(? > ?))";
		TAny<Any> package = Any::Wrap(Any(), Any());
		package[0].MakePast();
		package[0].MakeMissing();
		package[1].MakeFuture();
		package[1].MakeMissing();

		const Any required = Verbs::Create(
			Construct::From<Verb>(package)
		).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("7) The Code script: associate(`is` > (<? = ?>))") {
		Any pastMissing;
		pastMissing.MakePast();
		pastMissing.MakeMissing();

		Any futureMissing;
		futureMissing.MakeFuture();
		futureMissing.MakeMissing();

		const Code code = "associate(`is` > (<? = ?>))";
		TAny<Any> package = Any::Wrap(
			Text("is"),
			Verbs::Associate(futureMissing).SetPriority(2).SetSource(pastMissing)
		);

		package[0].MakePast();
		package[1].MakeFuture();
		const Any required = Verbs::Associate(package);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("8) The Code script: .Verb.(>? as Context)") {
		Any futureMissing;
		futureMissing.MakeFuture();
		futureMissing.MakeMissing();

		const Code code = ".Context = .Verb.(>?)";
		Any required = Verbs::Associate(
			Verbs::Select(futureMissing).SetSource(Verbs::Select(MetaData::Of<Verb>()))
		).SetSource(Verbs::Select(MetaTrait::Of<Traits::Context>()));

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("9) The Code script: Create!-1(Verb(<?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct)))") {
		Any pastMissing = Any::WrapCommon(
			MetaData::Of<A::Number>(),
			MetaData::Of<MetaData>(),
			MetaData::Of<Construct>()
		);
		pastMissing.MakePast();
		pastMissing.MakeMissing();

		Any futureMissing = Any::WrapCommon(
			MetaData::Of<A::Number>(), 
			MetaData::Of<MetaData>(), 
			MetaData::Of<Construct>()
		);
		futureMissing.MakeFuture();
		futureMissing.MakeMissing();

		const Code code = "Create!-1(Verb(<?(ANumber,DataID,Construct), >?(ANumber,DataID,Construct)))";
		Any required = Verbs::Create(
			Construct::From<Verb>(Any::WrapCommon(pastMissing, futureMissing))
		).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("10) The Code script: Create^1(Count(1)).Add^3(2)") {
		const Code code = "Create^1(Count(1)).Add^3(2)";
		Any required = Verbs::Add(
			Verbs::Create(Traits::Count(Real(1))).SetFrequency(1),
			Real(2)
		).SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("11) The Code script: Create^1(Count(1)).Add^2(2).Multiply^3(4)") {
		const Code code = "Create^1(Count(1)).Add^2(2).Multiply^3(4)";
		Any required = Verbs::Multiply(
			Verbs::Add(Real(2)).SetFrequency(2).SetSource(Verbs::Create(Traits::Count(Real(1))).SetFrequency(1)),
			Real(4)
		).SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("12) The Code script: -(2 * 8.75 - 14 ^ 2)") {
		const Code code = "-(2 * 8.75 - 14 ^ 2)";

		WHEN("Parsed without optimization") {
			Any required = Verbs::Add(
				Verbs::Add(
					Verbs::Exponent(Real(2)).SetSource(Real(14))
				).SetMass(-1).SetSource(Verbs::Multiply(Real(8.75)).SetSource(Real(2)))
			).SetMass(-1);

			const auto parsed = code.Parse(false);
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}

		WHEN("Parsed with optimization") {
			Any required = Real(178.5);
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}
}
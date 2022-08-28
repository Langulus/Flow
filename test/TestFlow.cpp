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
	Any pastMissing;
	pastMissing.MakePast();
	pastMissing.MakeMissing();

	Any futureMissing;
	futureMissing.MakeFuture();
	futureMissing.MakeMissing();


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
			.SetSource(',');

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create!-1(Verb(past?, future?))") {
		const Code code = "Create!-1(Verb(past?, future?))";
		TAny<Any> package = Any::Wrap(pastMissing, futureMissing);
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

	GIVEN("The script: past number    ?       ><  future     number ? ") {
		const Code code = "past number    ?       ><   future     number ? ";
		Any source {pastMissing};
		source << MetaData::Of<A::Number>();
		Any argument {futureMissing};
		argument << MetaData::Of<A::Number>();
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

	GIVEN("The script: `is` = (past? = future?)") {
		const Code code = "`is` = (past? = future?)";
		const auto package = 
			Verbs::Associate(futureMissing).SetSource(pastMissing);
		const Any required = 
			Verbs::Associate(package).SetSource("is"_text);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: .Context = .Verb.Future?") {
		const Code code = ".Context = .Verb.Future?";
		const Any required = Verbs::Associate(
			Verbs::Select(futureMissing).SetSource(
				Verbs::Select(MetaData::Of<Verb>())
			)
		).SetSource(
			Verbs::Select(MetaTrait::Of<Traits::Context>())
		);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create!-1(Verb(past?(Number,DMeta,Construct), future?(Number,DMeta,Construct)))") {
		Any a1 {pastMissing};
		a1
			<< MetaData::Of<A::Number>()
			<< MetaData::Of<MetaData>()
			<< MetaData::Of<Construct>();

		Any a2 {futureMissing};
		a2
			<< MetaData::Of<A::Number>()
			<< MetaData::Of<MetaData>()
			<< MetaData::Of<Construct>();

		const Code code = "Create!-1(Verb(past?(Number,DMeta,Construct), future?(Number,DMeta,Construct)))";
		const Any required = Verbs::Create(
			Construct::From<Verb>(Any::WrapCommon(a1, a2))
		).SetPriority(-1);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create^1(Count(1)) Add^3 2") {
		const Code code = "Create^1(Count(1)) Add^3 2";
		Any required = Verbs::Add(Real(2))
			.SetSource(
				Verbs::Create(Traits::Count(Real(1)))
					.SetFrequency(1))
			.SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create^1(Count(1)) Add^3(2)") {
		const Code code = "Create^1(Count(1)) Add^3(2)";
		Any required = Verbs::Add(Real(2))
			.SetSource(
				Verbs::Create(Traits::Count(Real(1)))
					.SetFrequency(1))
			.SetFrequency(3);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create^1(Count(1)) Add^2(2) Multiply^3(4)") {
		const Code code = "Create^1(Count(1)) Add^2(2) Multiply^3(4)";
		Any required = Verbs::Add(
			Verbs::Multiply(Real(4))
				.SetFrequency(3)
				.SetSource(Real(2)))
			.SetFrequency(2)
			.SetSource(
				Verbs::Create(Traits::Count(Real(1)))
				.SetFrequency(1)
			);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Create^1(Count(1)) + 2 * 4") {
		const Code code = "Create^1(Count(1)) + 2 * 4";
		Any required = Verbs::Add(
				Verbs::Multiply(Real(4))
				.SetSource(Real(2)))
			.SetSource(
				Verbs::Create(Traits::Count(Real(1)))
				.SetFrequency(1)
			);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: -(2 * 8.75 - 14 ^ 2)") {
		const Code code = "-(2 * 8.75 - 14 ^ 2)";

		WHEN("Parsed without optimization") {
			Any required = Verbs::Add(
				Verbs::Add(
					Verbs::Exponent(Real(2)).SetSource(Real(14))
				)
				.SetMass(-1)
				.SetSource(Verbs::Multiply(Real(8.75)).SetSource(Real(2)))
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
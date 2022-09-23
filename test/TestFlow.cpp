#include "Main.hpp"
#include <catch2/catch.hpp>

/// See https://github.com/catchorg/Catch2/blob/devel/docs/tostring.md			
CATCH_TRANSLATE_EXCEPTION(::Langulus::Exception const& ex) {
	const Text serialized {ex};
	return ::std::string {Token {serialized}};
}

/// Dump parse results and requirements													
template<class INPUT, class OUTPUT, class REQUIRED>
void DumpResults(const INPUT& in, const OUTPUT& out, const REQUIRED& required) {
	Logger::Special() << "-------------";
	Logger::Special() << "Script:   " << in;
	Logger::Special() << "Parsed:   " << out;
	Logger::Special() << "Required: " << required;
	Logger::Special() << "-------------";
}


SCENARIO("Parsing scripts with corner cases", "[flow]") {
	Any pastMissing;
	pastMissing.MakePast();

	Any futureMissing;
	futureMissing.MakeFuture();


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

	GIVEN("The script: ',' = ([number? >< number??] or single or \"and\")") {
		const Code code = "',' = ([number? >< number??] or single or \"and\")";
		auto argument = Any::Wrap("number? >< number??"_code, IndexSingle, "and"_text);
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

	GIVEN("The script: Create!-1(Verb(?, ??))") {
		const Code code = "Create!-1(Verb(?, ??))";
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

	GIVEN("The script: number    ?       ><       number ?? ") {
		const Code code = "number    ?       ><     number ?? ";
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

	GIVEN("The script: `is` = (? = ??)") {
		const Code code = "`is` = (? = ??)";
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

	GIVEN("The script: .Context = .Verb.??") {
		const Code code = ".Context = .Verb.??";
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

	GIVEN("The script: Create!-1(Verb(?(Number,DMeta,Construct), ??(Number,DMeta,Construct)))") {
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

		const Code code = "Create!-1(Verb(?(Number,DMeta,Construct), ??(Number,DMeta,Construct)))";
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
				Verbs::Multiply(Real(4)).SetSource(Real(2)).SetFrequency(3)
			)
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
		Any required = Verbs::Add(Real(8))
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

	GIVEN("The script: ? create Entity(User)") {
		const Code code = "? create Entity(User)";

		WHEN("Parsed") {
			Any required = Verbs::Create(
				Construct::From<Entity>(
					MetaData::Of<User>()
				)
			).SetSource(pastMissing);

			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (number? >< number??) or (? Conjunct!4 ??)") {
		const Code code = "(number? >< number??) or (? Conjunct!4 ??)";

		WHEN("Parsed") {
			Any pastNumber {pastMissing};
			pastNumber << MetaData::Of<A::Number>();
			Any futrNumber {futureMissing};
			futrNumber << MetaData::Of<A::Number>();

			Verbs::Catenate catenate(futrNumber);
			catenate.SetSource(pastNumber);

			Verbs::Conjunct conjunct(futureMissing);
			conjunct.SetSource(pastMissing);
			conjunct.SetPriority(4);

			Any required = Any::WrapCommon<Verb>(catenate, conjunct);
			required.MakeOr();

			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	/*
	const auto COMMA = "(number? >< future number?) or (? Conjunct!4 future?)"_code.Parse();
	const auto DOT = "(number? + Fraction(future number?)) or (? Conjunct!8 future?)"_code.Parse();
	const auto MY = "?.Entity(User).future?"_code.Parse();
	const auto NAME = "Traits::Name"_code.Parse();
	const auto IS = "? = future?"_code.Parse();
	const auto APOSTROPHE_S = "(? = future?) or (?.Entity(Session or User)) or (?.future?)"_code.Parse();
	const auto MAKE = "? create future?"_code.Parse();
	const auto AA = "single"_code.Parse();
	const auto GAME = "Entity(Universe, Window, Temporal)"_code.Parse();
	const auto CALLED = "? create Name(future text?)"_code.Parse();*/

}
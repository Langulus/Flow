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
	Logger::Special("-------------");
	Logger::Special("Script:   ", in);
	Logger::Special("Parsed:   ", out);
	Logger::Special("Required: ", required);
	Logger::Special("-------------");
}


SCENARIO("Parsing scripts with corner cases", "[flow]") {
	Any pastMissing;
	pastMissing.MakePast();

	Any futureMissing;
	futureMissing.MakeFuture();


	GIVEN("The script: `plural` associate many") {
		const auto code = "`plural` associate many"_code;
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {IndexMany}
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
		const Any required = Verbs::Associate {"Entity(past?, future?)"_code}
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
		const Any required = Verbs::Associate {"thing"_text, "plural"_text}
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
		const Any required = Verbs::Associate {"thing"_text, "plural"_text}
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
		Any argument {
			"number? >< number??"_code, 
			IndexSingle, 
			"and"_text
		};
		argument.MakeOr();
		const Any required = Verbs::Associate {argument}
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
		TAny<Any> package {pastMissing, futureMissing};
		const Any required = Verbs::Create {
			Construct::From<Verb>(package)
		}.SetPriority(-1);

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
		const Any required = Verbs::Catenate {argument}
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
		const Any package = Verbs::Associate {futureMissing}
			.SetSource(pastMissing);
		const Any required = Verbs::Associate {package}
			.SetSource("is"_text);

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
		const Any package = Verbs::Select {futureMissing}
			.SetSource(Verbs::Select {MetaData::Of<Verb>()});
		const Any required = Verbs::Associate {package}
			.SetSource(Verbs::Select {MetaTrait::Of<Traits::Context>()});

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
		const Any required = Verbs::Create {
			Construct::From<Verb>(Any {a1, a2})
		}.SetPriority(-1);

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
		const Any required = Verbs::Add {Real(2)}
			.SetSource(
				Verbs::Create {Traits::Count {Real(1)}}
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
		const Any required = Verbs::Add {Real(2)}
			.SetSource(
				Verbs::Create {Traits::Count {Real(1)}}
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
		const Any multiply = Verbs::Multiply {Real(4)}
			.SetSource(Real(2))
			.SetFrequency(3);
		const Any required = Verbs::Add {multiply}
			.SetFrequency(2)
			.SetSource(
				Verbs::Create {Traits::Count {Real(1)}}
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
		const Any required = Verbs::Add {Real(8)}
			.SetSource(
				Verbs::Create {Traits::Count {Real(1)}}
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
		const Any exponent = Verbs::Exponent {Real(2)}
			.SetSource(Real(14));
		const Any addition = Verbs::Add {exponent}
			.SetMass(-1)
			.SetSource(
				Verbs::Multiply {Real(8.75)}
					.SetSource(Real(2))
			);
		const Any required = Verbs::Add {addition}
			.SetMass(-1);

		WHEN("Parsed without optimization") {
			const auto parsed = code.Parse(false);
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}

		WHEN("Parsed with optimization") {
			Any required2 = Real(178.5);
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required2);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required2);
			}
		}
	}

	GIVEN("The script: ? create Thing(User)") {
		const Code code = "? create Thing(User)";
		const Any required = Verbs::Create {
			Construct::From<Thing>(MetaData::Of<User>())
		}.SetSource(pastMissing);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (number? >< number??) or (? Conjunct!4 ??)") {
		const Code code = "(number? >< number??) or (? Conjunct!4 ??)";
		Any pastNumber {pastMissing};
		pastNumber << MetaData::Of<A::Number>();
		Any futrNumber {futureMissing};
		futrNumber << MetaData::Of<A::Number>();

		Verbs::Catenate catenate {futrNumber};
		catenate.SetSource(pastNumber);

		Verbs::Conjunct conjunct {futureMissing};
		conjunct.SetSource(pastMissing);
		conjunct.SetPriority(4);

		Any required = Any::WrapAs<Verb>(catenate, conjunct);
		required.MakeOr();

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (number? + Fraction(number??)) or (? Conjunct!8 ??)") {
		const Code code = "(number? + Fraction(number??)) or (? Conjunct!8 ??)";
		Any pastNumber {pastMissing};
		pastNumber << MetaData::Of<A::Number>();
		Any futrNumber {futureMissing};
		futrNumber << MetaData::Of<A::Number>();

		Verbs::Add add {Construct::From<Fraction>(futrNumber)};
		add.SetSource(pastNumber);

		Verbs::Conjunct conjunct {futureMissing};
		conjunct.SetSource(pastMissing);
		conjunct.SetPriority(8);

		Any required = Any::WrapAs<Verb>(add, conjunct);
		required.MakeOr();

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (? + Fraction(number??)) or (? Conjunct!8 ??)") {
		const Code code = "(? + Fraction(number??)) or (? Conjunct!8 ??)";
		Any futrNumber {futureMissing};
		futrNumber << MetaData::Of<A::Number>();

		Verbs::Add add {Construct::From<Fraction>(futrNumber)};
		add.SetSource(pastMissing);

		Verbs::Conjunct conjunct {futureMissing};
		conjunct.SetSource(pastMissing);
		conjunct.SetPriority(8);

		Any required = Any::WrapAs<Verb>(add, conjunct);
		required.MakeOr();

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: ?.Thing(User).??") {
		const Code code = "?.Thing(User).??";
		const Any required = Verbs::Select {futureMissing}
			.SetSource(
				Verbs::Select {
					Construct::From<Thing>(MetaData::Of<User>())
				}.SetSource(pastMissing)
			);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Traits::Name") {
		const Code code = "Traits::Name";
		const Any required = MetaTrait::Of<Traits::Name>();

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: ? = ??") {
		const Code code = "? = ??";
		const Any required = Verbs::Associate {futureMissing}
			.SetSource(pastMissing);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: (? = ??) or (?.Thing(Session or User)) or (?.??)") {
		const Code code = "(? = ??) or (?.Thing(Session or User)) or (?.??)";

		Any sessionOrUser {MetaData::Of<Session>(), MetaData::Of<User>()};
		sessionOrUser.MakeOr();

		Verbs::Associate first {futureMissing};
		first.SetSource(pastMissing);

		Verbs::Select second {Construct::From<Thing>(sessionOrUser)};
		second.SetSource(pastMissing);

		Verbs::Select third {futureMissing};
		third.SetSource(pastMissing);

		Any required = Any::WrapAs<Verb>(first, second, third);
		required.MakeOr();

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: ? create ??") {
		const Code code = "? create ??";
		const Any required = Verbs::Create {futureMissing}
			.SetSource(pastMissing);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: single") {
		const Code code = "single";
		const Any required = IndexSingle;

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: Thing(Universe, Window, Temporal)") {
		const Code code = "Thing(Universe, Window, Temporal)";
		const Any required = Construct::From<Thing>(
			MetaData::Of<Universe>(),
			MetaData::Of<Window>(),
			MetaData::Of<Temporal>()
		);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}

	GIVEN("The script: ? create Name(A::Text??)") {
		const Code code = "? create Name(A::Text??)";
		Any missingFutureText {futureMissing};
		missingFutureText << MetaData::Of<A::Text>();
		const Any required = Verbs::Create {Traits::Name {missingFutureText}}
			.SetSource(pastMissing);

		WHEN("Parsed") {
			const auto parsed = code.Parse();
			DumpResults(code, parsed, required);
			THEN("The parsed contents must match the requirements") {
				REQUIRE(parsed == required);
			}
		}
	}
}
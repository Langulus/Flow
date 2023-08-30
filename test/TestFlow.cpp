#include "Main.hpp"
#include <Flow/Temporal.hpp>
#include <Flow/Verbs/Associate.hpp>
#include <Flow/Verbs/Create.hpp>
#include <Flow/Verbs/Select.hpp>
#include <Flow/Verbs/Catenate.hpp>
#include <Flow/Verbs/Conjunct.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include <catch2/catch.hpp>

LANGULUS_EXCEPTION_HANDLER

SCENARIO("Parsing scripts with corner cases", "[flow]") {
   Any pastMissing;
   pastMissing.MakePast();

   Any futureMissing;
   futureMissing.MakeFuture();

   // Required for constants, such as single, many, etc.                
   (void)MetaOf<Index>();

   GIVEN("The script with line comments") {
      const auto code = "//some comment\n`plural` associate many //same line comment with no new line"_code;
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
   
   GIVEN("The script with /**/ comments") {
      const auto code = "/*some comment*/`plural` /*inbetween*/ associate many /*bad comment at the end"_code;
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
      const Any required = Verbs::Create {Any {Verb {package}}}
         .SetPriority(-1);

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
      source << MetaOf<A::Number>();
      Any argument {futureMissing};
      argument << MetaOf<A::Number>();
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
         .SetSource(Verbs::Select {MetaOf<Verb>()});
      const Any required = Verbs::Associate {package}
         .SetSource(Verbs::Select {MetaOf<Traits::Context>()});

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
      a1 << MetaOf<A::Number>()
         << MetaOf<MetaData>()
         << MetaOf<Construct>();

      Any a2 {futureMissing};
      a2 << MetaOf<A::Number>()
         << MetaOf<MetaData>()
         << MetaOf<Construct>();

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

   GIVEN("The script: ? create Thing(User)") {
      const Code code = "? create Thing(User)";
      const Any required = Verbs::Create {
         Construct::From<Thing>(MetaOf<User>())
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
      pastNumber << MetaOf<A::Number>();
      Any futrNumber {futureMissing};
      futrNumber << MetaOf<A::Number>();

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

   GIVEN("The script: ?.Thing(User).??") {
      const Code code = "?.Thing(User).??";
      const Any required = Verbs::Select {futureMissing}
         .SetSource(
            Verbs::Select {
               Construct::From<Thing>(MetaOf<User>())
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
      const Any required = Traits::Name::GetTrait();

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

      Any sessionOrUser {MetaOf<Session>(), MetaOf<User>()};
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
         MetaOf<Universe>(),
         MetaOf<Window>(),
         MetaOf<Temporal>()
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
      missingFutureText << MetaOf<A::Text>();
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

   GIVEN("The script: ? create @ 1 ! 2 ^ 3 * 5 Name(A::Text??)") {
      const Code code = "? create @ 1 ! 2 ^ 3 * 5 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(1_real).SetPriority(2_real).SetRate(3_real).SetMass(5_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }

   GIVEN("The script: ? create @ 1.66 ! 2.11 ^ 3.22 * 5.33 Name(A::Text??)") {
      const Code code = "? create @ 1.66 ! 2.11 ^ 3.22 * 5.33 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(1.66_real).SetPriority(2.11_real).SetRate(3.22_real).SetMass(5.33_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }

   GIVEN("The script: ? create@0.2!0.1^0.3*0.4 Name(A::Text??)") {
      const Code code = "? create@0.2!0.1^0.3*0.4 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(0.2_real).SetPriority(0.1_real).SetRate(0.3_real).SetMass(0.4_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }

   GIVEN("The script: ? create@1!2^3*4 Name(A::Text??)") {
      const Code code = "? create@1!2^3*4 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(1_real).SetPriority(2_real).SetRate(3_real).SetMass(4_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }

   GIVEN("The script: ? create@1.66!2.22^0.04*0.05 Name(A::Text??)") {
      const Code code = "? create@1.66!2.22^0.04*0.05 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(1.66_real).SetPriority(2.22_real).SetRate(0.04_real).SetMass(0.05_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }

   GIVEN("The script: ? create@0.2!0.3^0.4*0.5 Name(A::Text??)") {
      const Code code = "? create@0.2!0.3^0.4*0.5 Name(A::Text??)";
      Any missingFutureText {futureMissing};
      missingFutureText << MetaOf<A::Text>();
      const Any required = Verbs::Create {Traits::Name {missingFutureText}}
         .SetSource(pastMissing)
         .SetTime(0.2_real).SetPriority(0.3_real).SetRate(0.4_real).SetMass(0.5_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         THEN("The parsed contents must match the requirements") {
            REQUIRE(parsed == required);
         }
      }
   }
}
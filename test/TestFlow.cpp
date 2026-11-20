///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"
#include <Langulus/Testing/Dumping.hpp>
#include <Langulus/Temporal.hpp>
#include <Langulus/Verbs/Associate.hpp>
#include <Langulus/Verbs/Create.hpp>
#include <Langulus/Verbs/Select.hpp>
#include <Langulus/Verbs/Catenate.hpp>
#include <Langulus/Verbs/Conjunct.hpp>
#include <Langulus/Verbs/Interpret.hpp>


SCENARIO("Parsing scripts with corner cases") {
   static MemoryState memoryState;

   static_assert(CT::Complete<Temporal>);

   // Required for constants, such as single, many, etc.                
   //(void) MetaOf<Index>();

   GIVEN("The script with line comments") {
      const auto code = "//some comment\n`plural` associate index::many //same line comment with no new line"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural"_txt);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         //DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }
   
   GIVEN("The script with /**/ comments") {
      const auto code = "/*some comment*/`plural` /*inbetween*/ associate index::many /*bad comment at the end"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }
   
   GIVEN("The script: `plural` associate index::many") {
      const auto code = "`plural` associate index::many"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `plural` associate(index::many)") {
      const auto code = "`plural` associate(index::many)"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (`plural`) associate (index::many)") {
      const auto code = "(`plural`) associate (index::many)"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (`plural`) associate (index::many)") {
      const auto code = "(`plural`) associate index::many"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `plural` = index::many") {
      const auto code = "`plural` = index::many"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `plural` = (index::many)") {
      const auto code = "`plural` = (index::many)"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (`plural`) = (index::many)") {
      const auto code = "(`plural`) = (index::many)"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (`plural`) = index::many") {
      const auto code = "(`plural`) = index::many"_code;
      Many const required = Verbs::Associate {Index::Many}.In("plural");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `thing` associate { Entity(past?, future?) }") {
      const auto code = "`thing` associate { Entity(past?, future?) }"_code;
      Many const required = Verbs::Associate {" Entity(past?, future?) "_code}
         .In("thing");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `things` associate(\"thing\", `plural`)") {
      const auto code = "`things` associate(\"thing\", `plural`)"_code;
      Many const required = Verbs::Associate {"thing", "plural"}.In("things");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `things` = (\"thing\", `plural`)") {
      const auto code = "`things` = (\"thing\", `plural`)"_code;
      Many const required = Verbs::Associate {"thing", "plural"}.In("things");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ',' = ({number? >< number??} or single or \"and\")") {
      const Code code = "',' = ({number? >< number??} or single or \"and\")";
      Many argument {
         "number? >< number??"_code, 
         Index::Single, 
         "and"
      };
      argument.EnableOr();
      Many const required = Verbs::Associate {argument}.In(',');

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: Create!-1(Verb(?, ??))") {
      const Code code = "Create!-1(Verb(?, ??))";
      TMany<Many> package {Many::Past(), Many::Future()};
      Many const required = Verbs::Create {Many {Verb {package}}}
         .SetPrecedence(-1);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: number    ?       ><       number ?? ") {
      const Code code = "number    ?       ><     number ?? ";
      Many source = Many::Past();
      source << MetaOf<AbstractNumber>();
      Many argument = Many::Future();
      argument << MetaOf<AbstractNumber>();
      Many const required = Verbs::Catenate {argument}.In(source);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: `is` = (? = ??)") {
      const Code code = "`is` = (? = ??)";
      Many const package = Verbs::Associate {Many::Future()}.In(Many::Past());
      Many const required = Verbs::Associate {package}.In("is");

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: .Context = .Verb.??") {
      const Code code = ".Context = .Verb.??";
      Many const package = Verbs::Select {Many::Future()}
         .In(Verbs::Select {MetaOf<Verb>()});
      Many const required = Verbs::Associate {package}
         .In(Verbs::Select {MetaOf<Tags::Context>()});

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: Create!-1(Verb(?(Number,DMeta,Recipe), ??(Number,DMeta,Recipe)))") {
      Many a1 = Many::Past();
      a1 << MetaOf<AbstractNumber>()
         << MetaOf<DMeta>()
         << MetaOf<Recipe>();

      Many a2 = Many::Future();
      a2 << MetaOf<AbstractNumber>()
         << MetaOf<DMeta>()
         << MetaOf<Recipe>();

      const Code code = "Create!-1(Verb(?(Number,DMeta,Recipe), ??(Number,DMeta,Recipe)))";
      Many const required = Verbs::Create {
         Many {Verb {a1, a2}}
      }.SetPrecedence(-1);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create Thing(User)") {
      const Code code = "? create Thing(User)";
      Many const required = Verbs::Create {
         Recipe::For<Thing>(MetaOf<User>())
      }.In(Many::Past());

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (number? >< number??) or (? Conjunct!4 ??)") {
      const Code code = "(number? >< number??) or (? Conjunct!4 ??)";
      Many pastNumber = Many::Past();
      pastNumber << MetaOf<AbstractNumber>();
      Many futrNumber = Many::Future();
      futrNumber << MetaOf<AbstractNumber>();

      Verbs::Catenate catenate {futrNumber};
      catenate.In(pastNumber);

      Verbs::Conjunct conjunct {Many::Future()};
      conjunct.In(Many::Past());
      conjunct.SetPrecedence(4);

      Many required {catenate, conjunct};
      required.EnableOr();

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ?.Thing(User).??") {
      const Code code = "?.Thing(User).??";
      Many const required = Verbs::Select {Many::Future()}
         .In(
            Verbs::Select {
               Recipe::For<Thing>(MetaOf<User>())
            }.In(Many::Past())
         );

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: Tags::Name") {
      const Code code = "Tags::Name";
      Many const required = MetaOf<Tags::Name>();

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? = ??") {
      const Code code = "? = ??";
      Many const required = Verbs::Associate {Many::Future()}
         .In(Many::Past());

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: (? = ??) or (?.Thing(Session or User)) or (?.??)") {
      const Code code = "(? = ??) or (?.Thing(Session or User)) or (?.??)";

      Many sessionOrUser {MetaOf<Session>(), MetaOf<User>()};
      sessionOrUser.EnableOr();

      Verbs::Associate first {Many::Future()};
      first.In(Many::Past());

      Verbs::Select second {Recipe::For<Thing>(sessionOrUser)};
      second.In(Many::Past());

      Verbs::Select third {Many::Future()};
      third.In(Many::Past());

      Many required {first, second, third};
      required.EnableOr();

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create ??") {
      const Code code = "? create ??";
      Many const required = Verbs::Create {Many::Future()}
         .In(Many::Past());

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: single") {
      const Code code = "single";
      Many const required = Index::Single;

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: Thing(Universe, Window, Temporal)") {
      const Code code = "Thing(Universe, Window, Temporal)";
      Many const required = Recipe::For<Thing>(
         MetaOf<Universe>(),
         MetaOf<Window>(),
         MetaOf<Temporal>()
      );

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create Name(Text??)") {
      const Code code = "? create Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past());

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create @ 1 ! 2 ^ 3 * 5 Name(Text??)") {
      const Code code = "? create @ 1 ! 2 ^ 3 * 5 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(1_real)
         .SetPrecedence(2_real)
         .SetRate(3_real)
         .SetMass(5_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create @ 1.66 ! 2.11 ^ 3.22 * 5.33 Name(Text??)") {
      const Code code = "? create @ 1.66 ! 2.11 ^ 3.22 * 5.33 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(1.66_real)
         .SetPrecedence(2.11_real)
         .SetRate(3.22_real)
         .SetMass(5.33_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create@0.2!0.1^0.3*0.4 Name(Text??)") {
      const Code code = "? create@0.2!0.1^0.3*0.4 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(0.2_real)
         .SetPrecedence(0.1_real)
         .SetRate(0.3_real)
         .SetMass(0.4_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create@1!2^3*4 Name(Text??)") {
      const Code code = "? create@1!2^3*4 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(1_real)
         .SetPrecedence(2_real)
         .SetRate(3_real)
         .SetMass(4_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create@1.66!2.22^0.04*0.05 Name(Text??)") {
      const Code code = "? create@1.66!2.22^0.04*0.05 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(1.66_real)
         .SetPrecedence(2.22_real)
         .SetRate(0.04_real)
         .SetMass(0.05_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create@0.2!0.3^0.4*0.5 Name(Text??)") {
      const Code code = "? create@0.2!0.3^0.4*0.5 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(0.2_real)
         .SetPrecedence(0.3_real)
         .SetRate(0.4_real)
         .SetMass(0.5_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: ? create@-0.2!-0.3^-0.4*-0.5 Name(Text??)") {
      const Code code = "? create@-0.2!-0.3^-0.4*-0.5 Name(Text??)";
      Many missingFutureText = Many::Future();
      missingFutureText << MetaOf<Text>();
      Many const required = Verbs::Create {Tags::Name {missingFutureText}}
         .In(Many::Past())
         .SetTime(-0.2_real)
         .SetPrecedence(-0.3_real)
         .SetRate(-0.4_real)
         .SetMass(-0.5_real);

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
      }
   }

   GIVEN("The script: Input(`test`, 6, true)") {
      const Code code = "Input(`test`, 6, true)";
      Many const required = Tags::Input {"test", 6_real, true};

      WHEN("Parsed") {
         const auto parsed = code.Parse();
         DumpResults(code, parsed, required);
         REQUIRE(parsed == required);
         REQUIRE(parsed.GetCount() == 1);
         REQUIRE(parsed.IsExact<Tag>());
         REQUIRE(parsed.Get<Tag>()->IsExact<Many>());
         REQUIRE(parsed.Get<Tag>()->GetCount() == 3);
         REQUIRE(parsed.Get<Tag>()->GetAt<Many>(0)->IsExact<Text>());
         REQUIRE(parsed.Get<Tag>()->GetAt<Many>(1)->IsExact<Real>());
         REQUIRE(parsed.Get<Tag>()->GetAt<Many>(2)->IsExact<bool>());
         REQUIRE(*parsed.Get<Tag>()->GetAt<Many>(0) == Text {"test"});
         REQUIRE(*parsed.Get<Tag>()->GetAt<Many>(1) == 6_real);
         REQUIRE(*parsed.Get<Tag>()->GetAt<Many>(2) == true);
      }
   }

   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}
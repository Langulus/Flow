#include "Main.hpp"
#include <Flow/Temporal.hpp>
#include <catch2/catch.hpp>

SCENARIO("Temporal flow", "[temporal]") {
   ///                                                                        
   /// The following tests rely on this ontology sequence                     
   const auto HI = "? create Thing(User)"_code.Parse();
   const auto COMMA = "(number? >< number??) or (? Conjunct!4 ??)"_code.Parse();
   const auto DOT = "(number? + Fraction(number??)) or (? Conjunct!8 ??)"_code.Parse();
   const auto MY = "?.Thing(User).??"_code.Parse();
   const auto NAME = "Traits::Name"_code.Parse();
   const auto IS = "? = ??"_code.Parse();
   const auto APOSTROPHE_S = "(? = ??) or (?.Thing(Session or User)) or (?.??)"_code.Parse();
   const auto MAKE = "? create ??"_code.Parse();
   const auto AA = "single"_code.Parse();
   const auto GAME = "Thing(Universe, Window, Temporal)"_code.Parse();
   const auto CALLED = "? create Name(a::text??)"_code.Parse();

   GIVEN("An empty temporal flow") {
      Temporal flow {new Thing {}};

      WHEN("Code is pushed to the flow") {
         flow.Push(HI);
         flow.Push(COMMA);
         flow.Push(" "_text);
         flow.Push(MY);
         flow.Push(" "_text);
         flow.Push(NAME);
         flow.Push(" "_text);
         flow.Push(IS);
         flow.Push(" Dimo"_text);
         flow.Push(DOT);
         flow.Push("let"_text);
         flow.Push(APOSTROPHE_S);
         flow.Push(" "_text);
         flow.Push(MAKE);
         flow.Push(" "_text);
         flow.Push(AA);
         flow.Push(" "_text);
         flow.Push(GAME);
         flow.Push(" "_text);
         flow.Push(CALLED);
         flow.Push(" t"_text);
         flow.Push("e"_text);
         flow.Push("st"_text);
         flow.Push(" game"_text);
         flow.Push(GAME);

         THEN("The generated temporal flow should be correct") {

         }
      }
   }
}

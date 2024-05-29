///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Flow/Temporal.hpp>
#include "Common.hpp"


SCENARIO("Temporal flow", "[temporal]") {
   static Allocator::State memoryState;

   ///                                                                        
   /// The following tests rely on this ontology sequence                     
   const auto HI = "? create Thing(User)"_code.Parse();
   Logger::Info("HI -> ", HI);

   const auto COMMA = "(number? >< number??) or (? Conjunct!4 ??)"_code.Parse();
   Logger::Info("COMMA -> ", COMMA);

   const auto DOT = "? Conjunct!8 ??"_code.Parse();
   Logger::Info("DOT -> ", DOT);

   const auto MY = "?.Thing(User).??"_code.Parse();
   Logger::Info("MY -> ", MY);

   const auto NAME = "Name"_code.Parse();
   Logger::Info("NAME -> ", NAME);

   const auto IS = "? = ??"_code.Parse();
   Logger::Info("IS -> ", IS);

   const auto APOSTROPHE_S = "(? = ??) or (?.Thing(Session or User)) or (?.??)"_code.Parse();
   Logger::Info("APOSTROPHE_S -> ", APOSTROPHE_S);

   const auto MAKE = "? create ??"_code.Parse();
   Logger::Info("MAKE -> ", MAKE);

   const auto AA = "single"_code.Parse();
   Logger::Info("AA -> ", AA);

   const auto GAME = "Thing(Universe, Window, Temporal)"_code.Parse();
   Logger::Info("GAME -> ", GAME);

   const auto CALLED = "? create Name(a::text??)"_code.Parse();
   Logger::Info("CALLED -> ", CALLED);


   GIVEN("An empty temporal flow") {
      Temporal flow;// {new Thing {}};

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
      }
   }

   REQUIRE(memoryState.Assert());
}

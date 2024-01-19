///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Flow/Verbs/Interpret.hpp>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)

int main(int argc, char* argv[]) {
   static_assert(CT::Convertible<Thing, Text>,
      "Thing must be convertible to Text");
   static_assert(CT::Convertible<Verb, Code>,
      "Verb must be convertible to Code");

   static_assert(CT::Stringifiable<Thing>,
      "Thing must be convertible to Text");
   static_assert(CT::Stringifiable<Verb>,
      "Verb must be convertible to Text");

   Catch::Session session;
   return session.run(argc, argv);
}

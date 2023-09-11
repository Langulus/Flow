///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <catch2/catch.hpp>

SCENARIO("Test resolvables", "[resolvable]") {
	GIVEN("A resolvable instance") {
		Thing2 resolvable;
		auto concreteptr = static_cast<void*>(&resolvable);
		auto abstractptr = static_cast<void*>(static_cast<Resolvable*>(&resolvable));

		WHEN("Wrapped inside a block as Resolvable pointer") {
			Any pack = static_cast<Resolvable*>(&resolvable);

			THEN("When resolving the block, the correct data offset should be applied") {
				REQUIRE(concreteptr != abstractptr);
				REQUIRE(pack.IsSparse());
				REQUIRE(pack.IsExact<Resolvable*>());
				REQUIRE(pack.GetResolved().IsExact<Thing2>());
				REQUIRE(pack.GetResolved().Get<Thing2>().mMember == 777);
				REQUIRE(pack.GetResolved().Get<Thing>().mMember == 666);
				REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing>());
				REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing2>());
				REQUIRE(pack.GetResolved().Get<Thing2*>() == &resolvable);
			}
		}

		WHEN("Wrapped inside a block as an intermediate abstract type") {
			Any pack = static_cast<Thing*>(&resolvable);

			THEN("When resolving the block, the correct data offset should be applied") {
				REQUIRE(concreteptr != abstractptr);
				REQUIRE(pack.IsSparse());
				REQUIRE(pack.IsExact<Thing*>());
				REQUIRE(pack.GetResolved().IsExact<Thing2>());
				REQUIRE(pack.GetResolved().Get<Thing2>().mMember == 777);
				REQUIRE(pack.GetResolved().As<Thing>().mMember == 666);
				REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing>());
				REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing2>());
				REQUIRE(pack.GetResolved().Get<Thing2*>() == &resolvable);
			}
		}
	}
}

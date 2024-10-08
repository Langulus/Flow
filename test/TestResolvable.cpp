///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"


SCENARIO("Test resolvables", "[resolvable]") {
	GIVEN("A resolvable instance") {
		Thing2 resolvable;
		auto concreteptr = static_cast<void*>(&resolvable);
		auto abstractptr = static_cast<void*>(static_cast<Resolvable*>(&resolvable));

		WHEN("Wrapped inside a block as Resolvable pointer") {
         Many pack = static_cast<Resolvable*>(&resolvable);

			REQUIRE(concreteptr == abstractptr);
			REQUIRE(pack.IsSparse());
			REQUIRE(pack.IsExact<Resolvable*>());
			REQUIRE(pack.GetResolved().IsExact<Many, Thing2>());
			REQUIRE(pack.GetResolved().Get<Thing2>().mMember == 777);
			REQUIRE(pack.GetResolved().Get<Thing>().mMember == 666);
			REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing>());
			REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing2>());
			REQUIRE(pack.GetResolved().Get<Thing2*>() == &resolvable);
		}

		WHEN("Wrapped inside a block as an intermediate abstract type") {
         Many pack = static_cast<Thing*>(&resolvable);

			REQUIRE(concreteptr == abstractptr);
			REQUIRE(pack.IsSparse());
			REQUIRE(pack.IsExact<Thing*>());
			REQUIRE(pack.GetResolved().IsExact<Many, Thing2>());
			REQUIRE(pack.GetResolved().Get<Thing2>().mMember == 777);
			REQUIRE(pack.GetResolved().As<Thing>().mMember == 666);
			REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing>());
			REQUIRE(pack.GetResolved().As<Resolvable>().CastsTo<Thing2>());
			REQUIRE(pack.GetResolved().Get<Thing2*>() == &resolvable);
		}
	}
}

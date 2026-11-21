///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"


SCENARIO("Test resolvables") {
   static MemoryState memoryState;

	GIVEN("A resolvable instance") {//TODO test with managed data
		Thing2 resolvable;
		auto concreteptr = static_cast<void*>(&resolvable);
		auto abstractptr = static_cast<void*>(static_cast<Downcastable*>(&resolvable));
		REQUIRE(concreteptr != abstractptr);

		WHEN("Wrapped inside a block as Resolvable pointer") {
         Many pack = static_cast<Downcastable*>(&resolvable);

			REQUIRE(pack.IsSparse());
			REQUIRE(pack.IsExact<Downcastable*>());
			REQUIRE(pack.GetResolved().type.IsExact(MetaDataOf<Thing2>()));
			REQUIRE(pack.GetResolved().As<Thing2>() == &resolvable);
			REQUIRE(pack.GetResolved().As<Thing>() == &resolvable);
			REQUIRE(pack.GetResolved().As<Thing2>()->mMember == 777);
			REQUIRE(pack.GetResolved().As<Thing>()->mMember == 666);
			REQUIRE(pack.GetResolved().As<Downcastable>()->CastsTo(MetaDataOf<Thing>()));
			REQUIRE(pack.GetResolved().As<Downcastable>()->CastsTo(MetaDataOf<Thing2>()));
			//REQUIRE(pack.GetResolved().GetMember()->CastsTo(MetaDataOf<Thing2>()));//TODO
		}

		WHEN("Wrapped inside a block as an intermediate abstract type") {
         Many pack = static_cast<Thing*>(&resolvable);

			REQUIRE(pack.IsSparse());
			REQUIRE(pack.IsExact<Thing*>());
			REQUIRE(pack.GetResolved().type.IsExact(MetaDataOf<Thing2>()));
			REQUIRE(pack.GetResolved().As<Thing2>() == &resolvable);
			REQUIRE(pack.GetResolved().As<Thing>() == &resolvable);
			REQUIRE(pack.GetResolved().As<Thing2>()->mMember == 777);
			REQUIRE(pack.GetResolved().As<Thing>()->mMember == 666);
			REQUIRE(pack.GetResolved().As<Downcastable>()->CastsTo(MetaDataOf<Thing>()));
			REQUIRE(pack.GetResolved().As<Downcastable>()->CastsTo(MetaDataOf<Thing2>()));
		}
	}
	
   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}

///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Main.hpp"
#include <Flow/Verbs/Create.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include <catch2/catch.hpp>

SCENARIO("Test factories", "[factory]") {
	Producer producer;

	GIVEN("A factory with default usage") {
		TFactory<Producible> factory {&producer};

		WHEN("Default-constructed") {
			THEN("Invariant state should be ensured") {
				REQUIRE(factory.IsUnique == false);
				REQUIRE(factory.mFactoryOwner == &producer);
				REQUIRE(factory.mReusable == nullptr);
				REQUIRE(not factory.mHashmap);
				REQUIRE(not factory.mData);
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactory<Producible>::Element>());
			}
		}

		WHEN("Two default elements produced") {
			Verbs::Create creator {Construct::From<Producible>()};

			factory.Create(creator);
			auto out1 = creator.GetOutput();
			REQUIRE(creator.IsDone());

			creator.Undo();
			factory.Create(creator);
			REQUIRE(creator.IsDone());
			auto out2 = creator.GetOutput();

			const Neat normalized {};
			const auto hash = normalized.GetHash();

			THEN("Requirements should be met") {
				REQUIRE(out1.GetCount() == 1);
				REQUIRE(out1.IsExact<Producible*>());
				REQUIRE(out1.IsSparse());
				REQUIRE(out2.GetCount() == 1);
				REQUIRE(out2.IsExact<Producible*>());
				REQUIRE(out2.IsSparse());

				REQUIRE(factory.mReusable == nullptr);
				REQUIRE(factory.mHashmap.GetCount() == 1);
				REQUIRE(factory.mData.GetCount() == 2);
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactory<Producible>::Element>());
				REQUIRE(factory.mData[0].mData == Producible {&producer});
				REQUIRE(factory.mData[0].mData.GetNeat() == Neat {});
				REQUIRE(factory.mData[0].mData.GetHash() == hash);
				REQUIRE(factory.mData[0].mData.GetNeat() == normalized);
				REQUIRE(factory.mData[1].mData == Producible {&producer});
				REQUIRE(factory.mData[1].mData.GetNeat() == Neat {});
				REQUIRE(factory.mData[1].mData.GetHash() == hash);
				REQUIRE(factory.mData[1].mData.GetNeat() == normalized);
				REQUIRE(factory.mHashmap[hash].GetCount() == 2);
				REQUIRE(factory.mHashmap[hash][0] == &factory.mData[0]);
				REQUIRE(factory.mHashmap[hash][1] == &factory.mData[1]);
			}
		}
	}

	GIVEN("A factory with unique usage") {
		TFactoryUnique<Producible> factory {&producer};

		WHEN("Default-constructed") {
			THEN("Invariant state should be ensured") {
				REQUIRE(factory.IsUnique == true);
				REQUIRE(factory.mFactoryOwner == &producer);
				REQUIRE(factory.mReusable == nullptr);
				REQUIRE(not factory.mHashmap);
				REQUIRE(not factory.mData);
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactoryUnique<Producible>::Element>());
			}
		}

		WHEN("Two default elements produced") {
			Verbs::Create creator {Construct::From<Producible>()};

			factory.Create(creator);
			auto out1 = creator.GetOutput();
			creator.Undo();
			factory.Create(creator);
			auto out2 = creator.GetOutput();

			const Neat normalized {};
			const auto hash = normalized.GetHash();

			THEN("Requirements should be met") {
				REQUIRE(creator.IsDone());
				REQUIRE(out1.GetCount() == 1);
				REQUIRE(out1.IsExact<Producible*>());
				REQUIRE(out1.IsSparse());
				REQUIRE(out2.GetCount() == 1);
				REQUIRE(out2.IsExact<Producible*>());
				REQUIRE(out2.IsSparse());

				REQUIRE(factory.mReusable == nullptr);
				REQUIRE(factory.mHashmap.GetCount() == 1);
				REQUIRE(factory.mData.GetCount() == 1);
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactoryUnique<Producible>::Element>());
				REQUIRE(factory.mData[0].mData == Producible {&producer});
				REQUIRE(factory.mData[0].mData.GetNeat() == Neat {});
				REQUIRE(factory.mData[0].mData.GetHash() == hash);
				REQUIRE(factory.mData[0].mData.GetNeat() == normalized);
				REQUIRE(factory.mHashmap[hash].GetCount() == 1);
				REQUIRE(factory.mHashmap[hash][0] == &factory.mData[0]);
			}
		}
	}
}

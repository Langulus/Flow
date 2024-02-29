///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Flow/Verbs/Create.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include <Flow/Factory.hpp>
#include "Common.hpp"


SCENARIO("Test factories", "[factory]") {
   static Allocator::State memoryState;

	Producer producer;

	GIVEN("A factory with default usage") {
		TFactory<Producible> factory {&producer};

		WHEN("Default-constructed") {
			REQUIRE(factory.IsUnique == false);
			REQUIRE(factory.mFactoryOwner == &producer);
			REQUIRE(factory.mReusable == nullptr);
			REQUIRE(not factory.mHashmap);
			REQUIRE(factory.IsEmpty());
			REQUIRE(factory.GetType() == MetaOf<Producible>());
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

			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
			REQUIRE(out1.As<Producible*>()->GetReferences() == 1);
			REQUIRE(out1.IsSparse());
			REQUIRE(out2.GetCount() == 1);
			REQUIRE(out2.IsExact<Producible*>());
         REQUIRE(out2.As<Producible*>()->GetReferences() == 1);
         REQUIRE(out2.IsSparse());

         REQUIRE(factory.mReusable == factory.mFrames[0].GetRaw() + 2);
         REQUIRE(factory.mHashmap.GetCount() == 1);
			REQUIRE(factory.GetCount() == 2);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData == Producible {&producer});
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetNeat() == Neat {});
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetNeat() == normalized);
			REQUIRE(factory.mFrames[0].GetRaw()[1].mData == Producible {&producer});
			REQUIRE(factory.mFrames[0].GetRaw()[1].mData.GetNeat() == Neat {});
			REQUIRE(factory.mFrames[0].GetRaw()[1].mData.GetHash() == hash);
			REQUIRE(factory.mFrames[0].GetRaw()[1].mData.GetNeat() == normalized);
			REQUIRE(factory.mHashmap[hash].GetCount() == 2);
			REQUIRE(factory.mHashmap[hash][0] == &factory.mFrames[0].GetRaw()[0]);
			REQUIRE(factory.mHashmap[hash][1] == &factory.mFrames[0].GetRaw()[1]);
		}
	}

	GIVEN("A factory with unique usage") {
		TFactoryUnique<Producible> factory {&producer};

		WHEN("Default-constructed") {
			REQUIRE(factory.IsUnique == true);
			REQUIRE(factory.mFactoryOwner == &producer);
			REQUIRE(factory.mReusable == nullptr);
			REQUIRE(not factory.mHashmap);
         REQUIRE(factory.IsEmpty());
         REQUIRE(factory.GetType() == MetaOf<Producible>());
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

			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
			REQUIRE(out1.IsSparse());
			REQUIRE(out2.GetCount() == 1);
			REQUIRE(out2.IsExact<Producible*>());
			REQUIRE(out2.IsSparse());

         REQUIRE(factory.mReusable == factory.mFrames[0].GetRaw() + 1);
         REQUIRE(factory.mHashmap.GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData == Producible {&producer});
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetNeat() == Neat {});
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.mFrames[0].GetRaw()[0].mData.GetNeat() == normalized);
			REQUIRE(factory.mHashmap[hash].GetCount() == 1);
			REQUIRE(factory.mHashmap[hash][0] == &factory.mFrames[0].GetRaw()[0]);
		}
	}

   REQUIRE(memoryState.Assert());
}

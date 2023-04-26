#include "Main.hpp"
#include <Flow/Verbs/Create.hpp>
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
				REQUIRE(factory.mHashmap.IsEmpty());
				REQUIRE(factory.mData.IsEmpty());
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactory<Producible>::Element>());
			}
		}

		WHEN("Element produced") {
			Verbs::Create creator {Construct::From<Producible>()};
			factory.Create(creator);
			auto& output = creator.GetOutput();
			const auto normalized = Normalized {{}};
			const auto hash = normalized.GetHash();

			THEN("Requirements should be met") {
				REQUIRE(creator.IsDone());
				REQUIRE(output.GetCount() == 1);
				REQUIRE(output.IsExact<Producible*>());
				REQUIRE(output.IsSparse());

				REQUIRE(factory.mReusable == nullptr);
				REQUIRE(factory.mHashmap.GetCount() == 1);
				REQUIRE(factory.mData.GetCount() == 1);
				REQUIRE(factory.mData.GetType() == MetaOf<typename TFactory<Producible>::Element>());
				REQUIRE(factory.mData[0].mData == Producible {&producer});
				REQUIRE(factory.mData[0].mData.GetDescriptor() == Normalized {{}});
				REQUIRE(factory.mData[0].mData.GetHash() == hash);
				REQUIRE(factory.mData[0].mData.GetDescriptor() == normalized);
				REQUIRE(factory.mHashmap[hash].GetCount() == 1);
				REQUIRE(factory.mHashmap[hash][0] == &factory.mData[0]);
			}
		}
	}
}

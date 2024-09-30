///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Common.hpp"
#include <Flow/Verbs/Create.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include <Flow/Factory.hpp>


SCENARIO("Test factories", "[factory]") {
   static Allocator::State memoryState;

	Producer producer;

	GIVEN("A factory with default usage") {
      TFactory<Producible> factory;

		REQUIRE(factory.IsUnique == false);
		REQUIRE(factory.GetReusable() == nullptr);
		REQUIRE(not factory.GetHashmap());
		REQUIRE(factory.IsEmpty());
		REQUIRE(factory.GetType() == MetaOf<Producible>());

		WHEN("Two default elements produced") {
         const auto descriptor = Construct::From<Producible>();
         Verbs::Create creator {descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};
         const Many normalized {};
         const auto hash = normalized.GetHash();

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
			REQUIRE(creator.IsDone());
         REQUIRE(out1.GetCount() == 1);
         REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->Reference(0) == 3);
         REQUIRE(out1.IsSparse());

			creator.Undo();

			factory.Create(&producer, creator);
			REQUIRE(creator.IsDone());
			auto out2 = creator.GetOutput();
			REQUIRE(out2.GetCount() == 1);
			REQUIRE(out2.IsExact<Producible*>());
         REQUIRE(out2.As<Producible*>()->Reference(0) == 3);
         REQUIRE(out2.IsSparse());

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 2);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 2);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == normalized);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == normalized);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 2);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);
			REQUIRE(factory.GetHashmap()[hash][1] == &factory.GetFrames()[0].GetRaw()[1]);

         prototype.Reference(-1);
		}
	}

	GIVEN("A factory with unique usage") {
      TFactoryUnique<Producible> factory;

		REQUIRE(factory.IsUnique == true);
		REQUIRE(factory.GetReusable() == nullptr);
		REQUIRE(not factory.GetHashmap());
      REQUIRE(factory.IsEmpty());
      REQUIRE(factory.GetType() == MetaOf<Producible>());

		WHEN("Two default elements produced") {
         const auto descriptor = Construct::From<Producible>();
			Verbs::Create creator {descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();

			const Many normalized {};
			const auto hash = normalized.GetHash();

			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->Reference(0) == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == normalized);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         prototype.Reference(-1);
		}
      
		WHEN("Two elements produced via descriptors") {
         TMany<Producer> context;
         context.New(1);

         const auto descriptor = Construct::From<Producible>(
            Traits::Parent(&context[0]),
            "test"_text
         );
			Verbs::Create creator {&descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
         REQUIRE(out1.As<Producible*>()->Reference(0) == 3);

			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();
         REQUIRE(out2.As<Producible*>()->Reference(0) == 4);

         // Parent traits shouldn't participate in hashing              
			const auto hash = descriptor.GetDescriptor().GetHash();

			REQUIRE(context.GetUses() == 2);
			REQUIRE(context[0].Reference(0) == 2);
			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->Reference(0) == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
         // Parent traits shouldn't participate in comparison           
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         prototype.Reference(-1);
		}
	}

   REQUIRE(memoryState.Assert());
}

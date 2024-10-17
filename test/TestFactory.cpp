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


SCENARIO("Test factories on the stack", "[factory]") {
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
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);
         REQUIRE(out1.IsSparse());

			creator.Undo();

			factory.Create(&producer, creator);
			REQUIRE(creator.IsDone());
			auto out2 = creator.GetOutput();
			REQUIRE(out2.GetCount() == 1);
			REQUIRE(out2.IsExact<Producible*>());
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(out2.As<Producible*>()->GetReferences() == 3);
         REQUIRE(out2.IsSparse());

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 2);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 2);
			REQUIRE(factory.GetType() == MetaOf<Producible>());

			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == normalized);
         // Kept once in TFactory, once in out1                         
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetReferences() == 2);

			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == normalized);
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetReferences() == 3);

			REQUIRE(factory.GetHashmap()[hash].GetCount() == 2);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);
			REQUIRE(factory.GetHashmap()[hash][1] == &factory.GetFrames()[0].GetRaw()[1]);

         const_cast<Producible&>(prototype).Reference(-1);
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
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
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

         const_cast<Producible&>(prototype).Reference(-1);
		}
      
		WHEN("Two elements produced via descriptors, with parent on the heap") {
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
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);

			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();
         REQUIRE(out2.As<Producible*>()->GetReferences() == 4);

         //TODO Missing stuff shouldn't participate in hashing
         const auto hash = descriptor->GetHash();

			REQUIRE(context.GetUses() == 2);
			REQUIRE(context[0].GetReferences() == 2);
			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
         //TODO Missing stuff shouldn't participate in comparison
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         const_cast<Producible&>(prototype).Reference(-1);
		}

		WHEN("Two elements produced via descriptors, with parent on the stack") {
         const auto descriptor = Construct::From<Producible>(
            Traits::Parent(&producer),
            "test"_text
         );
			Verbs::Create creator {&descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);

			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();
         REQUIRE(out2.As<Producible*>()->GetReferences() == 4);

         //TODO Missing stuff shouldn't participate in hashing
         const auto hash = descriptor->GetHash();

			REQUIRE(producer.GetReferences() == 1);
			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
         //TODO Missing stuff shouldn't participate in comparison
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         const_cast<Producible&>(prototype).Reference(-1);
		}
	}

   REQUIRE(memoryState.Assert());
}

SCENARIO("Test factories on the heap", "[factory]") {
   static Allocator::State memoryState;

	TMany<Producer> wrappedProducer;
   wrappedProducer.New();
   auto& producer = wrappedProducer[0];

	GIVEN("A factory with default usage") {
      auto& factory = producer.factory1;

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
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);
         REQUIRE(out1.IsSparse());

			creator.Undo();

			factory.Create(&producer, creator);
			REQUIRE(creator.IsDone());
			auto out2 = creator.GetOutput();
			REQUIRE(out2.GetCount() == 1);
			REQUIRE(out2.IsExact<Producible*>());
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(out2.As<Producible*>()->GetReferences() == 3);
         REQUIRE(out2.IsSparse());

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 2);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 2);
			REQUIRE(factory.GetType() == MetaOf<Producible>());

			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == normalized);
         // Kept once in TFactory, once in out1                         
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetReferences() == 2);

			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData == prototype);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetHash() == hash);
			REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetDescriptor() == normalized);
         // Kept once in TFactory, once in out1, once in creator.mOutput
         REQUIRE(factory.GetFrames()[0].GetRaw()[1].mData.GetReferences() == 3);

			REQUIRE(factory.GetHashmap()[hash].GetCount() == 2);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);
			REQUIRE(factory.GetHashmap()[hash][1] == &factory.GetFrames()[0].GetRaw()[1]);

         const_cast<Producible&>(prototype).Reference(-1);
		}

      factory.Teardown();
	}

	GIVEN("A factory with unique usage") {
      auto& factory = producer.factory2;

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
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
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

         const_cast<Producible&>(prototype).Reference(-1);
		}
      
		WHEN("Two elements produced via descriptors, with parent on the heap (inducing a circular dependency)") {
         const auto descriptor = Construct::From<Producible>(
            Traits::Parent(&producer),
            "test"_text
         );
			Verbs::Create creator {&descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);

			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();
         REQUIRE(out2.As<Producible*>()->GetReferences() == 4);

         //TODO Missing stuff shouldn't participate in hashing
			const auto hash = descriptor->GetHash();

			REQUIRE(producer.GetReferences() == 4);
			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
         //TODO Missing stuff shouldn't participate in comparison
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         const_cast<Producible&>(prototype).Reference(-1);
		}

      WHEN("Two elements produced via descriptors, with parent on the stack") {
         Producer context;
         const auto descriptor = Construct::From<Producible>(
            Traits::Parent(&context),
            "test"_text
         );
			Verbs::Create creator {&descriptor};
         const Producible prototype {&producer, descriptor.GetDescriptor()};

			factory.Create(&producer, creator);
			auto out1 = creator.GetOutput();
         REQUIRE(out1.As<Producible*>()->GetReferences() == 3);

			creator.Undo();
			factory.Create(&producer, creator);
			auto out2 = creator.GetOutput();
         REQUIRE(out2.As<Producible*>()->GetReferences() == 4);

         //TODO Missing stuff shouldn't participate in hashing
         const auto hash = descriptor->GetHash();

			REQUIRE(context.GetReferences() == 1);
			REQUIRE(creator.IsDone());
			REQUIRE(out1.GetCount() == 1);
			REQUIRE(out1.IsExact<Producible*>());
         REQUIRE(out1.As<Producible*>()->GetReferences() == 4);
         REQUIRE(out1.IsSparse());
         REQUIRE(out1 == out2);

         REQUIRE(factory.GetReusable() == factory.GetFrames()[0].GetRaw() + 1);
         REQUIRE(factory.GetHashmap().GetCount() == 1);
			REQUIRE(factory.GetCount() == 1);
			REQUIRE(factory.GetType() == MetaOf<Producible>());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData == prototype);
         //TODO Missing stuff shouldn't participate in comparison
         REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetDescriptor() == descriptor.GetDescriptor());
			REQUIRE(factory.GetFrames()[0].GetRaw()[0].mData.GetHash() == hash);
			REQUIRE(factory.GetHashmap()[hash].GetCount() == 1);
			REQUIRE(factory.GetHashmap()[hash][0] == &factory.GetFrames()[0].GetRaw()[0]);

         const_cast<Producible&>(prototype).Reference(-1);
		}

      factory.Teardown();
	}

   wrappedProducer.Reset();

   REQUIRE(memoryState.Assert());
}

SCENARIO("Nested factories and circular referencing", "[factory]") {
   static Allocator::State memoryState;

   TMany<DeepProducer> wrappedProducer;
   wrappedProducer.New();
   auto& deepProducer = wrappedProducer[0];

   GIVEN("A factory with unique usage") {
      Verbs::Create creator1 {
         Construct::From<ShallowProducer>(
            Traits::Parent(&deepProducer),
            "test"_text
         )
      };

      deepProducer.factory.Create(&deepProducer, creator1);
      REQUIRE(creator1.IsDone());
      auto& shallowProducer = creator1.GetOutput().As<ShallowProducer>();

      Verbs::Create creator2 {
         Construct::From<TheProducible>(
            Traits::Parent(&shallowProducer),
            "test"_text
         )
      };

      shallowProducer.factory.Create(&shallowProducer, creator2);
      REQUIRE(creator2.IsDone());
   }

   wrappedProducer.Reset();

   REQUIRE(memoryState.Assert());
}

///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Anyness/Serial.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include "Common.hpp"

constexpr Count SerialBlock = sizeof(Count) * 2 + sizeof(DataState);


SCENARIO("Serialization", "[serialization]") {
   static Allocator::State memoryState;

	GIVEN("An empty Many instance") {
      Many pack;

		WHEN("Pack is serialized as binary") {
         const auto serialized = Verbs::Interpret::To<Bytes>(pack);

			REQUIRE(serialized.Is<Byte>());
			REQUIRE(serialized.GetCount() == SerialBlock);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
			   REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("An Many instance containing other Many instances") {
      Many pack;
		pack << Many {1} << Many {2} << Many {3};

		WHEN("Pack is serialized as binary") {
         const auto serialized = Verbs::Interpret::To<Bytes>(pack);
         const auto intToken = MetaDataOf<int>()->mToken.size();
         const auto anyToken = MetaDataOf<Many>()->mToken.size();
         const auto requiredSize = 
              SerialBlock * 4             // 4 blocks
            + sizeof(int) * 3             // + three ints
            + intToken * 3 + anyToken;    // + three 'int' tokens and one 'Any' token
         REQUIRE(serialized.GetCount() == requiredSize);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("A TMany instance containing other Many instances") {
		TMany<Many> pack;
		pack << Many {1} << Many {2} << Many {3};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

   const Text texts[3] = {
      "hello"_text,
      "i love you"_text,
      "won't you tell me your name"_text
   };
   const auto txtToken = MetaDataOf<Text>()->mToken.size();
   const auto traitToken = MetaDataOf<Traits::Name>()->mToken.size();

	GIVEN("An Any instance containing Text") {
      Many pack;
		pack << texts[0] << texts[1] << texts[2];

		WHEN("Pack is serialized as binary") {
			const auto serialized = Verbs::Interpret::To<Bytes>(pack);
         const auto requiredSize =
            SerialBlock                   // 1 block
            + txtToken
            + texts[0].GetCount() + sizeof(Count)
            + texts[1].GetCount() + sizeof(Count)
            + texts[2].GetCount() + sizeof(Count);
         REQUIRE(serialized.GetCount() == requiredSize);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("An Any instance containing Trait") {
      Many pack;
		pack  << Traits::Name(texts[0])
		      << Traits::Name(texts[1])
		      << Traits::Name(texts[2]);

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);
         const auto requiredSize =
            SerialBlock * 4               // 4 blocks
            + traitToken + txtToken*3
            + texts[0].GetCount() + sizeof(Count)
            + texts[1].GetCount() + sizeof(Count)
            + texts[2].GetCount() + sizeof(Count);
         REQUIRE(serialized.GetCount() == requiredSize);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("An Any instance containing a verb") {
      Many pack = Verbs::Do(10).SetSource(5);

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("An Any instance containing various kinds of numbers") {
      Many pack {10, 5, 20.0f, 40.0};

		WHEN("Pack is serialized as binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

	GIVEN("A complex pack with various kinds of data") {
      Many pack {
			"some text"_text,
			10, 5, 20.0f, 40.0,
			Verbs::Do(10).SetSource(5),
			Verbs::Do(10).SetSource("some other text"_text),
			Verbs::Do("even more text"_text).SetSource(Verbs::Do(10).SetSource(5))
		};

		WHEN("Serialize and then deserialize container in binary") {
			auto serialized = Verbs::Interpret::To<Bytes>(pack);

			#if LANGULUS_FEATURE(MANAGED_REFLECTION)
				auto deserialized = Verbs::Interpret::To<Many>(serialized);
				REQUIRE(deserialized == pack);
			#endif
		}
	}

   REQUIRE(memoryState.Assert());
}
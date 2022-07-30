#include "Main.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	// For reflection of all verbs													
	(void)MetaVerb::Of<Verbs::Add>();
	(void)MetaVerb::Of<Verbs::Associate>();
	(void)MetaVerb::Of<Verbs::Catenate>();
	(void)MetaVerb::Of<Verbs::Conjunct>();
	(void)MetaVerb::Of<Verbs::Create>();
	(void)MetaVerb::Of<Verbs::Do>();
	(void)MetaVerb::Of<Verbs::Exponent>();
	(void)MetaVerb::Of<Verbs::Interpret>();
	(void)MetaVerb::Of<Verbs::Multiply>();
	(void)MetaVerb::Of<Verbs::Select>();

	Catch::Session session;
	return session.run(argc, argv);
}

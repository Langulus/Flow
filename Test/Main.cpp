#include "Main.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	MetaVerb::Of<Verbs::Associate>();

	Catch::Session session;
	return session.run(argc, argv);
}

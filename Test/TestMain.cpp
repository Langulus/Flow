#include "TestMain.hpp"

PC_MONOPOLIZE_MEMORY(0)

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

int main(int argc, char* argv[]) {
	pcInitGASM();

	Catch::Session session;
	return session.run(argc, argv);
}

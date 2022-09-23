#pragma once
#include <LangulusFlow.hpp>

using namespace Langulus;
using namespace Langulus::Flow;

#define CATCH_CONFIG_ENABLE_BENCHMARKING

/// A mockup of Langulus::Entity, for testing purposes								
struct Entity {
	LANGULUS(PRODUCER) Entity;
};

/// A mockup of a universe component, for testing purposes							
struct Universe {
	LANGULUS(PRODUCER) Entity;
};

/// A mockup of a window component, for testing purposes								
struct Window {
	LANGULUS(PRODUCER) Entity;
};

/// A mockup of a user component, for testing purposes								
struct User {
	LANGULUS(PRODUCER) Entity;
};

/// A mockup of a fraction																		
struct Fraction {

};


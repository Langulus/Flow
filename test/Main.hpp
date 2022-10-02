#pragma once
#include <LangulusFlow.hpp>

using namespace Langulus;
using namespace Langulus::Flow;

#define CATCH_CONFIG_ENABLE_BENCHMARKING

/// A mockup of Langulus::Entity, for testing purposes								
struct Entity : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS(PRODUCER) Entity;
	LANGULUS_BASES(Resolvable);
	Entity() : Resolvable(MetaData::Of<Entity>()) {}
};

/// A mockup of a universe component, for testing purposes							
struct Universe : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS(PRODUCER) Entity;
	LANGULUS_BASES(Resolvable);
	Universe() : Resolvable(MetaData::Of<Universe>()) {}
};

/// A mockup of a window component, for testing purposes								
struct Window : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS(PRODUCER) Entity;
	LANGULUS_BASES(Resolvable);
	Window() : Resolvable(MetaData::Of<Window>()) {}
};

/// A mockup of a user component, for testing purposes								
struct User : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS(PRODUCER) Entity;
	LANGULUS_BASES(Resolvable);
	User() : Resolvable(MetaData::Of<User>()) {}
};

/// A mockup of a session component, for testing purposes							
struct Session : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS(PRODUCER) Entity;
	LANGULUS_BASES(Resolvable);
	Session() : Resolvable(MetaData::Of<Session>()) {}
};

/// A mockup of a fraction																		
struct Fraction : public Resolvable {
	LANGULUS(ABSTRACT) false;
	LANGULUS(UNINSERTABLE) false;
	LANGULUS_BASES(Resolvable);
	Fraction() : Resolvable(MetaData::Of<Fraction>()) {}
};


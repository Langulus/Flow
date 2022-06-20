#pragma once
#include <PCFW.Memory.hpp>

#ifdef PC_EXPORT_FLOW
	#define LANGULUS_API_FOR_FLOW() LANGULUS_EXPORT()
#else
	#define LANGULUS_API_FOR_FLOW() LANGULUS_IMPORT()
#endif

namespace PCFW::Flow
{
	using namespace ::PCFW::Memory;
}
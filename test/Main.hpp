#pragma once
#include <iostream>
#include <Langulus.Flow.hpp>
using namespace Langulus;
using namespace Langulus::Flow;

#define CATCH_CONFIG_ENABLE_BENCHMARKING

namespace std
{
   inline ostream& operator << (ostream& os, const ::Langulus::Exception& value) {
      const ::Langulus::Anyness::Text stringified {value};
      os << Token {stringified};
      return os;
   }
}
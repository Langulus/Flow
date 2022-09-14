#pragma once
#include "Common.hpp"

namespace Langulus::Flow
{

	///																								
	/// A scope is simply an Any container (binary compatible, too)				
	/// It has some additional functions for executing flows							
	///																								
	struct Scope : public Any {
		using Any::Any;
		using Any::operator ==;

		Scope Clone() const;

		bool IsExecutable() const noexcept;
		bool IsExecutableDeep() const noexcept;

		bool Execute(Any&) const;
		bool Execute(Any&, Any& output) const;
		bool Execute(Any&, Any& output, bool& skipVerbs) const;
		bool ExecuteAND(Any&, Any& output, bool& skipVerbs) const;
		bool ExecuteOR(Any&, Any& output, bool& skipVerbs) const;

		static bool ExecuteVerb(Any&, Verb&);
		static bool IntegrateVerb(Any&, Verb&);
	};

} // namespace Langulus::Flow

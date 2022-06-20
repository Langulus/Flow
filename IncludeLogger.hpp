#pragma once
#include "Serial.hpp"

namespace PCFW::Logger
{

	using Debug = Memory::Debug;
	using GASM = Flow::GASM;

	/// Invoke reflected converters to Debug for T, and log the result			
	///	@tparam T - the type of the item (deducible)									
	///	@param item - the item to log														
	///	@return the logger for chaining													
	template<NotLogSpecific T>
	LoggerSystem& LoggerSystem::operator << (const T& item) {
		return Write(::PCFW::Flow::pcSerialize<Debug>(item));
	}

} // namespace PCFW::Logger

#define pcLogSelf					(pcLog << *this << ": ")
#define pcLogSelfUnknown		(pcLogUnknown << pcLogSelf)
#define pcLogSelfInfo			(pcLogInfo << pcLogSelf)
#define pcLogSelfVerbose		(pcLogVerbose << pcLogSelf)
#define pcLogSelfWarning		(pcLogWarning << pcLogSelf)
#define pcLogSelfError			(pcLogError << pcLogSelf)
#define pcLogSelfMessage		(pcLogMessage << pcLogSelf)
#define pcLogSelfSpecial		(pcLogSpecial << pcLogSelf)
#define pcLogSelfFlow			(pcLogFlow << pcLogSelf)
#define pcLogSelfInput			(pcLogInput << pcLogSelf)
#define pcLogSelfTCP				(pcLogTCP << pcLogSelf)
#define pcLogSelfUDP				(pcLogUDP << pcLogSelf)
#define pcLogSelfOS				(pcLogOS << pcLogSelf)
#define pcLogSelfChoice			(pcLogChoice << pcLogSelf)


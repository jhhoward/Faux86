#pragma once

#include "Types.h"
#include "Config.h"

namespace Faux86
{
	class VM;

	enum DebugMemFlags
	{
		MemArea_BIOS = 1,
		MemArea_VGABIOS = 2,
		MemArea_BDA = 4,
		MemArea_InterruptTable = 8,
		MemArea_FunctionEntryPoint = 16,
	};

	class Debugger
	{
	public:
		Debugger(VM& inVM);

		bool isDebugging = false;

		bool shouldBreakOnExecute(uint32_t address);
		void onMemoryWrite(uint32_t address);
		void flagRegion(uint32_t addressStart, uint32_t range, uint32_t flagMask);

		void flagAddress(uint32_t address, uint32_t flagMask)
		{
			memFlags[address] |= flagMask;
		}

		void unflagAddress(uint32_t address, uint32_t flagMask)
		{
			memFlags[address] &= ~flagMask;
		}

		void addExecutionBreakpoint(uint32_t address);
		void addDataBreakpoint(uint32_t address);

		void onCall(uint32_t functionAddress, uint32_t returningAddress);
		void onReturn(uint32_t returningAddress);

		void logCallstack();

	private:

		VM& vm;

		uint32_t memFlags[DEFAULT_RAM_SIZE];

		static constexpr int MaxCallStackSize = 32;
		uint32_t callStack[MaxCallStackSize];
		uint32_t callStackSize = 0;

		static constexpr int MaxBreakpoints = 16;
		uint32_t dataBreakpoints[MaxBreakpoints];
		uint32_t numDataBreakpoints = 0;
		uint32_t executionBreakpoints[MaxBreakpoints];
		uint32_t numExecutionBreakpoints = 0;
	};
}

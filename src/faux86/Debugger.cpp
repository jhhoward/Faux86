#include "Debugger.h"
#include "MemUtils.h"
#include "VM.h"

using namespace Faux86;

Debugger::Debugger(VM& inVM) : vm(inVM)
{
	MemUtils::memset(memFlags, 0, DEFAULT_RAM_SIZE * sizeof(uint32_t));
}

bool Debugger::shouldBreakOnExecute(uint32_t address)
{
	for (uint32_t n = 0; n < numExecutionBreakpoints; n++)
	{
		if (executionBreakpoints[n] == address)
		{
			log(LogDebugger, "Hit breakpoint at address %x", address);
			logCallstack();

			return true;
		}
	}
	return false;
}

void Debugger::onMemoryWrite(uint32_t address)
{
	unflagAddress(address, MemArea_FunctionEntryPoint);

	for (uint32_t n = 0; n < numDataBreakpoints; n++)
	{
		if (dataBreakpoints[n] == address)
		{
			isDebugging = true;
			log(LogDebugger, "Hit data breakpoint at address %x : changed to %x", address, vm.memory.RAM[address]);
			logCallstack();
		}
	}
}

void Debugger::flagRegion(uint32_t addressStart, uint32_t range, uint32_t flagMask)
{
	for (uint32_t n = 0; n < range; n++)
	{
		memFlags[addressStart + n] |= flagMask;
	}
}

void Debugger::onCall(uint32_t functionAddress, uint32_t returningAddress)
{
	//log(LogDebugger, "Function call %x from %x", functionAddress, returningAddress);
	if (callStackSize == MaxCallStackSize)
	{
		for (uint32_t n = 0; n < MaxCallStackSize - 1; n++)
		{
			callStack[n] = callStack[n + 1];
		}
		callStack[MaxCallStackSize - 1] = returningAddress;
	}
	else
	{
		callStack[callStackSize++] = returningAddress;
	}
	flagAddress(functionAddress, MemArea_FunctionEntryPoint);
}

void Debugger::onReturn(uint32_t returningAddress)
{
	if (callStackSize > 0)
	{
		//log(LogDebugger, "Return from %x", returningAddress);

		for (uint32_t n = callStackSize; n > 0; n--)
		{
			if (callStack[n - 1] == returningAddress)
			{
				callStackSize = n - 1;
				return;
			}
		}

		log(LogDebugger, "Mismatching return address %x : %x", callStack[callStackSize - 1], returningAddress);
	}
}

void Debugger::logCallstack()
{
	log(LogDebugger, "Call stack:");
	log(LogDebugger, "> %x", (vm.cpu.segregs[regcs] << 4) + vm.cpu.ip);
	for (int n = callStackSize; n > 0; n--)
	{
		log(LogDebugger, "  %x", callStack[n - 1]);
	}
}

void Debugger::addExecutionBreakpoint(uint32_t address)
{
	if (numExecutionBreakpoints < MaxBreakpoints)
		executionBreakpoints[numExecutionBreakpoints++] = address;
}

void Debugger::addDataBreakpoint(uint32_t address)
{
	if (numDataBreakpoints < MaxBreakpoints)
		dataBreakpoints[numDataBreakpoints++] = address;
}

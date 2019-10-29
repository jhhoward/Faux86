/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif
#include "TaskManager.h"
#include "VM.h"

using namespace Faux86;

TaskManager::TaskManager(VM& inVM) : vm(inVM)
{
}

void TaskManager::tick()
{
	if (vm.config.singleThreaded) 
	{
		for (int n = 0; n < numTasks; n++)
		{
			uint64_t currentTime = vm.timing.getTicks();

			if (tasks[n].task && tasks[n].running && tasks[n].nextTickTime < currentTime)
			{
				int result = tasks[n].task->update();
				if (result < 0)
				{
					// TODO: error?
				}
				else
				{
					//tasks[n].nextTickTime = vm.timing.getTicks() + result * vm.timing.getHostFreq() / 1000;
					tasks[n].nextTickTime = currentTime + result * vm.timing.getHostFreq() / 1000;
				}
			}
		}
	}
}

void TaskManager::updateTaskThreaded(void* taskDataPtr)
{
#ifdef _WIN32
	TaskData* taskData = (TaskData*)(taskDataPtr);

	taskData->task->begin();

	while (taskData->running)
	{
		int result = taskData->task->update();
		if (result >= 0)
		{
			Sleep(result);
		}
		else break;
	}
#endif
}

void TaskManager::addTask(Task* newTask)
{
	if (numTasks < maxTasks)
	{
		tasks[numTasks].timer = &vm.config.hostSystemInterface->getTimer();
		tasks[numTasks].task = newTask;
		tasks[numTasks].nextTickTime = 0;
		tasks[numTasks].running = true;

#ifdef _WIN32
		if (!vm.config.singleThreaded)
		{
			tasks[numTasks].thread = _beginthread(TaskManager::updateTaskThreaded, 0, (void*)&tasks[numTasks]);
		}
		else
#endif
		{
			newTask->begin();
		}

		numTasks++;
	}
	else
	{
		// TODO: error?
		return;
	}
}

void TaskManager::haltAll()
{
	for (int n = 0; n < numTasks; n++)
	{
		tasks[n].running = false;

#ifdef _WIN32
		if (tasks[n].thread)
		{
			TerminateThread((HANDLE)tasks[n].thread, 0);
		}
#endif
	}
}

TaskManager::~TaskManager()
{
	haltAll();
}

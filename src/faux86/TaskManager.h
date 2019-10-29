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
#pragma once

#include "Types.h"

namespace Faux86
{
	class VM;
	class TimerInterface;

	class Task
	{
	public:
		virtual void begin() {}
		virtual int update() = 0;
	};

	class TaskManager
	{
	public:
		TaskManager(VM& inVM);
		~TaskManager();

		void addTask(Task* task);
		void tick();
		void haltAll();

	private:
		struct TaskData 
		{
			TimerInterface* timer = nullptr;
			Task* task = nullptr;
			uint64_t nextTickTime = 0;
			volatile bool running = false;
			uintptr_t thread = 0;
		};

		static void updateTaskThreaded(void* taskData);

		static constexpr int maxTasks = 3;
		int numTasks = 0;
		TaskData tasks[maxTasks];

		VM& vm;
	};
}

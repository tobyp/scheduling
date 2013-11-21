// C++11 Multithreaded Task(-let) Scheduling
// Copyright (C) 2013 tobyp

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SCHEDULING_HPP
#define SCHEDULING_HPP

#include <chrono>
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

namespace scheduling {
	template <class Clock>
	class Scheduler {
		typedef Clock clock_type;
		typedef typename clock_type::time_point time_point;
		typedef typename clock_type::duration duration;
		typedef std::function<void()> task_type;
	private:
		struct Task {
		public:
			Task (task_type&& task, const time_point& start, const duration& repeat) : task(std::move(task)), start(start), repeat(repeat) { }
			task_type task;
			time_point start;
			duration repeat;

			bool operator<(const Task& other) const {
				return start < other.start;
			}
		};
	public:
		typedef typename std::list<Task>::iterator task_handle;
	private:
		std::mutex mutex;
		std::condition_variable tasks_updated;

		std::deque<task_handle> todo;
		std::condition_variable modified;

		bool running;
		std::list<Task> tasks;
		std::list<task_handle> handles;

		std::vector<std::thread> threads;
	public:
		Scheduler();
		~Scheduler();
		task_handle schedule(task_type&& task, const time_point& start, const duration& repeat=duration::zero());
		task_handle schedule(task_type&& task, const duration& delay=duration::zero(), const duration& repeat=duration::zero());
		void unschedule(const task_handle &handle);
		void clear();

		void run();
		void halt();
	private:
		void loop();
		
	};
}

#endif

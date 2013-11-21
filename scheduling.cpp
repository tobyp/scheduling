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

#include "scheduling.hpp"

#include <algorithm>

namespace scheduling {
	template <class Clock>
	typename Scheduler<Clock>::task_handle Scheduler<Clock>::schedule(Scheduler<Clock>::task_type&& task, const Scheduler<Clock>::time_point& start, const Scheduler<Clock>::duration& repeat) {
		task_handle h;
		{
			std::lock_guard<std::mutex> lk(mutex);
			h = tasks.emplace(tasks.end(), std::move(task), start, repeat);
			handles.push_back(h);
		}
		tasks_updated.notify_all();
		return h;
	}

	template <class Clock>
	typename Scheduler<Clock>::task_handle Scheduler<Clock>::schedule(Scheduler<Clock>::task_type&& task, const Scheduler<Clock>::duration& delay, const Scheduler<Clock>::duration& repeat) {
		return schedule(std::move(task, clock_type::now()+delay, repeat));
	}

	template <class Clock>
	void Scheduler<Clock>::unschedule(const Scheduler<Clock>::task_handle& handle) {
		{
			std::lock_guard<std::mutex> lk(mutex);
			auto handle_it = std::find(handles.begin(), handles.end(), handle);
			if (handle_it != handles.end()) {
				tasks.erase(handle);
				todo.remove(handle);
				handles.erase(handle_it);
			}
		}
		tasks_updated.notify_all();
	}

	template <class Clock>
	void Scheduler<Clock>::clear() {
		{
			std::lock_guard<std::mutex> lk(mutex);
			tasks.clear();
			handles.clear();
		}
		tasks_updated.notify_all();
	}

	template <class Clock>
	void Scheduler<Clock>::loop() {
		while (true) {
			std::function<void()> f;
			{
				std::unique_lock<std::mutex> lk(mutex);
				while (todo.empty() && running) {
					modified.wait(lk);
				}
				if (!running) {
					return;
				}
				f = todo.front()->task;
				todo.pop_front();
			}
			f();
		}
	}

	template <class Clock>
	void Scheduler<Clock>::run() {
		{
			std::lock_guard<std::mutex> lk(mutex);
			if (running) return;
			running = true;
			for (auto& t : threads) {
				t = std::thread([this]{this->loop();});
			}
		}
		while (true) {
			std::unique_lock<std::mutex> lk(mutex);
			if (!running) break;

			auto task_it = min_element(tasks.begin(), tasks.end());
			time_point next_task = task_it == tasks.end() ? clock_type::time_point::max() : task_it->start;
			if (tasks_updated.wait_until(lk, next_task) == std::cv_status::timeout) {
				if (task_it->repeat != clock_type::duration::zero()) {
					task_it->start += task_it->repeat;
				}
				else {
					handles.remove(task_it);
					tasks.erase(task_it);
				}
				todo.push_back(task_it);
				modified.notify_all();
			}
		}
		for (auto& t : threads) {
			t.join();
		}
	}

	template <class Clock>
	void Scheduler<Clock>::halt() {
		{
			std::lock_guard<std::mutex> lk(mutex);
			if (!running) return;
			running = false;
		}
		tasks_updated.notify_all();
		modified.notify_all();
	}

	template <class Clock>
	Scheduler<Clock>::Scheduler() : threads(3) {

	}

	template <class Clock>
	Scheduler<Clock>::~Scheduler() {
		halt();
	}
}

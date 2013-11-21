# scheduling
C++11 Multithreaded Task(-let) Scheduling

Copyright (C) 2013 tobyp. Licensed under the GNU General Public License v3+. See the code and the LICENSE file.
***

## Overview
Schedule independent Task(-lets) to execute at predefined times, and optionally repeat at constant intervals. The tasks will be executed on 3 threads (`scheduler.cpp:127`, sorry about that), managed by the scheduler itself on the main thread.

Obviously, perfect timing cannot be guaranteed, but no task will execute before its time. A task may execute multiple times simultaneously on seperate threads if its repeat interval is shorter than its execution duration. The scheduler itself is designed to be thread-safe (although I am no expert).

## TODO
* Task prioritization, in case of "traffic jams".
* Is it really safe to execute a function object on different threads simultaneously? Are lambda closures thread-safe?
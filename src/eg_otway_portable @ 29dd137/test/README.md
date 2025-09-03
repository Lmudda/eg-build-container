# Otway Portable Tests

## Rationale

The tests use [gtest](https://github.com/google/googletest), which is pulled in by the CMake script during configuration.

The nature of the event handling system means that the way `Signal`s and `EventLoop`s are integrated depends on compile decisions. There is a small amount of code required to tie these two features together. It could be made more dynamically flexible but this feature would never be used a real program and might lead to more indirection. For this reason the testing is divided into two executables:

- Executable 1: single threaded, two versions of the TestEventLoop class scoped to their own compilation units in the same executable, which also contains most of the other test code. 
  - `Signal::emit()` is fully synchronous, and basically boils down to directly invoking connected callbacks. See `TestSignal.cpp`.
  - `Signal::emit()` places events into a queue which is then explicitly driven to process the pending events. This demonstrates deferral of the callbacks, but is directly managed from within a single thread. See `TestSignalQueue.cpp`.
  
- Executable 2: multi threaded, uses mock/event_loop/TestEventLoop.h
  - `Signal::emit()` is fully asynchronous. Events are placed into the queues of one or more independent threads. Delays are used to given those threads a change to finish processing the pending events. See `TestSignalThread.cpp`. The event loop created for this is more complicated than would be required for FreeRTOS, but expresses the same features (thread safe addition and removal of events; blocks on an empty event queue; associates a free function with the thread to add events).

- Executable 3: like 1, single threaded for separate testing of TestFlashStorageBase. 

## Building for testing and static analysis

Use the CMakeLists.txt in the test directory (make a build folder inside it, path to it, `cmake .. -DOTWAY_TARGET_PLATFORM=XYZ` where `XYZ` is `BAREMETAL` or `LINUX`, and `make`). 

See pre-requisites in the .github/workflows folders. 

### Unit tests and coverage

Tests can be run using the standard ctest approach - `make test` will run all tests. Custom make targets are provided for convenience. 
- `make run-tests` will output test results to xml.
- `make tests-to-html` will do that and then produce a readable html report using junit2html.
- `make coverage` will run the tests, then calculate the coverage using lcov, outputting a html report. For accurate line counts, run on a clean build. 

### Static analysis

Perform static analysis with [Codechecker](https://github.com/Ericsson/codechecker) using `make codechecker-report`, which outputs to html. This must be run for a clean build. 
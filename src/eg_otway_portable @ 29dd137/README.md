# Otway Portable library

This library contains a number of theoretically portably features which can be used in any project. 

The original core of the library was created to support asynchronous event handling on bare-metal systems, and abstract interfaces for peripheral driver classes, but this has grown to includes other utilities such as a generic CRC calculator.

# Target platforms for the Otway Portable library

The private folder holds platform-specific implementations of some features of the Otway Portable library. Don't include files directly from this folder or its sub-folders in your project, but select the target platform by creating a compiler definition:

    OTWAY_TARGET_PLATFORM_BAREMETAL - for microcontroller projects (we only tried STM32 so far) 
    OTWAY_TARGET_PLATFORM_FREERTOS  - for microcontroller projects using FreeRTOS for scheduling 
    OTWAY_TARGET_PLATFORM_LINUX     - for Linux applications 

The reason the split in an ostensibly portable library is that some of the core elements used for asynchronous event handling (Signals, EventLoops, Timers) have somewhat different implementations. For example, event loops store pending events in a RingBuffer for baremetal, an RTOS Queue in FreeRTOS, and a std::queue in Linux. In general, the Linux version make full use of the C++ standard library (i.e. features which involve dynamic allocation): std::map, std::function, and so on. The FreeRTOS and bare-metal implementations rely on static allocation.

# Selecting the target platform

You can define OTWAY_TARGET_PLATFORM_xxx by adding the following line to your CMakeLists.txt:

    set(OTWAY_TARGET_PLATFORM BAREMETAL) # Note there is a space here.

You can add the Otway Portable library to your project (in your CMakeLists.txt) as follows:

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/eg_otway_portable otway_portable)
    target_link_libraries(your_target PUBLIC otway_portable)

The Otway portable library's own CMakeLists.txt has the following line, which add the necessary definition:

    target_compile_definitions(${OTWAY_PORTABLE_LIB} PUBLIC OTWAY_TARGET_PLATFORM_${OTWAY_TARGET_PLATFORM})

# CI/CD

`.github/workflows/main.yml` performs the following:
- Builds and runs gtest unit tests, outputting
  - Coverage report with lcov (HTML output)
  - Test results (XML, converted to HTML output using [junit2html](https://github.com/inorton/junit2html)) 
- Static analysis with [CodeChecker](https://github.com/Ericsson/codechecker) out to HTML

For complexity analysis use the eg_otway_portable.smproj file in SourceMonitor (3.4.0.283). A Windows installer for SourceMonitor can be found: `[Shared drive]\Projects\EGS - eg - Core Library Code\07 Technical Development\04 Software\SourceMonitor\SMSetupV340.exe`, or online - SHA-1 hash should be a233f53179ac0d252db13324ce7a58b572f62894. 


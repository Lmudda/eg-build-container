cmake_minimum_required(VERSION 3.22)

include(GoogleTest) # Needed to call gtest_discover_tests

if (CMAKE_VERSION VERSION_GREATER "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

# Set target platform
if(OTWAY_TARGET_PLATFORM STREQUAL "LINUX")
    set(OTWAY_TARGET_PLATFORM "LINUX")
elseif(OTWAY_TARGET_PLATFORM STREQUAL "BAREMETAL")
    set(OTWAY_TARGET_PLATFORM "BAREMETAL")
    set(OTWAY_EVENT_LOOP_WATER_MARK TRUE)  # Required only for baremetal at the moment. 4
else()
    message("You must set the target platform as -DOTWAY_TARGET_PLATFORM=XYZ, where XYZ is BAREMETAL or LINUX.")
    return()
endif()

# Max logging level
set(OTWAY_TARGET_LOG_LEVEL 5)

set(GTEST_BINARY_NAME "test_binary")
set(SUFFIX_SINGLE_THREAD "single_thread")
set(SUFFIX_THREADED "threaded")
set(SUFFIX_FLASH_STORAGE "flash_storage")
set(SUFFIX_DISABLE_INTERRUPTS "disable_interrupts")
set(CMAKE_SUPPRESS_REGENERATION true) # To skip making additional MSVS projects
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For cppcheck - so far I've needed to set this as part of the cmake command. 
# set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*;-warnings-as-errors=*;-header-filter=.")

project(${GTEST_BINARY_NAME})

include(FetchContent)
FetchContent_Declare(
   googletest
   # Specify the commit you depend on and update it regularly.
   URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 --coverage -fno-inline -fno-inline-small-functions -fno-default-inline -fno-elide-constructors -g -O0")

# Not working yet
# set(CMAKE_CXX_CPPCHECK "cppcheck")

add_subdirectory(.. otway_portable)

add_executable(${GTEST_BINARY_NAME}_${SUFFIX_SINGLE_THREAD} 
    MainGTest.cpp 
    mock/MockCriticalSection.cpp
    mock/MockDisableInterrupts.cpp
    TestSingleThreadedUtils.cpp
    TestCRC.cpp
    TestRingBuffer.cpp
    TestMemoryPool.cpp
    TestSignal.cpp
    TestSignalQueue.cpp
    TestTimerBareMetal.cpp
    TestMockFlashMemory.cpp
    TestFlashLog.cpp
    TestLogger.cpp
    TestBlockBuffer.cpp
    TestEnumUtils.cpp)
target_link_libraries(${GTEST_BINARY_NAME}_${SUFFIX_SINGLE_THREAD} gtest_main gtest otway_portable)

add_executable(${GTEST_BINARY_NAME}_${SUFFIX_THREADED}  
    MainGTest.cpp 
    mock/MockCriticalSection.cpp
    mock/MockDisableInterrupts.cpp
    TestSignalThread.cpp)
target_link_libraries(${GTEST_BINARY_NAME}_${SUFFIX_THREADED} gtest_main gtest otway_portable)

add_executable(${GTEST_BINARY_NAME}_${SUFFIX_FLASH_STORAGE} 
    MainGTest.cpp 
    mock/MockCriticalSection.cpp
    mock/MockDisableInterrupts.cpp
    TestFlashStorageBase.cpp 
    TestSingleThreadedUtils.cpp)
target_link_libraries(${GTEST_BINARY_NAME}_${SUFFIX_FLASH_STORAGE} gtest_main gtest otway_portable)

add_executable(${GTEST_BINARY_NAME}_${SUFFIX_DISABLE_INTERRUPTS} 
    MainGTest.cpp 
    TestSingleThreadedUtils.cpp
    mock/MockCriticalSection.cpp
    TestCallback.cpp
    TestDisableInterrupts.cpp)
target_link_libraries(${GTEST_BINARY_NAME}_${SUFFIX_DISABLE_INTERRUPTS} gtest_main gtest otway_portable)

# 32 bit for tests. 
# If your build fails, you might need to fetch updated libraries:
# sudo apt-get install gcc-multilib g++-multilib
# Currently fails due to gtest build failing.
#set_target_properties(${GTEST_BINARY_NAME} PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
#set_target_properties(gtest PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
#set_target_properties(otway_portable PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")

enable_testing() # enables `make test` command
# Makes the build process aware of the gtests
gtest_discover_tests(${GTEST_BINARY_NAME}_${SUFFIX_SINGLE_THREAD})
gtest_discover_tests(${GTEST_BINARY_NAME}_${SUFFIX_THREADED})
gtest_discover_tests(${GTEST_BINARY_NAME}_${SUFFIX_FLASH_STORAGE})
gtest_discover_tests(${GTEST_BINARY_NAME}_${SUFFIX_DISABLE_INTERRUPTS}) 

## Custom targets

# Run the tests, output in junit xml format.
add_custom_target(run-tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ctest --output-junit ${CMAKE_BINARY_DIR}/test_result.xml
)
add_dependencies(run-tests ${GTEST_BINARY_NAME}_${SUFFIX_SINGLE_THREAD})
add_dependencies(run-tests ${GTEST_BINARY_NAME}_${SUFFIX_THREADED})
add_dependencies(run-tests ${GTEST_BINARY_NAME}_${SUFFIX_FLASH_STORAGE})
add_dependencies(run-tests ${GTEST_BINARY_NAME}_${SUFFIX_DISABLE_INTERRUPTS})

# Convert test xml to readable html. 
# Requires: pip install junit2html==31.0.2
add_custom_target(tests-to-html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND junit2html test_result.xml test_result.html
)
add_dependencies(tests-to-html run-tests)

# Quantify coverage of unit tests, generate html report.
# Requires: apt-get install -y libcairo2-dev=1.16.0-5ubuntu2 lcov=1.15-1
add_custom_target(coverage
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/coverage
    COMMAND geninfo . -o ${CMAKE_BINARY_DIR}/coverage.info --rc branch_coverage=1 --ignore-errors negative,mismatch
    COMMAND lcov --remove ${CMAKE_BINARY_DIR}/coverage.info -o ${CMAKE_BINARY_DIR}/coverage2.info '*/test/*' '/usr/include/*'
    COMMAND genhtml ${CMAKE_BINARY_DIR}/coverage2.info -o coverage --branch-coverage --config-file ../../.lcovrc
)
add_dependencies(coverage run-tests)

# Static analysis with CodeChecker
# Requires: pip install -Iv codechecker==6.24.4
add_custom_target(codechecker-report
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    # Ignoring the error code which will be non-zero if there are any reports.
    COMMAND CodeChecker check --build make --clean --analyzers clangsa clang-tidy cppcheck -o codechecker --ignore ${CMAKE_SOURCE_DIR}/codechecker-skip.txt || true
    COMMAND CodeChecker parse codechecker --export html -o codechecker-html
)

# Complexity analysis with Codechecker. Must be a fresh build. 
# Requires: pip install -Iv codechecker==6.24.4
add_custom_target(codechecker-complexity-report
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND CodeChecker check --build make --clean -o codechecker_complexity --ignore ${CMAKE_SOURCE_DIR}/codechecker-skip.txt --analyzers=clang-tidy --disable-all --enable readability-function-cognitive-complexity --checker-config clang-tidy:readability-function-cognitive-complexity:Threshold=20 --checker-config clang-tidy:readability-function-cognitive-complexity:DescribeBasicIncrements=true --checker-config clang-tidy:readability-function-cognitive-complexity:IgnoreMacros=true || true
    COMMAND CodeChecker parse codechecker_complexity --export html -o codechecker-complexity-html
)
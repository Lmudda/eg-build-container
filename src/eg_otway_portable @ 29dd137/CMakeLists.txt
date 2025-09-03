#/////////////////////////////////////////////////////////////////////////////////////////////
#// (c) eg technology ltd, 2024.  All rights reserved.
#//
#// This software is the property of eg technology ltd. and may not be copied or reproduced
#// otherwise than on to a single hard disk for backup or archival purposes. The source code 
#// is confidential information and must not be disclosed to third parties or used without 
#// the express written permission of eg technology ltd.
#/////////////////////////////////////////////////////////////////////////////////////////////
message("Include portable Otway library in the project (otway_portable library)")

set(OTWAY_PORTABLE_LIB "otway_portable")

add_library(${OTWAY_PORTABLE_LIB} OBJECT "")

target_compile_definitions(${OTWAY_PORTABLE_LIB} PUBLIC 
    OTWAY_TARGET_PLATFORM_${OTWAY_TARGET_PLATFORM}
    OTWAY_MAX_LOG_LEVEL=${OTWAY_TARGET_LOG_LEVEL}
    OTWAY_EVENT_LOOP_WATER_MARK=${OTWAY_EVENT_LOOP_WATER_MARK}
)

# If the CMake variable is undefined then don't create the
# compile definition, so that a default can be used.
if (DEFINED OTWAY_MAX_SIGNAL_LINKS)
    target_compile_definitions(${OTWAY_PORTABLE_LIB} PUBLIC
        OTWAY_MAX_SIGNAL_LINKS=${OTWAY_MAX_SIGNAL_LINKS}
    )
endif()

target_sources(${OTWAY_PORTABLE_LIB} PRIVATE
    # Headers added only to make them appear in Visual Studio.
    ${CMAKE_CURRENT_SOURCE_DIR}/signals/Signal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timers/Timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logging/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logging/Assert.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashStorageBase.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashStorageBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashScratchpad.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashLog.h 

    #${CMAKE_CURRENT_SOURCE_DIR}/test/mock/MockFlashMemory.h 
    
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IADCDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IAnalogueInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IAnalogueOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IBackupRam.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ICANDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ICrcDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IDigitalInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IDigitalOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFlashMemory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IFlashStorage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IHardwareTimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/II2CDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IInterCoreCriticalSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IInterCoreEventSink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IInterCoreEventSource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IPWMx4Driver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IPwmManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IRtcManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ISMBusDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ISPIDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/ISystemReset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/IUARTDriver.h

    ${CMAKE_CURRENT_SOURCE_DIR}/event_loop/BareMetalEventLoop.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/event_loop/FreeRTOSEventLoop.h 
    
    ${CMAKE_CURRENT_SOURCE_DIR}/signals/Callback.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/signals/InterruptHandler.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/signals/Signal.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/timers/Timer.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/timers/ITimer.h 
    
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/BlockBuffer.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/DisableInterrupts.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/MemoryPool.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/NonCopyable.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/RingBuffer.h
)

if (BUILD_TESTS)
    target_sources(${OTWAY_PORTABLE_LIB} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/Test/TestDigitalInput.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/Test/TestDigitalOutput.h 
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces/Test/TestSPIDriver.h 
    )
endif()


# This file is only relevant for Linux build. TODO_AC - create a portable EventLoop with 
# distinct implementations in the private folder.
if (${OTWAY_TARGET_PLATFORM} STREQUAL LINUX)
    target_sources(${OTWAY_PORTABLE_LIB} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/event_loop/ThreadEventLoop.cpp
    )
endif()

target_include_directories(${OTWAY_PORTABLE_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

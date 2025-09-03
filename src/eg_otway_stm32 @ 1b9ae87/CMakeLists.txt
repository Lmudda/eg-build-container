#/////////////////////////////////////////////////////////////////////////////////////////////
#// (c) eg technology ltd, 2024.  All rights reserved.
#//
#// This software is the property of eg technology ltd. and may not be copied or reproduced
#// otherwise than on to a single hard disk for backup or archival purposes. The source code 
#// is confidential information and must not be disclosed to third parties or used without 
#// the express written permission of eg technology ltd.
#/////////////////////////////////////////////////////////////////////////////////////////////
message("Include STM32xx Otway drivers and HAL library in the project")

if (${OTWAY_TARGET_FAMILY} STREQUAL stm32f4)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/f4)

elseif (${OTWAY_TARGET_FAMILY} STREQUAL stm32g4)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/g4)

elseif (${OTWAY_TARGET_FAMILY} STREQUAL stm32h7)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/h7)

elseif (${OTWAY_TARGET_FAMILY} STREQUAL stm32u5)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/u5)

else()
    message(FATAL_ERROR "OTWAY_TARGET_FAMILY is not set (options are stm32f4, stm32g4, stm32h7, stm32u5)") 
endif()
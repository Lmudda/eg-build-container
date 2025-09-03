#/////////////////////////////////////////////////////////////////////////////////////////////
#// (c) eg technology ltd, 2024.  All rights reserved.
#//
#// This software is the property of eg technology ltd. and may not be copied or reproduced
#// otherwise than on to a single hard disk for backup or archival purposes. The source code 
#// is confidential information and must not be disclosed to third parties or used without 
#// the express written permission of eg technology ltd.
#/////////////////////////////////////////////////////////////////////////////////////////////
message("Include STM32U5 Otway drivers and HAL library in the project (otway_stm32u5 library)")


# Library containing the Otway drivers
set(OTWAY_STM32U5_LIB "otway_stm32u5")

add_library(${OTWAY_STM32U5_LIB} OBJECT "")

target_sources(${OTWAY_STM32U5_LIB} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/drivers/SpiDriverDMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/RtcManager.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashInternal.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashMemory.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SpiDriverDMA.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PwmManager.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInput.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.cpp 
        
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.cpp 
    
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/DisableInterrupts.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInput.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriver.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/RtcManager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PwmManager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SpiDriverDMA.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashInternal.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FlashMemory.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.h 
)

target_include_directories(${OTWAY_STM32U5_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${OTWAY_STM32U5_LIB} PUBLIC
    otway_portable
    otway_stm32u5_hal
)


# Library containing the HAL code
# This might be better placed in the STM32 submodule folder but we don't control that.
set(OTWAY_STM32U5_HAL_LIB "otway_stm32u5_hal")
set(STM32U5_DRIVERS_BASE_DIR ${PROJECT_SOURCE_DIR}/STM32CubeU5/Drivers)
# Convenience variables 
set(STM32U5_HAL_DRIVERS_SRC_DIR ${STM32U5_DRIVERS_BASE_DIR}/STM32U5xx_HAL_Driver/Src)

add_library(${OTWAY_STM32U5_HAL_LIB} OBJECT "")

target_sources(${OTWAY_STM32U5_HAL_LIB} PRIVATE
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_rcc.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_pwr.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_spi.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_spi_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_rcc_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_uart.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_uart_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_tim_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_dma_ex.c
    #${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_flash_ramfunc.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_dma.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_flash_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_cortex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_exti.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_tim.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_gpio.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_adc.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_adc_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_flash.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_pwr_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_rtc.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_rtc_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_i2c.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_i2c_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_lptim.c
    # Added to support USB
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_pcd.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_hal_pcd_ex.c
    ${STM32U5_HAL_DRIVERS_SRC_DIR}/stm32u5xx_ll_usb.c
)

target_compile_definitions(${OTWAY_STM32U5_HAL_LIB} PUBLIC 
    ${OTWAY_TARGET_DEVICE}  
)

# This is to get access to the HAL config file. CMake complained about a cyclic 
# dependency between OBJECT libraries when using target_link_libraries() to get the includes. 
get_property(otway_cube_include_dirs TARGET otway_cube PROPERTY INCLUDE_DIRECTORIES)

target_include_directories(${OTWAY_STM32U5_HAL_LIB} PUBLIC
    ${STM32U5_DRIVERS_BASE_DIR}/STM32U5xx_HAL_Driver/Inc
    ${STM32U5_DRIVERS_BASE_DIR}/CMSIS/Device/ST/STM32U5xx/Include
    ${STM32U5_DRIVERS_BASE_DIR}/CMSIS/Core/Include
    ${otway_cube_include_dirs}
)

# CMake complains about a cyclic dependency here. We obtain the includes paths in another way.
# target_link_libraries(${OTWAY_STM32U5_HAL_LIB} PUBLIC
#     # This is to get access to the HAL config file.
#     otway_cube
# )

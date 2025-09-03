#/////////////////////////////////////////////////////////////////////////////////////////////
#// (c) eg technology ltd, 2024.  All rights reserved.
#//
#// This software is the property of eg technology ltd. and may not be copied or reproduced
#// otherwise than on to a single hard disk for backup or archival purposes. The source code
#// is confidential information and must not be disclosed to third parties or used without
#// the express written permission of eg technology ltd.
#/////////////////////////////////////////////////////////////////////////////////////////////
message("Include STM32G4 Otway drivers and HAL library in the project (otway_stm32g4 library)")


# Library containing the Otway drivers
set(OTWAY_STM32G4_LIB "otway_stm32g4")

add_library(${OTWAY_STM32G4_LIB} OBJECT "")

target_compile_definitions(${OTWAY_STM32G4_LIB} 
    PUBLIC OTWAY_TARGET_PLATFORM_${OTWAY_TARGET_PLATFORM}
    # This should be imported from Otway portable anyway.
    #OTWAY_MAX_LOG_LEVEL=${OTWAY_TARGET_LOG_LEVEL}
)

target_sources(${OTWAY_STM32G4_LIB} PRIVATE
    # Headers added only to make them appear in Visual Studio.

    # Implementations for hardware peripheral driver interfaces.
    # Alphabetical for not particular reason.
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriverBlocking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriverBlocking.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FDCANDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FDCANDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PWMx4Driver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PWMx4Driver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/RTCDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/RTCDriver.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/drivers/RTCManager.h
    #${CMAKE_CURRENT_SOURCE_DIR}/drivers/RTCManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SMBusDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SMBusDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SPIDriverDMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SPIDriverDMA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverDMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverDMA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverUSB.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverUSB.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/WatchdogTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/WatchdogTimer.h

    # Command code which can be used to underlie the driver implementations and
    # partially isolate them from the particular STM32G4 device in use.
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/CANHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/CANHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.h

    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/CriticalSection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/DisableInterrupts.cpp

    # These files were copied into this library from an STM32Cube project. They
    # are just boilerplate which appears numerous times in the STM32CubeG4 repo
    # (identical files in every example and template project). Currently still
    # including them in the project from the "cube" folder.
    #${CMAKE_CURRENT_SOURCE_DIR}/syscalls/syscalls.c
    #${CMAKE_CURRENT_SOURCE_DIR}/syscalls/sysmem.c
)

target_include_directories(${OTWAY_STM32G4_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${OTWAY_STM32G4_LIB} PUBLIC
    otway_portable
    otway_stm32g4_hal
)

# Library containing the HAL code
# This might be better placed in the STM32 submodule folder but we don't control that.
set(OTWAY_STM32G4_HAL_LIB "otway_stm32g4_hal")
set(STM32G4_BASE_DIR ${PROJECT_SOURCE_DIR}/../submodules/STM32CubeG4)
set(STM32G4_DRIVERS_BASE_DIR ${STM32G4_BASE_DIR}/Drivers)
# Convenience variables
set(STM32G4_HAL_DRIVERS_SRC_DIR ${STM32G4_DRIVERS_BASE_DIR}/STM32G4xx_HAL_Driver/Src)
set(STM32G4_USB_DEVICE_LIB_DIR ${STM32G4_BASE_DIR}/Middlewares/ST/STM32_USB_Device_Library)
set(STM32G4_FREERTOS_LIB_DIR ${STM32G4_BASE_DIR}//Middlewares/Third_Party/FreeRTOS/Source)

add_library(${OTWAY_STM32G4_HAL_LIB} OBJECT "")

target_sources(${OTWAY_STM32G4_HAL_LIB} PRIVATE
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_adc.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_adc_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_comp.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_cordic.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_cortex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_crc.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_crc_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_cryp.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_cryp_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_dac.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_dac_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_dma.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_dma_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_exti.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_fdcan.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_flash.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_flash_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_flash_ramfunc.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_fmac.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_gpio.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_hrtim.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_i2c.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_i2c_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_i2s.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_irda.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_iwdg.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_lptim.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_msp_template.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_nand.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_nor.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_opamp.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_opamp_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_pcd.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_pcd_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_pwr.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_pwr_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_qspi.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_rcc.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_rcc_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_rng.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_rtc.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_rtc_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_sai.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_sai_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_smartcard.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_smartcard_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_smbus.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_smbus_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_spi.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_spi_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_sram.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_tim.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_timebase_tim_template.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_tim_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_uart.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_uart_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_usart.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_usart_ex.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_hal_wwdg.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_adc.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_comp.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_cordic.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_crc.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_crs.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_dac.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_dma.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_exti.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_fmac.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_fmc.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_gpio.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_hrtim.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_i2c.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_lptim.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_lpuart.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_opamp.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_pwr.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_rcc.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_rng.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_rtc.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_spi.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_tim.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_ucpd.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_usart.c
    ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_usb.c
    # ${STM32G4_HAL_DRIVERS_SRC_DIR}/stm32g4xx_ll_utils.c

    ###################################################################################
    # USB stack
    ###################################################################################
    # Common files for the USB stack. The template files need to be created for
    # each project, though they may have a lot of common boilerplate. This can be
    # done with STM32Cube.
    #${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_def.h
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Src/usbd_core.c
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_core.h
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Src/usbd_ctlreq.c
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_ctlreq.h
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Src/usbd_ioreq.c
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_ioreq.h
    # Two way bridge to tie the stack to the PCD hardware peripheral.
    #${STM32G4_USB_DEVICE_LIB_DIR}/Core/Src/usbd_conf_template.c
    #${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_conf_template.h
    # Callbacks which return the various device descriptors.
    #${STM32G4_USB_DEVICE_LIB_DIR}/Core/Src/usbd_desc_template.c
    #${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_desc_template.h

    # Files for the particular USB device class. The template files need to
    # be created for each project, though they may have a lot of common boilerplate.
    # This can be done with STM32Cube.
    ${STM32G4_USB_DEVICE_LIB_DIR}/Class/CDC/Src/usbd_cdc.c
    ${STM32G4_USB_DEVICE_LIB_DIR}/Class/CDC/Inc/usbd_cdc.h
    # Callback interface for the CDC class.
    #${STM32G4_USB_DEVICE_LIB_DIR}/Class/CDC/Src/usbd_cdc_if_template.c
    #${STM32G4_USB_DEVICE_LIB_DIR}/Class/CDC/Inc/usbd_cdc_if_template.h

    ###################################################################################
    # FreeRTOS
    ###################################################################################
    ${STM32G4_FREERTOS_LIB_DIR}/croutine.c
    ${STM32G4_FREERTOS_LIB_DIR}/event_groups.c
    ${STM32G4_FREERTOS_LIB_DIR}/list.c
    ${STM32G4_FREERTOS_LIB_DIR}/queue.c
    ${STM32G4_FREERTOS_LIB_DIR}/stream_buffer.c
    ${STM32G4_FREERTOS_LIB_DIR}/tasks.c
    ${STM32G4_FREERTOS_LIB_DIR}/timers.c
    ${STM32G4_FREERTOS_LIB_DIR}/CMSIS_RTOS_V2/cmsis_os2.c
    ${STM32G4_FREERTOS_LIB_DIR}/portable/GCC/ARM_CM4F/port.c
    # We are not using any dynamic allocation features with FreeRTOS. But could use
    # one of the heaps if necessary. Heap #1 is probably good enough.
    #${STM32G4_FREERTOS_LIB_DIR}/portable/MemMang/heap_1.c
    #${STM32G4_FREERTOS_LIB_DIR}/portable/MemMang/heap_2.c
    #${STM32G4_FREERTOS_LIB_DIR}/portable/MemMang/heap_3.c
    #${STM32G4_FREERTOS_LIB_DIR}/portable/MemMang/heap_4.c
    )

target_compile_definitions(${OTWAY_STM32G4_HAL_LIB} PUBLIC
    ${OTWAY_TARGET_DEVICE}
)

# This is to get access to the HAL config file. CMake complained about a cyclic
# dependency between OBJECT libraries when using target_link_libraries() to get the includes.
get_property(otway_cube_include_dirs TARGET otway_cube PROPERTY INCLUDE_DIRECTORIES)

target_include_directories(${OTWAY_STM32G4_HAL_LIB} PUBLIC
    ${STM32G4_DRIVERS_BASE_DIR}/STM32G4xx_HAL_Driver/Inc
    ${STM32G4_DRIVERS_BASE_DIR}/CMSIS/Device/ST/STM32G4xx/Include
    ${STM32G4_DRIVERS_BASE_DIR}/CMSIS/Core/Include
    ${otway_cube_include_dirs}

    ${STM32G4_USB_DEVICE_LIB_DIR}/Class/CDC/Inc
    ${STM32G4_USB_DEVICE_LIB_DIR}/Core/Inc

    ${STM32G4_FREERTOS_LIB_DIR}/include
    ${STM32G4_FREERTOS_LIB_DIR}/CMSIS_RTOS_V2
    ${STM32G4_FREERTOS_LIB_DIR}/portable/GCC/ARM_CM4F
)

# CMake complains about a cyclic dependency here. We obtain the includes paths in another way.
# target_link_libraries(${OTWAY_STM32G4_HAL_LIB} PUBLIC
#     # This is to get access to the HAL config file.
#     otway_cube
# )

#/////////////////////////////////////////////////////////////////////////////////////////////
#// (c) eg technology ltd, 2024.  All rights reserved.
#//
#// This software is the property of eg technology ltd. and may not be copied or reproduced
#// otherwise than on to a single hard disk for backup or archival purposes. The source code
#// is confidential information and must not be disclosed to third parties or used without
#// the express written permission of eg technology ltd.
#/////////////////////////////////////////////////////////////////////////////////////////////
message("Include STM32H7 Otway drivers and HAL library in the project (otway_stm32h7 library)")


# Library containing the Otway drivers
set(OTWAY_STM32H7_LIB "otway_stm32h7")

add_library(${OTWAY_STM32H7_LIB} OBJECT "")

target_compile_definitions(${OTWAY_STM32H7_LIB} PUBLIC OTWAY_TARGET_PLATFORM_${OTWAY_TARGET_PLATFORM})

target_sources(${OTWAY_STM32H7_LIB} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInputBlocking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriverBlocking.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriverMultiChannel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/BootSequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/CrcDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/CycleCounter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FDCANDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriverSequence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreCriticalSection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreEventSink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreEventSource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PWMx4Driver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SPIDriverDMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverDMA.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/WatchdogTimer.cpp


    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/CANHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/HSEMHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/DisableInterrupts.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/CriticalSection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utilities/AtomicReadModifyWrite.h

    # Headers added only to make them appear in Visual Studio.
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueInputBlocking.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/AnalogueOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriverBlocking.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/ADCDriverMultiChannel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/BackupRam.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/BootSequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/CycleCounter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/CrcDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalInputPolled.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/DigitalOutput.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/FDCANDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/HardwareTimer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/I2CDriverSequence.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreCriticalSection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreEventSink.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/InterCoreEventSource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/PWMx4Driver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/SPIDriverDMA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/UARTDriverDMA.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/WatchdogTimer.h

    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/ADCHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/CANHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/DMAHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/GPIOHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/HSEMHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/I2CHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/SPIHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/TIMHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers/helpers/UARTHelpers.h


    # These files were copied into this library from an STM32Cube project. They
    # are just boilerplate which appears numerous times in the STM32CubeH7 repo
    # (identical files in every example and template project). Currently still
    # including them in the project from the "cube" folder.
    #${CMAKE_CURRENT_SOURCE_DIR}/syscalls/syscalls.c
    #${CMAKE_CURRENT_SOURCE_DIR}/syscalls/sysmem.c
)

target_include_directories(${OTWAY_STM32H7_LIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${OTWAY_STM32H7_LIB} PUBLIC
    otway_portable
    otway_stm32h7_hal
)


# Library containing the HAL code
# This might be better placed in the STM32 submodule folder but we don't control that.
set(OTWAY_STM32H7_HAL_LIB "otway_stm32h7_hal")
set(STM32H7_BASE_DIR ${PROJECT_SOURCE_DIR}/../submodules/STM32CubeH7)
set(STM32H7_DRIVERS_BASE_DIR ${STM32H7_BASE_DIR}/Drivers)
# Convenience variables
set(STM32H7_HAL_DRIVERS_SRC_DIR ${STM32H7_DRIVERS_BASE_DIR}/STM32H7xx_HAL_Driver/Src)
set(STM32H7_USB_DEVICE_LIB_DIR ${STM32H7_BASE_DIR}/Middlewares/ST/STM32_USB_Device_Library)
set(STM32H7_FREERTOS_LIB_DIR ${STM32H7_BASE_DIR}//Middlewares/Third_Party/FreeRTOS/Source)

add_library(${OTWAY_STM32H7_HAL_LIB} OBJECT "")

target_sources(${OTWAY_STM32H7_HAL_LIB} PRIVATE
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_adc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_adc_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_cec.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_comp.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_cordic.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_cortex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_crc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_crc_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_cryp.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_cryp_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dac.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dac_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dcmi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dfsdm.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dfsdm_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dma.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dma_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dma2d.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dsi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_dts.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_eth.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_eth_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_exti.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_fdcan.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_flash.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_flash_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_fmac.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_gfxmmu.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_gpio.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_hash.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_hash_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_hcd.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_hrtim.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_hsem.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_i2c.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_i2c_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_i2s.c
    # The following file is empty, so is excluded to avoid a pedantic compiler warning
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_i2s_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_irda.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_iwdg.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_jpeg.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_lptim.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_ltdc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_ltdc_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_mdios.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_mdma.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_mmc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_mmc_ex.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_msp_template.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_nand.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_nor.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_opamp.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_opamp_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_ospi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_otfdec.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_pcd.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_pcd_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_pssi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_pwr.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_pwr_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_qspi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_ramecc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rcc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rcc_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rng.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rng_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rtc.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_rtc_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sai.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sai_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sd.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sd_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sdram.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_smartcard.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_smartcard_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_smbus.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_smbus_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_spdifrx.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_spi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_spi_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_sram.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_swpmi.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_tim.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_tim_ex.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_timebase_rtc_alarm_template.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_timebase_rtc_wakeup_template.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_timebase_tim_template.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_uart.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_uart_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_usart.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_usart_ex.c
    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_hal_wwdg.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_adc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_bdma.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_comp.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_cordic.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_crc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_crs.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_dac.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_delayblock.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_dma.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_dma2d.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_exti.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_fmac.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_fmc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_gpio.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_hrtim.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_i2c.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_lptim.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_lpuart.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_mdma.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_opamp.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_pwr.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_rcc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_rng.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_rtc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_sdmmc.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_spi.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_swpmi.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_tim.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_usart.c
#    ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_usb.c
    # ${STM32H7_HAL_DRIVERS_SRC_DIR}/stm32h7xx_ll_utils.c

#    ###################################################################################
#    # USB stack
#    ###################################################################################
#    # Common files for the USB stack. The template files need to be created for
#    # each project, though they may have a lot of common boilerplate. This can be
#    # done with STM32Cube.
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_def.h
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Src/usbd_core.c
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_core.h
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Src/usbd_ctlreq.c
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_ctlreq.h
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Src/usbd_ioreq.c
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_ioreq.h
#    # Two way bridge to tie the stack to the PCD hardware peripheral.
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Core/Src/usbd_conf_template.c
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_conf_template.h
#    # Callbacks which return the various device descriptors.
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Core/Src/usbd_desc_template.c
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc/usbd_desc_template.h
#
#    # Files for the particular USB device class. The template files need to
#    # be created for each project, though they may have a lot of common boilerplate.
#    # This can be done with STM32Cube.
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Src/usbd_cdc.c
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Inc/usbd_cdc.h
#    # Callback interface for the CDC class.
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Src/usbd_cdc_if_template.c
#    #${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Inc/usbd_cdc_if_template.h

    ###################################################################################
    # CMSIS
    ###################################################################################
    ${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Inc/usbd_cdc_if_template.h

#    ###################################################################################
#    # FreeRTOS
#    ###################################################################################
#    ${STM32H7_FREERTOS_LIB_DIR}/croutine.c
#    ${STM32H7_FREERTOS_LIB_DIR}/event_groups.c
#    ${STM32H7_FREERTOS_LIB_DIR}/list.c
#    ${STM32H7_FREERTOS_LIB_DIR}/queue.c
#    ${STM32H7_FREERTOS_LIB_DIR}/stream_buffer.c
#    ${STM32H7_FREERTOS_LIB_DIR}/tasks.c
#    ${STM32H7_FREERTOS_LIB_DIR}/timers.c
#    ${STM32H7_FREERTOS_LIB_DIR}/CMSIS_RTOS_V2/cmsis_os2.c
#    ${STM32H7_FREERTOS_LIB_DIR}/portable/GCC/ARM_CM4F/port.c
#    # We are not using any dynamic allocation features with FreeRTOS. But could use
#    # one of the heaps if necessary. Heap #1 is probably good enough.
#    #${STM32H7_FREERTOS_LIB_DIR}/portable/MemMang/heap_1.c
#    #${STM32H7_FREERTOS_LIB_DIR}/portable/MemMang/heap_2.c
#    #${STM32H7_FREERTOS_LIB_DIR}/portable/MemMang/heap_3.c
#    #${STM32H7_FREERTOS_LIB_DIR}/portable/MemMang/heap_4.c
    )

target_compile_definitions(${OTWAY_STM32H7_HAL_LIB} PUBLIC
    ${OTWAY_TARGET_DEVICE}
    CORE_${OTWAY_TARGET_CORE}
)

# This is to get access to the HAL config file. CMake complained about a cyclic
# dependency between OBJECT libraries when using target_link_libraries() to get the includes.
get_property(otway_cube_include_dirs TARGET otway_cube PROPERTY INCLUDE_DIRECTORIES)

target_include_directories(${OTWAY_STM32H7_HAL_LIB} PUBLIC
    ${STM32H7_DRIVERS_BASE_DIR}/STM32H7xx_HAL_Driver/Inc
    ${STM32H7_DRIVERS_BASE_DIR}/CMSIS/Device/ST/STM32H7xx/Include
    ${STM32H7_DRIVERS_BASE_DIR}/CMSIS/Core/Include
    ${otway_cube_include_dirs}

#    ${STM32H7_USB_DEVICE_LIB_DIR}/Class/CDC/Inc
#    ${STM32H7_USB_DEVICE_LIB_DIR}/Core/Inc

#    ${STM32H7_FREERTOS_LIB_DIR}/include
#    ${STM32H7_FREERTOS_LIB_DIR}/CMSIS_RTOS_V2
#    ${STM32H7_FREERTOS_LIB_DIR}/portable/GCC/ARM_CM4F
)

# CMake complains about a cyclic dependency here. We obtain the includes paths in another way.
# target_link_libraries(${OTWAY_STM32H7_HAL_LIB} PUBLIC
#     # This is to get access to the HAL config file.
#     otway_cube
# )

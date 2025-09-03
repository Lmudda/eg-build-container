/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "helpers/GPIOHelpers.h"
#include "helpers/UARTHelpers.h"
#include "interfaces/IUARTDriver.h"
#include "utilities/BlockBuffer.h"
#include "stm32g4xx_hal.h"
#include "usbd_cdc.h"
#include <cstdint>


namespace eg {


// This class is a wrapper for the USB CDC implementation generated/copied into the project
// by STM32CubeIDE. The aim is to pull together some of the key structures which are passed 
// to ST's core USB implementation and ST's USB CDC implementation. These structures are 
// largely in the form of tables of callbacks. One of the key structures is the device descriptor,
// which is represented by a compile-time configuration passed to the constructor. 
class UARTDriverUSBBase : public IUARTDriver
{
public:
    struct DescriptorConfig
    {
        uint16_t    VendorID;
        uint16_t    LanguageID;
        uint16_t    ProductID;
        uint16_t    DeviceID;
        const char* ManufacturerString;
        const char* ProductString;
        const char* ConfigurationString;
        const char* InterfaceString;
    };

public:
    UARTDriverUSBBase(const DescriptorConfig& conf, BlockBuffer& tx_buffer);
    void write(const uint8_t* data, uint16_t length) override;
    void write(const char* data) override;
    SignalProxy<RXData> on_rx_data() override { return SignalProxy<RXData>{m_on_rx_data}; }
    SignalProxy<> on_error() override { return SignalProxy<>{m_on_error}; };

private:
    uint8_t transmit();

    int8_t init_cb();
    int8_t deinit_cb();
    int8_t control_cb(uint8_t cmd, uint8_t* buffer, uint16_t length);
    int8_t receive_cb(uint8_t* buffer, uint32_t* length);
    int8_t transmit_complete_cb(uint8_t* buffer, uint32_t* length, uint8_t ep_num);

    // Interface callbacks needed for m_interface_ops.
    static int8_t Init();
    static int8_t DeInit();
    static int8_t Control(uint8_t cmd, uint8_t* buffer, uint16_t length);
    static int8_t Receive(uint8_t* buffer, uint32_t* length);
    static int8_t TransmitCplt(uint8_t* buffer, uint32_t* length, uint8_t ep_num);

    // Interface callbacks needed for m_descriptor_ops.
    static uint8_t* GetDeviceDescriptor          (USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetLangIDStrDescriptor       (USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetManufacturerStrDescriptor (USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetProductStrDescriptor      (USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetSerialStrDescriptor       (USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetConfigurationStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length);
    static uint8_t* GetInterfaceStrDescriptor    (USBD_SpeedTypeDef speed, uint16_t* length);

private:
    // We need this to elevate calls from the static functions. Since we have only 
    // a single instance of the USB hardware/driver, maybe we can make the class 
    // entirely static. The current design is similar to other drivers and that's 
    // perhaps more familiar, even if it is a singleton. 
    static inline UARTDriverUSBBase* m_self;

    const DescriptorConfig& m_conf;

    // This was originally defined as hUsbDeviceFS in usb_device.c (a generated file).
    USBD_HandleTypeDef m_usb_handle{};

    // This is a table of callbacks used by the stack to retrieve descriptors for the device.
    // The object is defined (for now) in usbd_desc.c (a generated file). It is desirable to 
    // separate the values for the descriptors from the implementation. These are currently
    // macros defined in usbd_desc.c. Need to wrap descriptors in a cleaner way.
    USBD_DescriptorsTypeDef m_descriptor_ops{}; 

    // This is a table of callbacks used by the statck to implement CDC device class. 
    // The object is defined in usbd_cdc.c (a library file we don't touch).
    //USBD_ClassTypeDef  m_cdc_class_ops; // If we implemented the class here

    // This is a table of callbacks used by the stack to tell the application about USB
    // events such as data received, transmission complete, and so on.  
    // The object was originally defined in usbd_cdc if.c (a generated file). We 
    // incorportate it here to more cleanly integrate the callback with the application.
    USBD_CDC_ItfTypeDef m_interface_ops{};

    using TXBuffer = BlockBuffer;
    using TXBlock  = TXBuffer::Block;
    TXBuffer&     m_tx_buffer; 
    TXBlock       m_tx_block{};
    bool          m_tx_busy{false}; 
    // This is a workaround to prevent the TX buffer blocking forever on first transmit. 
    // Only allow TX items to be queued after we have received RX - this shows that the 
    // USB UART is up and running. Better to work out the error condition in the driver.
    bool          m_tx_enabled{false};

    // NOTE: Sort this lot out. Do we need the TX at all? We initially set the TX buffer
    // to zero length anyway. The RX buffer may not need to be bigger than 64 bytes.
    uint8_t m_tx_buffer_temp[256];
    uint8_t m_rx_buffer_temp[256];
    
    Signal<RXData> m_on_rx_data{};
    Signal<>       m_on_error{};
};


template<uint16_t TX_BUFFER_SIZE>
class UARTDriverUSB : public UARTDriverUSBBase
{
public:
    UARTDriverUSB(const DescriptorConfig& conf) 
    : UARTDriverUSBBase{conf, m_data}
    {        
    }

private:
    BlockBufferArray<TX_BUFFER_SIZE> m_data;
};


} // namespace eg {



/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "UARTDriverUSB.h"
#include "utilities/ErrorHandler.h"
#include "usbd_cdc.h"
#include "stm32g4xx.h"
#include "stm32g4xx_hal.h"
#include "drivers/helpers/GlobalDefs.h"


namespace eg {


UARTDriverUSBBase::UARTDriverUSBBase(const DescriptorConfig& conf, BlockBuffer& tx_buffer)
: m_conf{conf}
, m_tx_buffer{tx_buffer}
{
    // Enforce single instance nature of the class. Destructor should
    // perhaps reset the pointer for the purposes of testing.
    if (m_self != nullptr)
    {
        Error_Handler();
    }
    m_self = this;

    // These callbacks allow the USB stack to tell us about events.
    // They are all called in interrupt context.
    m_interface_ops.Init         = &UARTDriverUSBBase::Init;
    m_interface_ops.DeInit       = &UARTDriverUSBBase::DeInit;
    m_interface_ops.Control      = &UARTDriverUSBBase::Control;
    m_interface_ops.Receive      = &UARTDriverUSBBase::Receive;
    m_interface_ops.TransmitCplt = &UARTDriverUSBBase::TransmitCplt;

    m_descriptor_ops.GetDeviceDescriptor           = &UARTDriverUSBBase::GetDeviceDescriptor;
    m_descriptor_ops.GetLangIDStrDescriptor        = &UARTDriverUSBBase::GetLangIDStrDescriptor;
    m_descriptor_ops.GetManufacturerStrDescriptor  = &UARTDriverUSBBase::GetManufacturerStrDescriptor;
    m_descriptor_ops.GetProductStrDescriptor       = &UARTDriverUSBBase::GetProductStrDescriptor;
    m_descriptor_ops.GetSerialStrDescriptor        = &UARTDriverUSBBase::GetSerialStrDescriptor;
    m_descriptor_ops.GetConfigurationStrDescriptor = &UARTDriverUSBBase::GetConfigurationStrDescriptor;
    m_descriptor_ops.GetInterfaceStrDescriptor     = &UARTDriverUSBBase::GetInterfaceStrDescriptor;

    // This constructor replaces MX_USB_Device_Init(). The code from that
    // function is implemented below with structures owned here.
    if (USBD_Init(&m_usb_handle, &m_descriptor_ops, DEVICE_FS) != USBD_OK)
    //if (USBD_Init(&m_usb_handle, &CDC_Desc, DEVICE_FS) != USBD_OK)
    {
        Error_Handler();
    }
    // If we implemented the CDC class ourselves.
    //if (USBD_RegisterClass(&m_usb_handle, &m_cdc_class_ops) != USBD_OK)
    if (USBD_RegisterClass(&m_usb_handle, &USBD_CDC) != USBD_OK)
    {
        Error_Handler();
    }
    if (USBD_CDC_RegisterInterface(&m_usb_handle, &m_interface_ops) != USBD_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USB_LP_IRQn, GlobalsDefs::DefPreemptPrio, GlobalsDefs::DefSubPrio);

    if (USBD_Start(&m_usb_handle) != USBD_OK)
    {
        Error_Handler();
    }

    // Set up the buffers used by the stack. How big do these need
    // to be. Examples generates large buffers but the packet size is
    // only up to 64 bytes for FS USB. How does the stack know how
    // big the RX buffer is?
    USBD_CDC_SetTxBuffer(&m_usb_handle, &m_tx_buffer_temp[0], 0); // Zero length ?
    // Setup for the first packet received.
    USBD_CDC_SetRxBuffer(&m_usb_handle, &m_rx_buffer_temp[0]);
    USBD_CDC_ReceivePacket(&m_usb_handle);
}


void UARTDriverUSBBase::write(const uint8_t* data, uint16_t length)
{
    CriticalSection cs;

    // NOTE: We need a further check that the device is ready. That is, it has been
    // enumerated and initialised. Sending data before this breaks the driver by never
    // resetting m_tx_busy because, I guess, the first transmit is lost. Also should as a
    // timeout and maybe flush the TX buffer.
    if (!m_tx_enabled) return;

    if (m_tx_buffer.append({ data, length }))
    {
        if (!m_tx_busy)
        {
            transmit();
        }
    }
}


void UARTDriverUSBBase::write(const char* data)
{
    write(reinterpret_cast<const uint8_t*>(data), strlen(data));
}


uint8_t UARTDriverUSBBase::transmit()
{
    // The transfer finished. Called in ISR context or with interrupts disabled.
    CriticalSection cs;

    // Clean up the just-sent block.
    if (m_tx_busy)
    {
        m_tx_buffer.remove(m_tx_block);
    }

    // Decide whether to start another block.
    if (m_tx_buffer.size() > 0)
    {
        m_tx_busy  = true;
        m_tx_block = m_tx_buffer.front();

        uint8_t* tx_buffer = const_cast<uint8_t*>(m_tx_block.buffer);
        USBD_CDC_SetTxBuffer(&m_usb_handle, tx_buffer, m_tx_block.length);
        return USBD_CDC_TransmitPacket(&m_usb_handle);
    }
    else
    {
        m_tx_busy  = false;
        m_tx_block = TXBlock {};
    }

    return USBD_OK;
}


int8_t UARTDriverUSBBase::init_cb()
{
    return USBD_OK;
}


int8_t UARTDriverUSBBase::deinit_cb()
{
    return USBD_OK;
}


int8_t UARTDriverUSBBase::control_cb(uint8_t cmd, uint8_t* buffer, uint16_t length)
{
    // Some examples store data about the virtual COM port speed etc.
    return USBD_OK;
}


int8_t UARTDriverUSBBase::receive_cb(uint8_t* buffer, uint32_t* length)
{
    // Data has been received. Process it and set up for the next receive.
    uint32_t count  = *length;
    uint32_t offset = 0;
    while (count > 0)
    {
        RXData rx{};
        rx.length = std::min<uint32_t>(count, RXData::MaxData);
        std::memcpy(rx.data, &buffer[offset], rx.length);
        m_on_rx_data.emit(rx);

        offset += rx.length;
        count  -= rx.length;
    }

    // TX bug workaround. Now that we have received data from the other side, we
    // assume the USB UART connection is up and running. Now we can transmit messages.
    m_tx_enabled = true;

    // Setup for the next packet received.
    USBD_CDC_SetRxBuffer(&m_usb_handle, &m_rx_buffer_temp[0]);
    USBD_CDC_ReceivePacket(&m_usb_handle);
    return USBD_OK;
}


int8_t UARTDriverUSBBase::transmit_complete_cb(uint8_t* buffer, uint32_t* length, uint8_t ep_num)
{
    // Should we pay attention to the arguments here? We used m_tx_block instead.
    // Data has been transmitted. Check the TX buffer and transmit some more.
    return transmit();
}


int8_t UARTDriverUSBBase::Init()
{
    if (!UARTDriverUSBBase::m_self)
    {
        Error_Handler();
    }
    return m_self->init_cb();
}


int8_t UARTDriverUSBBase::DeInit()
{
    if (!UARTDriverUSBBase::m_self)
    {
        Error_Handler();
    }
    return m_self->deinit_cb();
}


int8_t UARTDriverUSBBase::Control(uint8_t cmd, uint8_t* buffer, uint16_t length)
{
    if (!UARTDriverUSBBase::m_self)
    {
        Error_Handler();
    }
    return m_self->control_cb(cmd, buffer, length);
}


int8_t UARTDriverUSBBase::Receive(uint8_t* buffer, uint32_t* length)
{
    if (!UARTDriverUSBBase::m_self)
    {
        Error_Handler();
    }
    return m_self->receive_cb(buffer, length);
}


int8_t UARTDriverUSBBase::TransmitCplt(uint8_t* buffer, uint32_t* length, uint8_t ep_num)
{
    if (!UARTDriverUSBBase::m_self)
    {
        Error_Handler();
    }
    return m_self->transmit_complete_cb(buffer, length, ep_num);
}


extern "C" void USB_LP_IRQHandler()
{
    extern PCD_HandleTypeDef hpcd_USB_FS;
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}


static uint8_t lo_byte(uint16_t value) { return static_cast<uint8_t>(value & 0x00FF); }
static uint8_t hi_byte(uint16_t value) { return static_cast<uint8_t>((value & 0xFF00) >> 8); }


// This structure is probably overkill, but I wanted to clearly document the format of the
// descriptor. Source "USB in a nutshell": https://www.beyondlogic.org/usbnutshell/usb1.shtml.
// struct __attribute__((packed)) DeviceDescriptor
struct DeviceDescriptor
{
    // Number: Size of the Descriptor in Bytes (18 bytes)
    uint8_t  bLength;
    // Constant: Device Descriptor (0x01)
    uint8_t  bDescriptorType;
    // BCD: USB Specification Number which device complies to.
    // e.g. USB 2.0 is reported as 0x0200 (little endian)
    uint16_t bcdUSB;
    // Class: Class Code (Assigned by USB Org)
    uint8_t  bDeviceClass;
    // Subclass: Subclass Code (Assigned by USB Org)
    uint8_t  bDeviceSubClass;
    // Protocol: Protocol Code (Assigned by USB Org)
    uint8_t  bDeviceProtocol;
    // Number: Maximum Packet Size for Zero Endpoint. Valid Sizes are 8, 16, 32, 64
    uint8_t  bMaxPacketSize;
    // ID: Vendor ID (Assigned by USB Org)
    uint16_t idVendor;
    // ID: Product ID (Assigned by Manufacturer)
    uint16_t idProduct;
    // BCD: Device Release Number. User value in same format as USB version above.
    uint16_t bcdDevice;
    // Index: Index of Manufacturer String Descriptor
    uint8_t  iManufacturer;
    // Index: Index of Product String Descriptor
    uint8_t  iProduct;
    // Index: Index of Serial Number String Descriptor
    uint8_t  iSerialNumber;
    // Integer: Number of Possible Configurations
    uint8_t  bNumConfigurations;
};


// This buffer is used to construct values for the descriptors when they are requested.
// Theoretically maximum size is presumably 255 since the first byte of the descriptor
// is its length.
constexpr uint16_t DescBufferSize = 128;
static uint8_t g_desc_buffer[DescBufferSize];


uint8_t* UARTDriverUSBBase::GetDeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    DeviceDescriptor desc =
    {
        .bLength            = 0x12,
        .bDescriptorType    = USB_DESC_TYPE_DEVICE,       // From usbd_def.h
        .bcdUSB             = 0x0200,
        // 0x02 = CDC. See https://www.usb.org/defined-class-codes.
        .bDeviceClass       = 0x02,
        // No idea what the 0x02 means here.
        .bDeviceSubClass    = 0x02,
        // No idea what the 0x00 means here.
        .bDeviceProtocol    = 0x00,
        .bMaxPacketSize     = USB_MAX_EP0_SIZE,           // From usbd_def.h
        .idVendor           = m_self->m_conf.VendorID,
        .idProduct          = m_self->m_conf.ProductID,
        .bcdDevice          = m_self->m_conf.DeviceID,
        .iManufacturer      = USBD_IDX_MFC_STR,           // From usbd_def.h
        .iProduct           = USBD_IDX_PRODUCT_STR,       // From usbd_def.h
        .iSerialNumber      = USBD_IDX_SERIAL_STR,        // From usbd_def.h
        .bNumConfigurations = USBD_MAX_NUM_CONFIGURATION, // From usbd_conf.h
    };

    // Use __attribute__((packed)) for the structure and just use memcpy?
    // Assumes that the uint16s will be little endian.
    // std::memcpy(&g_desc_buffer[0], &desc, sizeof(DeviceDescriptor));

    uint8_t* data = &g_desc_buffer[0];
    *data++ = desc.bLength;
    *data++ = desc.bDescriptorType;
    *data++ = lo_byte(desc.bcdUSB);                       // MFrom usbd_def.h
    *data++ = hi_byte(desc.bcdUSB);
    *data++ = desc.bDeviceClass;
    *data++ = desc.bDeviceSubClass;
    *data++ = desc.bDeviceProtocol;
    *data++ = desc.bMaxPacketSize;
    *data++ = lo_byte(desc.idVendor);
    *data++ = hi_byte(desc.idVendor);
    *data++ = lo_byte(desc.idProduct);
    *data++ = hi_byte(desc.idProduct);
    *data++ = lo_byte(desc.bcdDevice);
    *data++ = hi_byte(desc.bcdDevice);
    *data++ = desc.iManufacturer;
    *data++ = desc.iProduct;
    *data++ = desc.iSerialNumber;
    *data++ = desc.bNumConfigurations;

    *length = g_desc_buffer[0];
    return &g_desc_buffer[0];
}


uint8_t* UARTDriverUSBBase::GetLangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    uint8_t* data = &g_desc_buffer[2];
    *data++ = lo_byte(m_self->m_conf.LanguageID);
    *data++ = hi_byte(m_self->m_conf.LanguageID);
    // Can add more languages here if necessary

    *length = data - &g_desc_buffer[0];
    g_desc_buffer[0] = *length;
    g_desc_buffer[1] = USB_DESC_TYPE_STRING;  // From usbd_def.h
    return &g_desc_buffer[0];
}


static uint8_t* make_string_descriptor(const char* str, uint16_t* length)
{
    auto len = (str ? std::strlen(str) : 0) + 2;
    if (len > DescBufferSize)
    {
        *length = 0;
        return nullptr;
    }

    uint8_t* data = &g_desc_buffer[2];
    while (*str != 0)
    {
        *data++ = *str++;
        *data++ = 0;
    }

    *length   = data - &g_desc_buffer[0];
    g_desc_buffer[0] = *length;
    g_desc_buffer[1] = USB_DESC_TYPE_STRING;
    return &g_desc_buffer[0];
}


uint8_t* UARTDriverUSBBase::GetManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    return make_string_descriptor(m_self->m_conf.ManufacturerString, length);
}


uint8_t* UARTDriverUSBBase::GetProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    return make_string_descriptor(m_self->m_conf.ProductString, length);
}


uint8_t* UARTDriverUSBBase::GetSerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    uint8_t* data = &g_desc_buffer[2];

    uint32_t uid[3] = { HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2() };
    for (uint8_t i = 0; i < 3; ++i)
    {
        // Word separator added for clarity.
        if (i > 0)
        {
            *data++ = '-';
            *data++ = 0;
        }

        for (uint8_t j = 0; j < 8; ++j)
        {
            // High nybble first.
            uint8_t nybble = (uid[i] >> 28) & 0xF;
            uid[i]  = uid[i] << 4;
            *data++ = nybble + ((nybble < 0xA) ? '0' : ('A' - 10));
            *data++ = 0;
        }
    }

    *length   = data - &g_desc_buffer[0];
    g_desc_buffer[0] = *length;
    g_desc_buffer[1] = USB_DESC_TYPE_STRING;
    return &g_desc_buffer[0];
}


uint8_t* UARTDriverUSBBase::GetConfigurationStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    return make_string_descriptor(m_self->m_conf.ConfigurationString, length);
}


uint8_t* UARTDriverUSBBase::GetInterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t* length)
{
    return make_string_descriptor(m_self->m_conf.InterfaceString, length);
}


} // namespace eg {

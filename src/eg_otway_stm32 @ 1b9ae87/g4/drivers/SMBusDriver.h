/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "interfaces/ISMBusDriver.h"
#include "helpers/I2CHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "utilities/RingBuffer.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_i2c.h"
#include <cstdint>


namespace eg {


// This class is more or less identical to the I2C driver, which uses the same peripherals, 
// but configured differently and with a whole different bunch of HAL API functions and HAL
// callbacks.
//
// I took the view that it is simpler and cleaner to leave the I2C driver alone: it appears 
// that we use SMBus rarely, so let's avoid #ifdef or 'if constexpr ()' clutter in the driver 
// we use a lot. We can also directly tailor the API to SMBus specific features such as SendByte(), 
// SendWord() and so on, rather than writing and SMBus adapter around the driver instance.
//
// The ST code has a middleware SMBUS/PMBUS "stack" which is intimately tangled up with 
// the weakly defined callbacks for the HAL_SMBUS_xxx family of functions. There is no clear 
// separation between the driver and the stack. The stack is represented by the STACK_SMBUS_xxx 
// family of functions, some of which are themselves weakly defined. STACK_SMBUS_xxx implement
// a kind of state machine which uses the HAL_SMBUS_xxx callbacks and other functions to 
// receive SMBUS commands and notifications while in "listening mode". While simple in principle,
// the whole this is a horrible mess which tries to handle being a host, a master and a slave all
// in one code base. A lot of code is devoted to receiving PMBUS (power management?) commands, 
// which we don't care about.
// 
// Given that we only care about receiving alerts from the battery, I have ignored the whole stack 
// for now. We can poll frequently enough to understand the status in any case. Suggest adding the
// ability to receive host notifications (do alerts fall into this category?). We can write a more
// general purpose SMBus stack if/when the need arises.
//
// STACK_SMBUS_xxx basically work as follows in listen mode (basically we are behaving as an I2C slave):
// - HAL_SMBUS_AddrCallback() is called when address byte is received by the peripheral: 
//     - If we don't care about the address, issue a NAK.
//     - If we do care about the address, read 1 byte to get the command: HAL_SMBUS_Slave_Receive_IT().
//     - The actual implementation is a lot more involved, dealing with alert, host notify, etc.
// - HAL_SMBUS_SlaveRxCpltCallback() is called when the byte has been read:
//     - Look up the command code in a table to determine the length of the rest.
//     - Call HAL_SMBUS_Slave_Receive_IT() to receive the rest of the command bytes.
// - HAL_SMBUS_SlaveRxCpltCallback() is called when the remaining bytes have been received:
//     - Handle the command.
//     - Go back to listening for more commands, notifications and alerts.
// - In case of error, reset the bus and go back to listening. 
//
// From Zephyr docs:
//    SMBus peripheral devices can initiate communication with Controller with two methods:
//        1. Host Notify protocol: Peripheral device that supports the Host Notify protocol behaves 
//           as a Controller to perform the notification. It writes a three-bytes message to a special 
//           address “SMBus Host (0x08)” with own address and two bytes of relevant data.
//        2. SMBALERT# signal: Peripheral device uses special signal SMBALERT# to request attention from 
//           the Controller. The Controller needs to read one byte from the special “SMBus Alert Response 
//           Address (ARA) (0x0c)”. The peripheral device responds with a data byte containing its own 
//           address.
//
// The smart battery uses the host notify protocol for alerts. This means we will receive 7-bit address 0x08
// and then read three bytes to obtain the data from the battery. This should be simple enough. Need a way 
// to inject this command onto the bus to be able to test this.
class SMBusDriverBase : public ISMBusDriver
{
public:
    struct Config
    {
        I2c       i2c;      
        uint32_t  i2c_src;
        // It looks like a magic number but populates 
        // multiple fields of a single regiser, TIMINGR.
        // See I2CHelpers::make_i2c_timing();
        uint32_t  i2c_timing;
        // Own address for the purpose of receiving commands 
        // from other devices on the bus. This is the 7-bit 
        // address before shifting for direction bit.
        uint16_t  i2c_address;
        
        Port      sda_port;   
        Pin       sda_pin;
        uint8_t   sda_alt;  

        Port      scl_port;
        Pin       scl_pin;
        uint8_t   scl_alt;    
    }; 

public:
    SMBusDriverBase(const Config& conf, RingBuffer<Transfer>& queue);
    void queue_transfer(Transfer& trans) override;
    SignalProxy<Transfer> on_complete() override { return SignalProxy<Transfer>{m_on_complete}; }
    SignalProxy<Transfer> on_notify() override   { return SignalProxy<Transfer>{m_on_notify}; }
    SignalProxy<Transfer> on_error() override    { return SignalProxy<Transfer>{m_on_error}; }

private:
    // Called from the constructor.
    void gpio_init();
    void i2c_init();

    void start_transfer();
    void finish_transfer();

    void event_isr();
    void error_isr();

    void MasterTxCpltCallback();
    void MasterRxCpltCallback();
    void SlaveRxCpltCallback();
    void ListenCpltCallback();
    void ErrorCallback();
    void AddrCallback(uint8_t direction, uint16_t address_match_code);

    // HAL callbacks we might want to implement for this driver.
    static void MasterTxCpltCallback(SMBUS_HandleTypeDef* hsmbus);
    static void MasterRxCpltCallback(SMBUS_HandleTypeDef* hsmbus);
    //static void SlaveTxCpltCallback(SMBUS_HandleTypeDef* hsmbus);
    static void SlaveRxCpltCallback(SMBUS_HandleTypeDef* hsmbus);
    static void ListenCpltCallback(SMBUS_HandleTypeDef* hsmbus);
    static void ErrorCallback(SMBUS_HandleTypeDef* hsmbus);
    static void AddrCallback(SMBUS_HandleTypeDef* hsmbus, uint8_t direction, uint16_t address_match_code);
    //static void MspInitCallback(SMBUS_HandleTypeDef* hsmbus);
    //static void MspDeInitCallback(SMBUS_HandleTypeDef* hsmbus);

private:
    // Nasty trick to add a user value to the handle for a bit of context in callbacks.
    struct SMBusHandle : public SMBUS_HandleTypeDef
    {
        SMBusDriverBase* Self{};
    };

private:
    // Totally arbitrary depth for a queue of pending transfers.
    using Transfers = RingBuffer<Transfer>;

    const Config&     m_conf;
    SMBusHandle       m_hsmbus{};

    // Active transfer, if any.    
    Transfer          m_tx_trans{};     
    // Pending transfers, if any.
    Transfers&        m_tx_queue;       

    // True while processing active and pending transfers.
    bool              m_busy{false};

    // Used only to receive host notification protocol messages (for now).
    enum class RxState { Command, Data };
    RxState           m_rx_state{RxState::Command};
    Transfer          m_rx_trans{};     

    // Host/controller transfer has completed successfully. 
    Signal<Transfer>  m_on_complete;
    // Host has received a notification message.
    Signal<Transfer>  m_on_notify;
    // An error occurred on the bus. Sends the current TX transfer as context.
    Signal<Transfer>  m_on_error;

};


// This is a lightweight wrapper template which lets us choose the size of the 
// queue for pending transfers, or even have different sizes for different instances.
// AT the actual work is in the common base class, avoiding template bloat.
template <uint16_t QUEUE_SIZE>
class SMBusDriver : public SMBusDriverBase
{
public:
    SMBusDriver(const Config& conf)
    : SMBusDriverBase{conf, m_data}
    {        
    }

private:
    RingBufferArray<Transfer, QUEUE_SIZE> m_data; 
};


} // namespace eg {





/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "interfaces/ISPIDriver.h"
#include "drivers/DigitalOutput.h"
#include "helpers/SPIHelpers.h"
#include "helpers/DMAHelpers.h"
#include "helpers/GPIOHelpers.h"
#include "helpers/GlobalDefs.h"
#include "utilities/RingBuffer.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include <cstdint>

namespace eg
{

    // Wrapper for the DMA HAL API for SPI.
    // ******************
    // **** WARNING! ****
    // ******************
    // The regular DMA (DMA1 and DMA2) on the STM32H7 cannot access the ITCM and DTCM internal RAM.
    // STM32CubeMX uses these by default, so you will need to configure the linker to use another
    // area of RAM if you want to use these DMA.
    class SPIDriverDMABase : public ISPIDriver
    {
    public:
        enum class Prescaler
        {
            Div2 = SPI_BAUDRATEPRESCALER_2,
            Div4 = SPI_BAUDRATEPRESCALER_4,
            Div8 = SPI_BAUDRATEPRESCALER_8,
            Div16 = SPI_BAUDRATEPRESCALER_16,
            Div32 = SPI_BAUDRATEPRESCALER_32,
            Div64 = SPI_BAUDRATEPRESCALER_64,
            Div128 = SPI_BAUDRATEPRESCALER_128,
            Div256 = SPI_BAUDRATEPRESCALER_256,
        };

        // Defines the size of the SPI frame. Other sizes are available, add as required.
        enum class DataSize
        {
            Size8Bit = SPI_DATASIZE_8BIT,
            Size12Bit = SPI_DATASIZE_12BIT,
            Size16Bit = SPI_DATASIZE_16BIT,
            Size24Bit = SPI_DATASIZE_24BIT,
            Size32Bit = SPI_DATASIZE_32BIT,
        };

        enum class SPIMode
        {
            // Clock low in idle state, data sampled on rising edge and shifted out on falling edge
            Mode0,
            // Clock low in idle state, data sampled on falling edge and shifted out on rising edge
            Mode1,
            // Clock high in idle state, data sampled on falling edge and shifted out on rising edge
            Mode2,
            // Clock high in idle state, data sampled on rising edge and shifted out on falling edge
            Mode3
        };

        enum class MIDI
        {
            NoDelay = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE,
            Delay1 = SPI_MASTER_INTERDATA_IDLENESS_01CYCLE,
            Delay2 = SPI_MASTER_INTERDATA_IDLENESS_02CYCLE,
            Delay3 = SPI_MASTER_INTERDATA_IDLENESS_03CYCLE,
            Delay4 = SPI_MASTER_INTERDATA_IDLENESS_04CYCLE,
            Delay5 = SPI_MASTER_INTERDATA_IDLENESS_05CYCLE,
            Delay6 = SPI_MASTER_INTERDATA_IDLENESS_06CYCLE,
            Delay7 = SPI_MASTER_INTERDATA_IDLENESS_07CYCLE,
            Delay8 = SPI_MASTER_INTERDATA_IDLENESS_08CYCLE,
            Delay9 = SPI_MASTER_INTERDATA_IDLENESS_09CYCLE,
            Delay10 = SPI_MASTER_INTERDATA_IDLENESS_10CYCLE,
            Delay11 = SPI_MASTER_INTERDATA_IDLENESS_11CYCLE,
            Delay12 = SPI_MASTER_INTERDATA_IDLENESS_12CYCLE,
            Delay13 = SPI_MASTER_INTERDATA_IDLENESS_13CYCLE,
            Delay14 = SPI_MASTER_INTERDATA_IDLENESS_14CYCLE,
            Delay15 = SPI_MASTER_INTERDATA_IDLENESS_15CYCLE,
        };

        struct Config
        {
            Spi spi;
            uint32_t spi_clk_src;
            uint32_t spi_prio{GlobalsDefs::DefPreemptPrio};

            Prescaler prescaler{Prescaler::Div32};

            // If increasing the size from 8 bit then you need to be very careful with
            // alignment of memory for the values in the Transfer struct. Using the internal
            // buffer in the Transfer struct is unlikely to work the way you want. So for 16-bit
            // data you should store the tx/rx data in uint16_t arrays and pass the
            // pointers to those arrays in the Transfer struct, reinterpret_cast to uint8_t*. The
            // tx_len and rx_len parameters then refer to the number of 16 bit values that will
            // be transferred.
            DataSize data_size{DataSize::Size8Bit};

            SPIMode mode;

	        const DigitalOutput::Config *miso_config;

            Dma miso_dma;
            uint8_t miso_req;
            uint32_t miso_prio{GlobalsDefs::DefPreemptPrio};

	        const DigitalOutput::Config *mosi_config;
            Dma mosi_dma;
            uint8_t mosi_req;
            uint32_t mosi_prio{GlobalsDefs::DefPreemptPrio};

	        const DigitalOutput::Config *sck_config;

            // Chip select output only used if cs_hardware set to true.
            bool cs_hardware{false};
	        const DigitalOutput::Config *cs_config;
            // Only applies when cs_hardware set to true. If cs_interleave is set to true,
            // then it interleaves SPI data frames with nonactive pulses when midi is
            // set to MIDI::Delay2 or above. Used for fast readout of chips that need
            // a CS pulse between each readout (common with ADCs).
            bool cs_interleave{false};

            // Specifies the master inter-data idleness.
            MIDI midi{MIDI::NoDelay};
        };

    public:
        SPIDriverDMABase(const Config &conf, RingBuffer<Transfer> &queue);
        void queue_transfer(Transfer &trans) override;
        SignalProxy<Transfer> on_complete() override { return SignalProxy<Transfer>{m_on_complete}; }
        SignalProxy<Transfer> on_error() override { return SignalProxy<Transfer>{m_on_error}; }

    private:
        // Called from the constructor.
        void gpio_init();
        void spi_init();
        void mosi_dma_init();
        void miso_dma_init();

        void assert_cs();
        void deassert_cs();

        void start_transfer();
        void finish_transfer();

        void spi_isr();
        void mosi_dma_isr();
        void miso_dma_isr();

        void TxCpltCallback();
        void RxCpltCallback();
        void TxRxCpltCallback();
        void ErrorCallback();

        // HAL callbacks - just trampoline the calls to non-static members above.
        static void TxCpltCallback(SPI_HandleTypeDef *hspi);
        static void RxCpltCallback(SPI_HandleTypeDef *hspi);
        static void TxRxCpltCallback(SPI_HandleTypeDef *hspi);
        static void ErrorCallback(SPI_HandleTypeDef *hspi);

    private:
        // Nasty trick to add a user value to the handle for a bit of context in callbacks.
        struct SPIHandle : public SPI_HandleTypeDef
        {
            SPIDriverDMABase *Self{};
        };

        struct DMAHandle : public DMA_HandleTypeDef
        {
            SPIDriverDMABase *Self{};
        };

    private:
        // Totally arbitrary depth for a queue of pending transfers.
        using Transfers = RingBuffer<Transfer>;

        const Config &m_conf;
        SPIHandle m_hspi{};
        DMAHandle m_mosi_dma{};
        DMAHandle m_miso_dma{};

        // Active transfer, if any.
        Transfer m_trans{};
        // Pending transfers, if any.
        Transfers &m_queue;
        // True while processing active and pending transfers.
        bool m_busy{false};
        Signal<Transfer> m_on_complete;
        Signal<Transfer> m_on_error;
        // This is used to track which of the two phases/buffers
        // we last sent. Needed to prevent repetition in the TX callback.
        RxMode m_phase{};
    };

    template <uint16_t QUEUE_SIZE>
    class SPIDriverDMA : public SPIDriverDMABase
    {
    public:
        SPIDriverDMA(const Config &conf)
            : SPIDriverDMABase{conf, m_data}
        {
        }

    private:
        RingBufferArray<Transfer, QUEUE_SIZE> m_data;
    };

} // namespace eg {

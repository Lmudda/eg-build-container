/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_crc.h"

#include "CrcDriver.h"

namespace eg {

CrcDriver::CrcDriver(const Config& config)
{
    init(config);
}

void CrcDriver::init(const Config& config)
{
    // There is only one CRC unit in the CPU
    m_crc_instance.Instance = reinterpret_cast<CRC_TypeDef*>(CRC_BASE);

    m_crc_instance.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    m_crc_instance.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    m_crc_instance.Init.GeneratingPolynomial = config.polynomial;

    switch (config.crc_length)
    {
    case Length::e_7_bit:
        m_crc_instance.Init.CRCLength = CRC_POLYLENGTH_7B;
        break;

    case Length::e_8_bit:
        m_crc_instance.Init.CRCLength = CRC_POLYLENGTH_8B;
        break;

    case Length::e_16_bit:
        m_crc_instance.Init.CRCLength = CRC_POLYLENGTH_16B;
        break;

    case Length::e_32_bit:
        m_crc_instance.Init.CRCLength = CRC_POLYLENGTH_32B;
        break;

    default:
        m_crc_instance.Init.CRCLength = CRC_POLYLENGTH_32B;
        break;
    }

    m_crc_instance.Init.InitValue = config.initial_value;

    switch (config.input_inversion)
    {
    case InputInversion::e_none:
        m_crc_instance.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
        break;

    case InputInversion::e_8_bit_inversion:
        m_crc_instance.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
        break;

    case InputInversion::e_16_bit_inversion:
        m_crc_instance.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_HALFWORD;
        break;

    case InputInversion::e_32_bit_inversion:
        m_crc_instance.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_WORD;
        break;

    default:
        m_crc_instance.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
        break;
    }

    switch (config.output_inversion)
    {
    case OutputInversion::e_none:
        m_crc_instance.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
        break;

    case OutputInversion::e_32_bit_inversion:
        m_crc_instance.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
        break;

    default:
        m_crc_instance.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
        break;
    }

    m_crc_instance.Lock = HAL_UNLOCKED;
    m_crc_instance.State = HAL_CRC_STATE_RESET;
    
    switch (config.input_data_format)
    {
    case InputDataFormat::e_8_bit:
        m_crc_instance.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
        break;

    case InputDataFormat::e_16_bit:
        m_crc_instance.InputDataFormat = CRC_INPUTDATA_FORMAT_HALFWORDS;
        break;

    case InputDataFormat::e_32_bit:
        m_crc_instance.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
        break;

    default:
        m_crc_instance.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
        break;
    }
      
    HAL_CRC_Init(&m_crc_instance);
}


uint32_t CrcDriver::required_length(uint32_t length)
{
    uint32_t multiplier = 1u;

    switch (m_crc_instance.InputDataFormat)
    {
    case CRC_INPUTDATA_FORMAT_BYTES:
        multiplier = 4u;
        break;

    case CRC_INPUTDATA_FORMAT_HALFWORDS:
        multiplier = 2u;
        break;

    case CRC_INPUTDATA_FORMAT_WORDS:
        multiplier = 1u;
        break;

    default:
        // Nothig to do
        break;
    }

    return length * multiplier;
}


uint32_t CrcDriver::accumulate(uint32_t const * buffer, uint32_t length)
{
    return HAL_CRC_Accumulate(&m_crc_instance, const_cast<uint32_t*>(buffer), required_length(length));
}

uint32_t CrcDriver::calculate(uint32_t const * buffer, uint32_t length)
{
    return HAL_CRC_Calculate(&m_crc_instance, const_cast<uint32_t*>(buffer), required_length(length));
}

CrcDriver::State CrcDriver::get_state()
{
    State state = State::eError;

    switch (HAL_CRC_GetState(&m_crc_instance))
    {
    case HAL_CRC_STATE_RESET:
        state = State::eReset;
        break;

    case HAL_CRC_STATE_READY:
        state = State::eIdle;
        break;

    case HAL_CRC_STATE_BUSY:
        state = State::eBusy;
        break;

    case HAL_CRC_STATE_TIMEOUT:
        state = State::eTimeout;
        break;

    case HAL_CRC_STATE_ERROR:
        state = State::eError;
        break;

    default:
        state = State::eError;
        break;
    }

    return state;
}


} // namespace eg {

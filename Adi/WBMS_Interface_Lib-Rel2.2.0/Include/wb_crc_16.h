/******************************************************************************
 * @file    wb_crc_16.h
 *
 * @brielf  CRC16 implementation header
 *
 * @details The CRC Logic used in this implementation was provided by
 *          Bastian Molkenthin (www.bastian-molkenthin.de)
 *
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_CRC_16_H
#define WB_CRC_16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * The polynomial 0xAC9A has Hamming distance (HD) = 5 up to
 * a dataword length of 241 bits (30 bytes), excluding the 16-bit CRC
 * code itself.
 *
 * The chosen polynomial in Koopman's notation is k = 0xAC9Au. The
 * actual polynomial is ((k << 1) + 1) = 0x15935. This needs to be
 * converted to "normal" notation before generating the table which
 * means dropping the most-significant bit,
 * resulting in 0x5935.  Lastly, since we are using
 * WB_CRC_CFG_LSB_FIRST=1, this value must be bit-reversed. In this
 * particular case, since the polynomial 0x15935 is symmetrical,
 * reversing the normal notation gives a value identical to the
 * original Koopman's notation, but this is not true in general.
 *****************************************************************************/

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

/**
 * @brief        Computes CRC16 for the submitted buffer
 *
 * @param[in]   pData           Pointer to input data buffer on which CRC is calculated
 * @param[in]   iLength         Size of input data in bytes
 * @param[in]   iSeedValue      Sets the initial seed value for the CRC operation
 * 
 * @details     CRC16 computation uses a pre-calculated look-up table.
 *              The CRC calculation takes places on bytes level.
 *
 * @return      CRC16 Result
 */
uint16_t wb_crc_ComputeCRC16 (uint8_t const * const pData,
                              uint32_t iLength,
                              uint16_t iSeedValue);

#ifdef __cplusplus
}
#endif
#endif /* WB_CRC_16_H */

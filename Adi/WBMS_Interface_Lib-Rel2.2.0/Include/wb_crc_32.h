/******************************************************************************
 * @file    wb_crc_32.h
 *
 * @brielf  CRC32 implementation header
 *
 * @details The CRC Logic used in this implementation was provided by
 *          Bastian Molkenthin (www.bastian-molkenthin.de)
 *
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_CRC_32_H
#define WB_CRC_32_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * The chosen polynomial in Koopman's notation is k = 0x9960034C. It
 * has Hamming distance (HD) = 6 up to dataword length of 32738 bits
 * (4092 bytes).  The actual polynomial is ((k << 1) + 1) =
 * 0x132C00699. This needs to be converted to "normal" notation before
 * generating the table which means dropping the
 * most-significant bit, resulting in 0x32C00699.  Lastly, since we
 * are using WB_CRC_CFG_LSB_FIRST=1, this value must be
 * bit-reversed. In this particular case, since the polynomial
 * 0x132C00699 is symmetrical, reversing the normal notation gives a
 * value identical to the original Koopman's notation, but this is not
 * true in general.
 *****************************************************************************/

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

/**
 * @brief       Computes CRC32 for the submitted buffer
 *
 * @param[in]   pData           Pointer to input data buffer on which CRC is calculated
 * @param[in]   iLength         Size of input data in bytes
 * @param[in]   iSeedValue      Sets the initial seed value for the CRC operation
 * 
 * @details     CRC32 computation uses a pre-calculated look-up table.
 *              The CRC calculation takes places on bytes level.
 *
 * @return      CRC32 Result
 */
uint32_t wb_crc_ComputeCRC32 (uint8_t const * const pData,
                              uint32_t iLength,
                              uint32_t iSeedValue);

#ifdef __cplusplus
}
#endif
#endif /* WB_CRC_32_H */

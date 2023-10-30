/******************************************************************************
 * @file     adi_wil_file.h
 *
 * @brief    WBMS file data definitions
 *
 * @details  Contains file type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_FILE__H
#define ADI_WIL_FILE__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   GetFile API structure definition
 */
struct adi_wil_file_t {
    uint16_t iRemainingBytes;         /*!< Remaining bytes in file after this block */
    uint8_t iByteCount;               /*!< Number of bytes to follow this header */
    uint16_t iOffset;                 /*!< Current offset in file of this block */
    uint8_t *pData;                   /*!< Pointer to block of file data*/
};

#endif //ADI_WIL_FILE__H

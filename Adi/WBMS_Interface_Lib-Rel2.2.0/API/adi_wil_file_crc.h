/******************************************************************************
 * @file     adi_wil_file_crc.h
 *
 * @brief    WBMS container info definitions
 *
 * @details  Contains container information type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/



#ifndef ADI_WIL_FILE_CRC__H
#define ADI_WIL_FILE_CRC__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Return type of adi_wil_GetFileCRCAPI 
 */
struct adi_wil_file_crc_list_t {
    adi_wil_device_t eFileExists;                                    /*!< Bitmap representing whether the requested file exists on a manager/device. Bits 63:62 indicate file exists on manager 1 and 0 respectively. Bits 61-0 indicate file exists on Nodes 61:0 respectively. The corresponding CRC is in iCRC array if file exists*/
    uint32_t iCRC [ADI_WIL_NUM_NW_MANAGERS + ADI_WIL_MAX_NODES];     /*!< Retrieved CRC value iCRC[63:62] is the CRC of the file on managers 1 and 0, respectively. iCRC[61:0] is the CRC of the file on nodes[61:0] respectively. */
};

#endif //ADI_WIL_FILE_CRC__H

/******************************************************************************
 * @file     adi_wil_load_file.h
 *
 * @brief    WBMS loadfile return definitions
 *
 * @details  Contains loadfile return data type definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_LOAD_FILE__H
#define ADI_WIL_LOAD_FILE__H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   LoadFile return Buffer
 */
struct adi_wil_loadfile_status_t {
    uint32_t iOffset;                       /*!< Current offset in file of this block */
    adi_wil_device_t iDeviceLoadSuccess;    /*!< If a device successfully completed the file download, 
                                                 the corresponding bit is to be set to 1, 0 otherwise. 
	                                               Manager bits are set in top 2 bits of bitmask. */
};

#endif //ADI_WIL_LOAD_FILE__H

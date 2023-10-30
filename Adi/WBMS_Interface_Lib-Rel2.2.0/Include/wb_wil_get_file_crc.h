/******************************************************************************
 * @file    wb_wil_get_file_crc.h
 *
 * @brief   Erase the black-box log on a manager or a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_FILE_CRC_H
#define WB_WIL_GET_FILE_CRC_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetFileCRCAPI (adi_wil_pack_internals_t * const pInternals,
                                    uint64_t iDeviceId,
                                    adi_wil_file_type_t eFileType);

void wb_wil_HandleGetFileCRCResponse (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      wbms_cmd_resp_get_file_crc_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GET_FILE_CRC_H

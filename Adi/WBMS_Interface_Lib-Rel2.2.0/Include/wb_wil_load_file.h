/******************************************************************************
 * @file    wb_wil_load_file.h
 *
 * @brief   State machine to send file to device (node/manager)
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_LOAD_FILE_H
#define WB_WIL_LOAD_FILE_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_LoadFileAPI (adi_wil_pack_internals_t * const pInternals,
                                  adi_wil_device_t eDeviceId,
                                  adi_wil_file_type_t eFileType,
                                  uint8_t const * const pData);

void wb_wil_HandleHandshakeResponse (adi_wil_pack_internals_t * const pInternals,
                                    uint64_t iDeviceId,
                                    wbms_cmd_resp_otap_handshake_t const * const pResponse);

void wb_wil_HandleStatusResponse (adi_wil_pack_internals_t * const pInternals,
                                  uint64_t iDeviceId,
                                  wbms_cmd_resp_otap_status_t const * const pResponse);

void wb_wil_HandleCommitResponse (adi_wil_pack_internals_t * const pInternals,
                                  uint64_t iDeviceId,
                                  wbms_cmd_resp_generic_t const * const pResponse);

void wb_wil_HandleDataResponse (adi_wil_pack_internals_t * const pInternals,
                                wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_LOAD_FILE_H

/******************************************************************************
 * @file    wb_wil_capture_network_data.h
 *
 * @brief   Capture network metadata from BMS/environmental measurement
 *          packets
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_CAPTURE_NETWORK_DATA_H
#define WB_WIL_CAPTURE_NETWORK_DATA_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"
#include "wb_ntf_packet_received.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_EnableNetworkDataCaptureAPI (adi_wil_pack_internals_t * const pInternals,
                                               adi_wil_network_data_t * pDataBuffer,
                                               uint16_t iDataBufferCount,
                                               bool bEnable);

void wb_wil_NetworkDataNotify (adi_wil_pack_internals_t * const pInternals,
                               uint64_t iDeviceId,
                               wbms_notif_packet_received_t const * const pElement);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_CAPTURE_NETWORK_DATA_H

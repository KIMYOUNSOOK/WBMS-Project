/******************************************************************************
 * @file    wb_wil_query_device.h
 *
 * @brief   Get the configuration data from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_QUERY_DEVICE_H
#define WB_WIL_QUERY_DEVICE_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_QueryDeviceAPI (adi_wil_port_t * const pPort);

void wb_wil_HandleQueryDeviceResponse (adi_wil_port_t * const pPort,
                                       wbms_cmd_resp_query_device_t const * const pResponse);

void wb_wil_QueryDeviceCheckTimeout (adi_wil_port_t * const pPort);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_QUERY_DEVICE_H

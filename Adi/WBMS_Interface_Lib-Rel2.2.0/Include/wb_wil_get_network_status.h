/******************************************************************************
 * @file    wb_wil_get_network_status.h
 *
 * @brief   Returns the network status of all nodes in the pack
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_NETWORK_STATUS_H
#define WB_WIL_GET_NETWORK_STATUS_H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetNetworkStatusAPI (adi_wil_pack_internals_t const * const pInternals,
                                          adi_wil_network_status_t * const pStatus);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_GET_NETWORK_STATUS_H

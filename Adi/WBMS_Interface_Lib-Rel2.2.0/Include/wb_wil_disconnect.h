/******************************************************************************
 * @file    wb_wil_disconnect.h
 *
 * @brief   Disconnect from network managers in a pack
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_DISCONNECT_H
#define WB_WIL_DISCONNECT_H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_DisconnectAPI (adi_wil_pack_internals_t * const pInternals);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_DISCONNECT_H

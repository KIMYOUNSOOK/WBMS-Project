/******************************************************************************
 * @file    wb_wil_setgpio.h
 *
 * @brief   Get the value of a supported GPIO
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GETGPIO_H
#define WB_WIL_GETGPIO_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_GetGPIOAPI (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 adi_wil_gpio_id_t eGPIOId);

void wb_wil_HandleGetGPIOResponse (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   wbms_cmd_resp_get_gpio_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_GETGPIO_H

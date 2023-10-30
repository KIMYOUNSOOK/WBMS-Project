/******************************************************************************
 * @file    wb_wil_setgpio.h
 *
 * @brief   Set a supported GPIO pin on a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_SETGPIO_H
#define WB_WIL_SETGPIO_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_SetGPIOAPI (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 adi_wil_gpio_id_t eGPIOId,
                                 adi_wil_gpio_value_t eGPIOValue);

void wb_wil_HandleSetGPIOResponse (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   wbms_cmd_resp_generic_t const * const pResponse);

adi_wil_err_t wb_wil_SetGPIOCheckPin (adi_wil_gpio_id_t gpio_pin);

#ifdef __cplusplus
}
#endif
#endif  // WB_WIL_SETGPIO_H

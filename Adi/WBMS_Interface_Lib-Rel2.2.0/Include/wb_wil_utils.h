/******************************************************************************
 * @file    wb_wil_utils.h
 *
 * @brief   Functions to convert values to enumerated types and similar
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_UTILS_H
#define WB_WIL_UTILS_H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


adi_wil_err_t wb_wil_SensorIdToUint(adi_wil_sensor_id_t eSensorId, uint8_t * pSensorId);

adi_wil_err_t wb_wil_GetGpioValueFromUint(uint8_t iGpioValue, adi_wil_gpio_value_t * const pGPIOValue);

uint8_t wb_wil_GPIOtoUint(adi_wil_gpio_id_t GPIOPin);

adi_wil_err_t wb_wil_ContextualIDToUint(adi_wil_contextual_id_t eContextualId, uint8_t * pContextualId);

adi_wil_err_t wb_wil_GetErrFromUint(uint8_t iRC);

uint8_t wb_wil_GetUintFromCustomerIdentifier(adi_wil_customer_identifier_t eCustomerIdentifier);

uint8_t wb_wil_GetUintFromWILMode(adi_wil_mode_t eMode);

adi_wil_err_t wb_wil_GetUintFromFileType (adi_wil_file_type_t eFileType, uint8_t * pFileType);

void wb_wil_IncrementWithRollover8(uint8_t * pValue);

void wb_wil_IncrementWithRollover16(uint16_t * pValue);

void wb_wil_IncrementWithRollover32(uint32_t * pValue);

adi_wil_device_t wb_wil_GetExternalDeviceId (uint8_t iDeviceId);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_UTILS_H

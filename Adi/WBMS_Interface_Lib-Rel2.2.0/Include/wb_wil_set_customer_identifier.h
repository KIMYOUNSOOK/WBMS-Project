/*******************************************************************************
 * @file    wb_wil_set_customer_identifier.h
 *
 * @brief   Highest level (WIL) State Machine for SetCustomerIdentifier API
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_WIL_SET_CUSTOMER_IDENTIFIER_H
#define WB_WIL_SET_CUSTOMER_IDENTIFIER_H

#include "adi_wil_types.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetCustomerIdentifierDataAPI (adi_wil_pack_internals_t * const pInternals,
                                                   adi_wil_customer_identifier_t eCustomerIdentifier,
                                                   uint8_t const * const pData,
                                                   uint8_t iLength);

void wb_wil_HandleSetCustomerIdentifierResponse (adi_wil_pack_internals_t * const pInternals,
                                                 uint64_t iDeviceId,
                                                 wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_SET_CUSTOMER_IDENTIFIER_H

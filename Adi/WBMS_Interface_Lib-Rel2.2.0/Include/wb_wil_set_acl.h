/******************************************************************************
 * @file    wb_wil_set_acl.h
 *
 * @brief   Set the ACL list from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_SET_ACL_H
#define WB_WIL_SET_ACL_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/
adi_wil_err_t wb_wil_SetACLAPI (adi_wil_pack_internals_t * const pInternals,
                                uint8_t const * const pData,
                                uint8_t iCount);

void wb_wil_HandleClearACLResponse (adi_wil_pack_internals_t * const pInternals,
                                    wbms_cmd_resp_generic_t const * const pResponse);

void wb_wil_HandleSetACLResponse (adi_wil_pack_internals_t * const pInternals,
                                  wbms_cmd_resp_generic_t const * const pResponse);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_SET_ACL_H

/******************************************************************************
 * @file    wb_wil_get_acl.h
 *
 * @brief   Get the ACL list from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_GET_ACL_H
#define WB_WIL_GET_ACL_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetACLAPI (adi_wil_pack_internals_t * const pInternals);

void wb_wil_HandleGetACLResponse (adi_wil_pack_internals_t * const pInternals,
                                  wbms_cmd_resp_get_acl_t const * const pResponse,
                                  uint8_t const * const pData);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_GET_ACL_H

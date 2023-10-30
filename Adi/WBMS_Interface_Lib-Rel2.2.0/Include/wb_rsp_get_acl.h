/******************************************************************************
 * @file     wb_rsp_get_acl.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_GET_ACL_H
#define WB_RSP_GET_ACL_H

#include <stdint.h>
/**
 * @brief   wbms_cmd_resp_get_acl_t
 */
struct wbms_cmd_resp_get_acl_t {
    uint16_t iToken;               /* Token to match response with request */
    uint8_t iCount;                /* Number of 8-byte MACs in the ACL that follow this header */
    uint8_t rc;
};

#endif //WB_RSP_GET_ACL_H

/******************************************************************************
 * @file     wb_req_get_acl.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GET_ACL_H
#define WB_REQ_GET_ACL_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_get_acl_t
 */
struct wbms_cmd_req_get_acl_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iIndex;                /* Index in the ACL to retrieve - up to 8 entries will be returned */
};

#endif //WB_REQ_GET_ACL_H

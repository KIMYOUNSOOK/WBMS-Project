/******************************************************************************
 * @file     wb_req_set_acl.h
 *
 * @brief
 *
 * @details
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_CMD_SET_ACL_REQ_H
#define WB_CMD_SET_ACL_REQ_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_acl_t
 */
struct wbms_cmd_req_set_acl_t {
    uint16_t iToken;
    uint8_t iCount;
};

#endif //WB_CMD_SET_ACL_REQ_H

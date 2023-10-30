/******************************************************************************
 * @file     wb_rsp_set_mode.h
 *
 * Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_SET_MODE_H
#define WB_RSP_SET_MODE_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_set_mode_t
 */
struct wbms_cmd_resp_set_mode_t {
    uint16_t iToken;                           /* Token to match request with response */
    uint8_t iBitMapSize;                       /* Number of valid bits in the nodeBitMap array */
    uint8_t nodeBitMap[WBMS_NODE_BITMAP_SIZE]; /* Contains bitMap for nodes, bitMap is in the order of ACL. 1-node successfully changed the mode, 0-node failed to change the mode or response not received from the node*/
    uint8_t rc;                                /* RC indicating if the above values are valid */
};

#endif //WB_RSP_SET_MODE_H

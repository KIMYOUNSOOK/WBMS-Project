/******************************************************************************
 * @file     wb_req_inventory_transition.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_INVENTORY_TRANSITION_H
#define WB_REQ_INVENTORY_TRANSITION_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_inventory_transition_t
 */
struct wbms_cmd_req_inventory_transition_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t bExitFlag;             /* Flag to indicate if the device is to enter or exit the inventory state */
                                   /* 0 = Enter inventory state */
                                   /* 1 = Exit inventory state*/
    uint64_t iTimeInSeconds;       /* Current UNIX time in seconds */
};

#endif //WB_REQ_INVENTORY_TRANSITION_H

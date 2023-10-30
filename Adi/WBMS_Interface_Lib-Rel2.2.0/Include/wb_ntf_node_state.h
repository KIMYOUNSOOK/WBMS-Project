/******************************************************************************
 * @file     wb_ntf_node_state.h
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_NODE_STATE_H
#define WB_NOTIF_NODE_STATE_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_notif_node_state_t
 */
struct wbms_notif_node_state_t {
	uint16_t iNotifId;						/* Notifiction ID to map the acknowledgement to Notification  */
    uint8_t iDeviceID;                      /* Device ID of the node */
    uint8_t	iState;                         /* Boolean 0 or 1 indicating whether the device has connected or disconnected  */
};

#endif //WB_NOTIF_NODE_STATE_H

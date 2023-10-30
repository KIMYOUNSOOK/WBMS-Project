/******************************************************************************
 * @file     wbms_notif_node_mode_mismatch_t
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_NODE_MODE_MISMATCH_H
#define WB_NOTIF_NODE_MODE_MISMATCH_H

#include <stdint.h>

/**
 * @brief   wbms_notif_node_mode_mismatch_t
 */
struct wbms_notif_node_mode_mismatch_t {
	uint16_t iNotifId;						/* Notifiction ID to map the acknowledgement to Notification  */
    uint8_t iDeviceID;                      /* Device ID of the node that caused the mode mismatch */
};

#endif //WB_NOTIF_NODE_MODE_MISMATCH_H

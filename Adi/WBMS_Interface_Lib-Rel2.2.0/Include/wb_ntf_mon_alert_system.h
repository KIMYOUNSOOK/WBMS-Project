/******************************************************************************
 * @file     wb_ntf_mon_alert_system.h
 *
 * Copyright (c) 2019-2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_MON_ALERT_DEVICE_H
#define WB_NOTIF_MON_ALERT_DEVICE_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_notif_mon_alert_system_t
 */
struct wbms_notif_mon_alert_system_t {
    uint8_t iEventedManagers;                           /* Bitmap representing managers for which a monitor mode event was received or detected */
    uint8_t iEventedNodes[WBMS_NODE_BITMAP_SIZE];       /* Bitmap represeting nodes for which a monitor mode event was received or detected */
};

#endif //WB_NOTIF_MON_ALERT_SYSTEM_H

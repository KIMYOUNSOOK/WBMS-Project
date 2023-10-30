/******************************************************************************
 * @file     wb_ntf_mon_alert_device.h
 *
 * Copyright (c) 2019-2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_MON_ALERT_SYSTEM_H
#define WB_NOTIF_MON_ALERT_SYSTEM_H

#include <stdint.h>
#include "wbms_cmd_defs.h"

/**
 * @brief   wbms_notif_mon_alert_device_t
 */
struct wbms_notif_mon_alert_device_t {
    uint16_t iAlertTypes;                         /* Bitmap represeting alert types detected on this device */
    uint8_t iChannels[WBMS_FAULT_CHANNELS_SIZE];  /* Bitmap represeting channels on which plausibility faults have been detected */
};

#endif //WB_NOTIF_MON_ALERT_DEVICE_H

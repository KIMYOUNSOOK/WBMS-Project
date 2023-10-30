/******************************************************************************
 * @file     wb_ntf_sensor_data.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_SENSOR_DATA_H
#define WB_NOTIF_SENSOR_DATA_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_notif_sensor_data_t
 */
struct wbms_notif_sensor_data_t {
    uint8_t	iTotalPackets;
    uint8_t	iPacketIndex;
    uint8_t iLength;
};

#endif //WB_NOTIF_SENSOR_DATA_H

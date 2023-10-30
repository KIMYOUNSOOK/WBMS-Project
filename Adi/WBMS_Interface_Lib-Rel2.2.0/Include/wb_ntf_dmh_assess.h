/******************************************************************************
 * @file     wb_ntf_dmh_assess.h
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_DMH_ASSESS_H
#define WB_NOTIF_DMH_ASSESS_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
* @brief   wbms_notif_dmh_assess_t
*/
struct wbms_notif_dmh_assess_t {
    uint16_t iNotifId;                          /* Notifiction ID to map the acknowledgement to notification */
    int8_t iRssiDeltas [WBMS_MAX_NODES];        /* Expected change in RSSI for all assessed nodes expressed in dBs */
    int8_t iSignalFloorImprovement;             /* Interference resilience improvement (worst path) expressed in dBs */
    uint8_t rc;                                 /* Result of assessment process */
};

#endif //WB_NOTIF_DMH_ASSESS_H

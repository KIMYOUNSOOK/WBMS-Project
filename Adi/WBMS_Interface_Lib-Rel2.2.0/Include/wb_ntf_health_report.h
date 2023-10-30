/******************************************************************************
 * @file     wb_ntf_health_report.h
 *
 * Copyright (c) 2019-2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_HEALTH_REPORT_H
#define WB_NOTIF_HEALTH_REPORT_H

#include <stdint.h>

/**
 * @brief   wbms_notif_health_report_t
 */
struct wbms_notif_health_report_t {
	uint16_t iNotifId;                           /* Notifiction ID to map the acknowledgement to Notification  */
    uint8_t  iDeviceId;                          /* Index in device of ACL which generated the report. */
    uint16_t iSequenceNumber;                    /* Sequence Number of Health Report */
    uint64_t iASN;                               /* Timestamp of packet generation. */
    uint8_t  iLength;                            /* number of valid bytes in the buffer Data */
};

#endif //WB_NOTIF_HEALTH_REPORT_H

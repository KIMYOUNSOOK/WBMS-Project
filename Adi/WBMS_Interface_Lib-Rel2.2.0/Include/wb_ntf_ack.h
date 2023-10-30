/******************************************************************************
 * @file     wb_ntf_ack.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_ACK_H
#define WB_NOTIF_ACK_H

#include <stdint.h>

/**
* @brief   wbms_notif_ack_t
*/
struct wbms_notif_ack_t {
    uint16_t iNotifId;                          /* Notifiction ID to map the acknowledgement to Notification  */
};

#endif //WB_NOTIF_ACK_H

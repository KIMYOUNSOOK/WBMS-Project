/******************************************************************************
 * @file     wb_ntf_dmh_apply.h
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_DMH_APPLY_H
#define WB_NOTIF_DMH_APPLY_H

#include <stdint.h>

/**
* @brief   wbms_notif_dmh_apply_t
*/
struct wbms_notif_dmh_apply_t {
    uint16_t iNotifId;                          /* Notifiction ID to map the acknowledgement to Notification  */
    uint8_t rc;                                 /* Result of application process */
};

#endif //WB_NOTIF_DMH_APPLY_H

/******************************************************************************
 * @file     wb_ntf_security_error.h
 *
 * Copyright (c) 2019-2020 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_SECURITY_ERROR_H
#define WB_NOTIF_SECURITY_ERROR_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_notif_security_error_t
 */
struct wbms_notif_security_error_t {
    uint16_t iNotifId;
    uint8_t iNotificationType;
    uint8_t MAC[WBMS_MAC_ADDR_LEN];
};

#endif //WB_NOTIF_SECURITY_ERROR_H

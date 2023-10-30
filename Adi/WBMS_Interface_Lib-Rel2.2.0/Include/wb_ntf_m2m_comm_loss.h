/******************************************************************************
 * @file     wb_ntf_m2m_comm_loss.h
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_M2M_COMM_LOSS_H
#define WB_NOTIF_M2M_COMM_LOSS_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_notif_m2m_comm_loss_t
 */
struct wbms_notif_m2m_comm_loss_t {
	uint16_t iNotifId;						/* Notifiction ID to map the acknowledgement to Notification  */
};

#endif //WB_NOTIF_M2M_COMM_LOSS_H

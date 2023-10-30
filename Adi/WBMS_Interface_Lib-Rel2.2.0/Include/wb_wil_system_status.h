/******************************************************************************
* @file    wb_wil_system_status.h
*
* @brief
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_SYSTEM_STATUS_H
#define WB_WIL_SYSTEM_STATUS_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public functions
*****************************************************************************/

/**
* @brief Handle System Status Notification
*
* @param pPort - Manager Id
* @param pPack - pointer to pack instance struct
* @param pNotif - pointer to status Notification
*
* @return none
*
*/
void wb_wil_HandleSystemStatus(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_notif_system_status_t const * const pNotif);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_SYSTEM_STATUS_H

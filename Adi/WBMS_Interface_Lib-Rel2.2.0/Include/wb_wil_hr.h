/******************************************************************************
* @file    wb_wil_hr.h
*
* @brief   Handled the Node State change in WBMS Network.
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_HEALTH_REPORT_H
#define WB_WIL_HEALTH_REPORT_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public functions
*****************************************************************************/

/**
* @brief Handle Health Report Notification
*
* @param pPort - Manager Id
* @param pPack - pointer to pack instance struct
* @param pNotif - pointer to State Notification
*
* @return none
*
*/
void wb_wil_HandleHealthReport (adi_wil_pack_internals_t const * const pInternals,
                                uint64_t iDeviceId,
                                wbms_notif_health_report_t const * const pNotif,
                                uint8_t const * const pData);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_HEALTH_REPORT_H

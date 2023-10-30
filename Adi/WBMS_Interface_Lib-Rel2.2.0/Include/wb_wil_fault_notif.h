/******************************************************************************
* @file    wb_wil_fault_notif.h
*
* @brief   Handle a security error notification from a device
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_FAULT_NOTIF_H
#define WB_WIL_FAULT_NOTIF_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public functions
*****************************************************************************/

/**
* @brief Handle system fault notification error
*
* @param pPack      Pointer to pack instance struct
* @param pNotif     Pointer to notification data structure
*
*/
void wb_wil_HandleFaultSummaryNotif(adi_wil_pack_internals_t const * const pInternals, wbms_notif_mon_alert_system_t const * const pNotif);

/**
* @brief Handle device fault notification error
*
* @param pPack      Pointer to pack instance struct
* @param eDeviceId  Device ID of the manager that generated this error notification
* @param pNotif     Pointer to notification data structure
*
*/
void wb_wil_HandleFaultReportNotif(adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wbms_notif_mon_alert_device_t const * const pNotif);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_FAULT_NOTIF_H

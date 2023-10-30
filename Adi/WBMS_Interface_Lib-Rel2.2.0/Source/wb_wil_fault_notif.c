/******************************************************************************
* @file    wb_wil_fault_notif.c
*
* @brief   Handle fault notification.
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*****************************************************************************/

#include "wb_wil_fault_notif.h"
#include "wbms_cmd_mgr_defs.h"
#include "adi_wil_pack_internals.h"
#include "wb_ntf_mon_alert_device.h"
#include "wb_ntf_mon_alert_system.h"
#include "adi_wil_faults.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include <string.h>

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

void wb_wil_HandleFaultSummaryNotif (adi_wil_pack_internals_t const * const pInternals, wbms_notif_mon_alert_system_t const * const pNotif)
{
    adi_wil_device_t iFaultDevices;

    iFaultDevices = 0ULL;

    /* Set the node fault bits */
    for (uint8_t i = 0; i < WBMS_NODE_BITMAP_SIZE; i++)
    {
        iFaultDevices |= ((uint64_t) pNotif->iEventedNodes[i]) << (i * 8u);
    }

    /* Clear the top 2 manager bits */
    iFaultDevices &= ADI_WIL_DEV_ALL_NODES;

    /* Set the manager fault bits */
    iFaultDevices |= ((adi_wil_device_t)pNotif->iEventedManagers) << 62u;

    wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_FAULT_SOURCES, &iFaultDevices);
}

void wb_wil_HandleFaultReportNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wbms_notif_mon_alert_device_t const * const pNotif)
{
    adi_wil_fault_report_t FaultReport;

    /* Initialize FaultReport structure */
    (void) memset (&FaultReport, 0, sizeof (FaultReport));

    FaultReport.eDeviceId = iDeviceId;
    FaultReport.iAlertTypes = pNotif->iAlertTypes;
    (void) memcpy (&FaultReport.iChannels [0], &pNotif->iChannels [0], sizeof (FaultReport.iChannels));

    /* Generate an event to the host application with the fault report data */
    wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_FAULT_REPORT, &FaultReport);
}

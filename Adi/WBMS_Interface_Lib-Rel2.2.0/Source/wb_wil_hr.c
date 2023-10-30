/******************************************************************************
 * @file    wb_wil_health_report.c
 *
 * @brief   Handle Health Report Notification.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_hr.h"
#include "adi_wil_health_report.h"
#include "adi_wil_pack_internals.h"
#include "wb_ntf_health_report.h"
#include "wb_wil_utils.h"
#include "wb_wil_ack.h"
#include "wb_wil_ui.h"
#include <string.h>

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static void wb_wil_AcknowledgeHealthReport (adi_wil_pack_internals_t const * const pInternals,
                                            uint64_t iDeviceId,
                                            uint16_t iNotifId);

/******************************************************************************
 * Public function definitions 
 *****************************************************************************/

void wb_wil_HandleHealthReport (adi_wil_pack_internals_t const * const pInternals,
                                uint64_t iDeviceId,
                                wbms_notif_health_report_t const * const pNotif,
                                uint8_t const * const pData)
{
    /* Storage for user-facing notification structure */
    adi_wil_health_report_t Report;

    /* Initialize notification structure */
    (void) memset (&Report, 0, sizeof (Report));

    /* Validate input parameters */
    if ((pInternals != (void *) 0) &&
        (pNotif != (void *) 0) &&
        (pData != (void *) 0))
    {
        /* Ensure we've got a valid health report */
        if (pNotif->iLength <= ADI_WIL_HR_DATA_SIZE)
        {
            /* Populate the user notification structure */
            Report.eDeviceId = wb_wil_GetExternalDeviceId (pNotif->iDeviceId);
            Report.iLength = pNotif->iLength;
            Report.iPacketGenerationTime = pNotif->iASN;
            (void) memcpy (&Report.Data, pData, pNotif->iLength);

            /* Generate the event to the user */
            wb_wil_ui_GenerateEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_DATA_READY_HEALTH_REPORT,
                                     &Report);

            /* Check if this notification requires an acknowledgment. A
             * iNotifId of 0 indicates no acknowledgment required */
            if ((pNotif->iNotifId != 0u))
            {
                /* Non-zero notification ID - trigger acknowledgment */
                wb_wil_AcknowledgeHealthReport (pInternals,
                                                iDeviceId,
                                                pNotif->iNotifId);
            }
        }
    }
}

/******************************************************************************
 * Static function definitions
 *****************************************************************************/

static void wb_wil_AcknowledgeHealthReport (adi_wil_pack_internals_t const * const pInternals,
                                            uint64_t iDeviceId,
                                            uint16_t iNotifId)
{
    /* Storage for local reference to port on which to queue acknowledgment */
    adi_wil_port_t * pPort;

    /* Initialize port reference based on received device ID */
    pPort = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? pInternals->pManager0Port : pInternals->pManager1Port;

    /* Validate port pointer is non-NULL before dereference */
    if ((void *) 0 != pPort)
    {
        /* Attempt to queue acknowledgment.. */
        if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue,
                                                   iNotifId,
                                                   WBMS_NOTIF_HEALTH_REPORT))
        {
            /* If acknowledgment queue failed, increment statistic */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
        }
    }
}

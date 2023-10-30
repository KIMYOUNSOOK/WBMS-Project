/******************************************************************************
 * @file    wb_wil_node_state.c
 *
 * @brief   Handle Node State Notification.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "adi_wil_pack_internals.h"
#include "wb_wil_node_state.h"
#include "wb_ntf_node_state.h"
#include "wb_ntf_node_mode_mismatch.h"
#include "wb_wil_ack.h"
#include "wb_wil_utils.h"
#include "wb_wil_ui.h"
#include <string.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

void wb_wil_HandleNodeState (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_notif_node_state_t const * const pNotif)
{
    adi_wil_port_t * pPort;
    bool bStatus;
    adi_wil_device_t iNwDeviceId;

    /* Convert from integer to boolean. 0 = disconnected : 1 = connected */
    bStatus = ((pNotif->iState & 1u) == 1u);

    /* Check device ID is within range */
    if (pNotif->iDeviceID < pInternals->NodeState.iCount)
    {
        iNwDeviceId = wb_wil_GetExternalDeviceId (pNotif->iDeviceID);

        if (bStatus)
        {
            pInternals->NodeState.iConnectState |= iNwDeviceId;

            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_NODE_CONNECTED, &iNwDeviceId);
        }
        else
        {
            pInternals->NodeState.iConnectState &= ~iNwDeviceId;

            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_NODE_DISCONNECTED, &iNwDeviceId);
        }

        pPort = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? pInternals->pManager0Port : pInternals->pManager1Port;

        if (((void *) 0 != pPort) && (pNotif->iNotifId != 0u))
        {
            if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue, pNotif->iNotifId, WBMS_NOTIF_NODE_STATE))
            {
                wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
            }
        }
    }
}


void wb_wil_HandleNodeModeMismatchNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wbms_notif_node_mode_mismatch_t const * const pNotif)
{
    /* Storage for event data */
    adi_wil_device_t iDevice;

    /* Validate input parameters */
    if (((void *) 0 != pInternals) &&
        ((void *) 0 != pPort) &&
        ((void *) 0 != pNotif))
    {
        /* Populate local device ID variable */
        iDevice = wb_wil_GetExternalDeviceId (pNotif->iDeviceID);

        /* Generate the event */
        wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_NODE_MODE_MISMATCH, &iDevice);

        /* Only send an acknowledgment if notification id is non-zero */
        if (pNotif->iNotifId != 0u)
        {
            /* Add acknowledgment to queue */
            if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue,
                                                       pNotif->iNotifId,
                                                       WBMS_NOTIF_NODE_MODE_MISMATCH))
            {
                /* Increment internal statistic if we failed to queue
                 * acknowledgment */
                wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
            }
        }
    }
}

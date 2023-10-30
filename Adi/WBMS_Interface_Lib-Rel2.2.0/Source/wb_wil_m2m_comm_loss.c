/******************************************************************************
* @file    wb_wil_m2m_comm_loss.c
*
* @brief   Handle fault notification.
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*****************************************************************************/

#include "wbms_cmd_mgr_defs.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"
#include "wb_wil_m2m_comm_loss.h"
#include "wb_ntf_m2m_comm_loss.h"
#include "wb_wil_ack.h"
#include "wb_wil_utils.h"
#include "wb_wil_ui.h"
#include <string.h>

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

void wb_wil_HandleM2MCommLossNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wbms_notif_m2m_comm_loss_t const * const pNotif)
{
    adi_wil_port_t * pPort;

    /* Generate an event with the error code for M2M Communication loss. */
    wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_MGR_TO_MGR_ERROR, (void*)0);

    pPort = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? pInternals->pManager0Port : pInternals->pManager1Port;

    if (((void *) 0 != pPort) && (pNotif->iNotifId != 0u))
    {
        if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue, pNotif->iNotifId, WBMS_NOTIF_M2M_COMM_LOSS))
        {
            /* Keep track of lost ACK count */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
        }
    }
}

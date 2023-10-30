/******************************************************************************
 * @file     wb_wil_process_task.c
 *
 * @brief    WBMS Interface Library API source.
 *
 * @details  Contains API declarations for the WBMS Interface Library.
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_process_task.h"
#include "wb_nil.h"
#include "wb_ntf_ack.h"
#include "wb_wil_request.h"
#include "wb_wil_connect.h"
#include "wb_wil_ack.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "wb_wil_ui.h"
#include "adi_wil_pack_internals.h"
#include "wb_assl.h"
#include <string.h>

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

static adi_wil_err_t wb_wil_ProcessPendingTasks (adi_wil_pack_internals_t const * const pInternals,
                                                 adi_wil_port_t * const pPort);

static void wb_wil_ProcessPendingASSLMessages (adi_wil_pack_internals_t * const pInternals,
                                               adi_wil_port_t const * const pManager0Port,
                                               adi_wil_port_t const * const pManager1Port);

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

adi_wil_err_t wb_wil_ProcessTaskAPI (adi_wil_pack_internals_t * const pInternals,
                                     uint32_t iCurrentTicks)
{
    adi_wil_err_t rc;
    adi_wil_pack_t * pLockId;

    rc = ADI_WIL_ERR_SUCCESS;

    /* Proceed only if input is valid */
    if (pInternals != (void *) 0)
    {
        (void) memset (&pLockId, 0xFF, sizeof (void *));
        rc = wb_wil_ui_AcquireLock ((void *) 0, (pLockId));

        /* If processing is activated for this pack, perform the processing */
        if (rc == ADI_WIL_ERR_SUCCESS)
        {
            rc = adi_wil_ValidateInstance (pInternals, false);

            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                /* Process manager 0 and 1 separately */
                if (pInternals->pManager0Port != (void *) 0)
                {
                    (void) wb_nil_Process (pInternals->pManager0Port);
                }

                if (pInternals->pManager1Port != (void *) 0)
                {
                    (void) wb_nil_Process (pInternals->pManager1Port);
                }

                /* Attempt to reconnect if the corresponding flag is set */
                if (pInternals->bReconnectPending)
                {
                    adi_wil_Reconnect (pInternals);
                }

                wb_wil_CheckConnectionTimeout (pInternals, iCurrentTicks);
                wb_wil_api_CheckForTimeout (pInternals, iCurrentTicks);

                if ((void *) 0 != pInternals->pManager0Port)
                {
                    rc = wb_wil_ProcessPendingTasks (pInternals, pInternals->pManager0Port);
                }

                if ((void *) 0 != pInternals->pManager1Port)
                {
                    rc = wb_wil_ProcessPendingTasks (pInternals, pInternals->pManager1Port);
                }

                wb_wil_ProcessPendingASSLMessages (pInternals, pInternals->pManager0Port, pInternals->pManager1Port);
            }

            wb_wil_ui_ReleaseLock ((void *)0, (pLockId));
        }
        else if (rc == ADI_WIL_ERR_API_IN_PROGRESS)
        {
            rc = ADI_WIL_ERR_SUCCESS;
        }
        else
        {
            /* MISRA else */
        }
    }

    wb_nil_ProcessAllPorts ();

    return rc;
}

static adi_wil_err_t wb_wil_ProcessPendingTasks (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort)
{
    adi_wil_err_t rc;
    wbms_notif_ack_t ack = { 0 };
    uint8_t iNotifType;

    rc = ADI_WIL_ERR_SUCCESS;

    /* Validate input port */
    /* GCOV_EXCL_START - Ignore defensive programming technique checking input
     * params in static function */
    if ((void *) 0 == pPort)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* GCOV_EXCL_STOP - Finished checking input parameters */
    else if (!pPort->Internals.bProcessTaskRequestFramePending)
    {
        /* If we've got the background request tx slot available, check if
         * there's any outgoing messages */
        if (pPort->Internals.bConnectionRequestPending)
        {
            wb_wil_TriggerConnectRequest (pInternals, pPort);
        }
        else if (ADI_WIL_ERR_SUCCESS == wb_wil_ack_Get (&pPort->Internals.AckQueue,
                                                        &ack.iNotifId,
                                                        &iNotifType))
        {
            rc = wb_wil_GenericAcknowledgement (pPort,
                                                &ack,
                                                iNotifType);

            if (rc != ADI_WIL_ERR_SUCCESS)
            {
                wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckTxFailCount);
            }
        }
        else
        {
            /* No outgoing messages - return */
        }
    }
    else
    {
        /* Background request already pending - return */
    }


    return rc;
}

static void wb_wil_ProcessPendingASSLMessages (adi_wil_pack_internals_t * const pInternals,
                                               adi_wil_port_t const * const pManager0Port,
                                               adi_wil_port_t const * const pManager1Port)
{
    uint8_t * pMessage;
    uint8_t iDeviceId;
    bool bProcessTaskRequestPending;

    bProcessTaskRequestPending = false;

    /* Check if there are any pending messages on Manager 0 port */
    if ((void *) 0 != pManager0Port)
    {
        if (pManager0Port->Internals.bProcessTaskRequestFramePending)
        {
            bProcessTaskRequestPending = true;
        }
    }

    /* Check if there are any pending messages on Manager 1 port */
    if ((void *) 0 != pManager1Port)
    {
        if (pManager1Port->Internals.bProcessTaskRequestFramePending)
        {
            bProcessTaskRequestPending = true;
        }
    }

    /* Check if process task message is not pending on either port */
    if (!bProcessTaskRequestPending)
    {
        if (wb_assl_GetPendingMessage (pInternals->pPack, &pMessage, &iDeviceId))
        {
            (void) wb_wil_ASSLSendDataRequest (pInternals, pMessage, iDeviceId);

            wb_assl_ReleaseBuffer (pInternals->pPack, pMessage);
        }
    }
}

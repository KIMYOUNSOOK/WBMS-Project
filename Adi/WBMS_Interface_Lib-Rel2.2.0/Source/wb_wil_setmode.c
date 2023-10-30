/******************************************************************************
 * @file    wb_wil_setmode.c
 *
 * @brief   Get the system mode for the given network
 *
 * Copyright(c) 2022 Analog Devices, Inc.All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.and its licensors.
 *****************************************************************************/

#include "wb_wil_setmode.h"
#include "wb_wil_request.h"
#include "wb_req_set_mode.h"
#include "wb_rsp_set_mode.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "wb_wil_device.h"
#include "wb_wil_xms_sequence_generation.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetModeFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_SetModeComplete (adi_wil_pack_internals_t * const pInternals,
                                    adi_wil_err_t rc);

static adi_wil_err_t wb_wil_ValidateModeTransition (adi_wil_pack_internals_t const * const pInternals,
                                                    adi_wil_mode_t eMode);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetModeAPI (adi_wil_pack_internals_t * const pInternals,
                                 adi_wil_mode_t eMode)
{
    uint64_t iDeviceId;
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_ALL_MANAGERS };
    adi_wil_err_t rc;
    bool bReleaseLock;

    /* Initialize local variables */
    iDeviceId = 0ULL;
    bReleaseLock = false;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        if ((void *) 0 != pInternals->pManager0Port)
        {
            /* Set a flag we are expecting a response from manager 0
             * if connected */
            pInternals->SetModeState.bExpectManager0Response = pInternals->pManager0Port->Internals.bConnected;

            /* If manager 0 is connected, select it */
            if (pInternals->pManager0Port->Internals.bConnected)
            {
                iDeviceId = ADI_WIL_DEV_MANAGER_0;
            }
        }

        if ((void *) 0 != pInternals->pManager1Port)
        {
            /* Set a flag we are expecting a response from manager 1
             * if connected */
            pInternals->SetModeState.bExpectManager1Response = pInternals->pManager1Port->Internals.bConnected;

            /* If we didn't select manager 0 and manager 1 is connected,
             * select manager 1 */
            if ((iDeviceId == 0u) && pInternals->pManager1Port->Internals.bConnected)
            {
                iDeviceId = ADI_WIL_DEV_MANAGER_1;
            }
        }

        /* If at least one manager is connected, we're good. If not, return
         * ADI_WIL_ERR_NOT_CONNECTED */
        if (iDeviceId == 0u)
        {
            rc = ADI_WIL_ERR_NOT_CONNECTED;
        }
        /* ... Otherwise, validate the mode transition */
        else
        {
            rc = wb_wil_ValidateModeTransition (pInternals, eMode);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* If transitioning between the STANDBY and OTAP modes, send
             * SetMode commands to both managers, otherwise send to only
             * one manager */
            if ( ((pInternals->eMode == ADI_WIL_MODE_STANDBY) && (eMode == ADI_WIL_MODE_OTAP)) ||
                 ((pInternals->eMode == ADI_WIL_MODE_OTAP) && (eMode == ADI_WIL_MODE_STANDBY)) ||
                 ((pInternals->eMode == ADI_WIL_MODE_OTAP) && (eMode == ADI_WIL_MODE_OTAP)) )
            {
                rc = wb_wil_SetupRequest (pInternals, ADI_WIL_DEV_ALL_MANAGERS, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_SetModeFunc);
            }
            else
            {
                rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_SetModeFunc);
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->SetModeState.iNodeModeMask = 0ULL;
            pInternals->SetModeState.eMode = eMode;
            wb_wil_SetModeFunc (pInternals);
        }

        /* Release lock if any of the above steps returns a failure */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;
        }
    }

    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    return rc;
}

adi_wil_err_t wb_wil_GetModeAPI (adi_wil_pack_internals_t const * const pInternals,
                                 adi_wil_mode_t * const pMode)
{
    adi_wil_err_t rc;
    void const * const NullableParams[] = { pMode };

    rc = adi_wil_ValidateInstance (pInternals, true);

    if (rc == ADI_WIL_ERR_SUCCESS)
    {
        /* Validate input parameters */
        rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        if ((pInternals->eMode != ADI_WIL_MODE_STANDBY) &&
            (pInternals->eMode != ADI_WIL_MODE_ACTIVE) &&
            (pInternals->eMode != ADI_WIL_MODE_COMMISSIONING) &&
            (pInternals->eMode != ADI_WIL_MODE_MONITORING) &&
            (pInternals->eMode != ADI_WIL_MODE_OTAP) &&
            (pInternals->eMode != ADI_WIL_MODE_SLEEP))
        {
            /* If the WIL has been initialized, but a successful connection
             * response has not yet been received, then eMode will be invalid,
             * and the system mode is not yet known */
            rc = ADI_WIL_ERR_INVALID_STATE;
        }
        else
        {
            /* This is a valid mode - so, pass it on */
            *pMode = pInternals->eMode;
        }
    }

    return rc;
}

static void wb_wil_SetModeFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_mode_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iMode = wb_wil_GetUintFromWILMode (pInternals->SetModeState.eMode);

    /* Reset pending responses to what they were initialized to */
    pInternals->SetModeState.bPendingManager0Response = pInternals->SetModeState.bExpectManager0Response;
    pInternals->SetModeState.bPendingManager1Response = pInternals->SetModeState.bExpectManager1Response;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_SetModeComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetModeRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_SetModeComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

void wb_wil_HandleSetModeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_set_mode_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_SetModeComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        /* Clear pending flag for device we just received response from */
        if (ADI_WIL_DEV_MANAGER_0 == iDeviceId)
        {
            pInternals->SetModeState.bPendingManager0Response = false;
        }
        else
        {
            pInternals->SetModeState.bPendingManager1Response = false;
        }

        /* Collate the node bitmap with the existing bitmap */
        pInternals->SetModeState.iNodeModeMask |= ((uint64_t) pResponse->nodeBitMap [0u] |
                                                  ((uint64_t) pResponse->nodeBitMap [1u] << 8u) |
                                                  ((uint64_t) pResponse->nodeBitMap [2u] << 16u) |
                                                  ((uint64_t) pResponse->nodeBitMap [3u] << 24u) |
                                                  ((uint64_t) pResponse->nodeBitMap [4u] << 32u) |
                                                  ((uint64_t) pResponse->nodeBitMap [5u] << 40u) |
                                                  ((uint64_t) pResponse->nodeBitMap [6u] << 48u) |
                                                  ((uint64_t) pResponse->nodeBitMap [7u] << 56u));

        /* Check if we are pending on any more responses. Complete if all
         * have been received */
        if (!pInternals->SetModeState.bPendingManager0Response &&
            !pInternals->SetModeState.bPendingManager1Response)
        {
            /* Clear the token now that request has finished */
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            /* If the mode transition is between Standby and OTAP, clear the
             * node connection status mask and not to update eMode to the new
             * mode since at this point the managers have not yet rebooted and
             * switched to the new mode.eMode will be updated when the WIL
             * reconnects to the managers after their reboots */
            if ( ((pInternals->eMode == ADI_WIL_MODE_STANDBY) && (pInternals->SetModeState.eMode == ADI_WIL_MODE_OTAP)) ||
                 ((pInternals->eMode == ADI_WIL_MODE_OTAP) && (pInternals->SetModeState.eMode == ADI_WIL_MODE_STANDBY)) )
            {
                pInternals->NodeState.iConnectState = 0ULL;
            }
            else
            {
                /* Set internally cached mode to requested mode, even if the
                 * network has only partially switched, the managers would be
                 * in the requested mode if it returned SUCCESS */
                pInternals->eMode = pInternals->SetModeState.eMode;
            }

            if (pInternals->SetModeState.eMode != ADI_WIL_MODE_COMMISSIONING)
            {
                /* Mask off any nodes we received a response from but haven't
                 * received a connection notification for yet */
                pInternals->SetModeState.iNodeModeMask &= pInternals->NodeState.iConnectState;

                /* If there are 
                 * 1. No nodes in the ACL, or
                 * 2. We received a response from each connected node
                 * ... return success */
                if ((pInternals->NodeState.iCount == 0u) || 
                    (pInternals->SetModeState.iNodeModeMask == pInternals->NodeState.iConnectState))
                {
                    wb_wil_SetModeComplete (pInternals, ADI_WIL_ERR_SUCCESS);
                }
                else
                {
                    wb_wil_SetModeComplete (pInternals, ADI_WIL_ERR_PARTIAL_SUCCESS);
                }
            }
            else
            {
                wb_wil_SetModeComplete (pInternals, ADI_WIL_ERR_SUCCESS);
            }
        }
    }
}

static void wb_wil_SetModeComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    if (pInternals->eMode == ADI_WIL_MODE_STANDBY)
    {
        wb_wil_XmsStandbyTransition (pInternals);
    }

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SET_MODE, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

static adi_wil_err_t wb_wil_ValidateModeTransition (adi_wil_pack_internals_t const * const pInternals, adi_wil_mode_t eMode)
{
    adi_wil_err_t rc;

    if ((eMode != ADI_WIL_MODE_STANDBY) && (eMode != ADI_WIL_MODE_COMMISSIONING) &&
        (eMode != ADI_WIL_MODE_ACTIVE) && (eMode != ADI_WIL_MODE_MONITORING) &&
        (eMode != ADI_WIL_MODE_OTAP) && (eMode != ADI_WIL_MODE_SLEEP))
    {
        /* Only standby, commissioning, active, monitoring, OTAP and sleep
         * modes are supported */
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else if (eMode == pInternals->eMode)
    {
        /* There is a mode match */
        rc = ADI_WIL_ERR_SUCCESS;
    }
    else if ((eMode == ADI_WIL_MODE_ACTIVE) &&
            ((pInternals->eMode == ADI_WIL_MODE_SLEEP) || (pInternals->eMode == ADI_WIL_MODE_MONITORING)))
    {
        /* Allow transition from Sleep/Monitoring to Active without going to Standby first */
        rc = ADI_WIL_ERR_SUCCESS;
    }
    else if ((eMode != ADI_WIL_MODE_STANDBY) && (pInternals->eMode != ADI_WIL_MODE_STANDBY))
    {
        rc = ADI_WIL_ERR_INVALID_MODE;
    }
    else
    {
        /* This is a valid mode */
        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

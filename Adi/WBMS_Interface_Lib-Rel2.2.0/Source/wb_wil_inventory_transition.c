/******************************************************************************
 * @file    wb_wil_inventory_transition.c
 *
 * @brief   Enter or exit inventory state on one or all nodes
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_inventory_transition.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_request.h"
#include "wb_req_inventory_transition.h"
#include "wb_rsp_generic.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_InventoryTransitionFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_InventoryTransitionComplete (adi_wil_pack_internals_t * const pInternals,
                                                adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_EnterInventoryStateAPI (adi_wil_pack_internals_t * const pInternals,
                                             uint64_t iDeviceId,
                                             bool bEnable,
                                             uint64_t iCurrentTime)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_ALL_NODES, ADI_WIL_TARGET_SINGLE_NODE };
    bool bReleaseLock = false;

    /* Validate input parameter */
    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Validate input system mode */
        rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_InventoryTransitionFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->InventoryTransitionState.iExitInventoryStateFlag = bEnable ? 0u : 1u;
            pInternals->InventoryTransitionState.iCurrentTime = iCurrentTime;
            /* Send 'Inventory transition' request packet */
            wb_wil_InventoryTransitionFunc (pInternals);
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

void wb_wil_HandleInventoryTransitionResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_InventoryTransitionComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
    {
        wb_wil_InventoryTransitionComplete (pInternals, ADI_WIL_ERR_SUCCESS);
    }
    else
    {
        /* MISRA else */
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_InventoryTransitionFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_inventory_transition_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iTimeInSeconds = pInternals->InventoryTransitionState.iCurrentTime;
    Request.bExitFlag = pInternals->InventoryTransitionState.iExitInventoryStateFlag;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_InventoryTransitionComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_InventoryTransitionRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_InventoryTransitionComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_InventoryTransitionComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_ENTER_INVENTORY_STATE, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

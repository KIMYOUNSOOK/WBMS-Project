/******************************************************************************
 * @file    wb_wil_ResetDevice.c
 *
 * @brief   Reset the specified Network Manager(s) or Node(s)
 *
 * Copyright(c) 2022 Analog Devices, Inc.All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.and its licensors.
 *****************************************************************************/

#include "wb_wil_reset.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_request.h"
#include "adi_wil_types.h"
#include "wb_req_generic.h"
#include "wb_rsp_generic.h"
#include "wb_wil_utils.h"
#include "wb_wil_connect.h"
#include "wb_wil_api.h"
#include <string.h>

 /******************************************************************************
  * Static functions
  *****************************************************************************/

static void wb_wil_ResetFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_ResetComplete (adi_wil_pack_internals_t * const pInternals,
                                  adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ResetDeviceAPI (adi_wil_pack_internals_t * const pInternals,
                                     uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_ALL_NODES };
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
        /* Send reset request packet */
        rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_ResetFunc);

        if ((ADI_WIL_ERR_SUCCESS == rc) &&
            ((ADI_WIL_TARGET_SINGLE_MANAGER == pInternals->UserRequestState.eTarget) ||
             (ADI_WIL_TARGET_ALL_MANAGERS == pInternals->UserRequestState.eTarget)))
        {
            pInternals->bMaintainConnection = false;
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            wb_wil_ResetFunc (pInternals);
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

static void wb_wil_ResetFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        /* If reset request function has been tried for more than the max retry
         * count, abort with return count = TIMEOUT */
        wb_wil_ResetComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_RESET, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        /* If reset request function has not been successful, abort with return
         * count = FAIL */
        wb_wil_ResetComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        pInternals->ResetDeviceState.iToken = Request.iToken;
    }
}

adi_wil_err_t wb_wil_ResetMgrNotifyConnectionLost (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId)
{
    adi_wil_err_t rc;

    if (pInternals->UserRequestState.pfRequestFunc != &wb_wil_ResetFunc)
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else if ((ADI_WIL_TARGET_SINGLE_MANAGER != pInternals->UserRequestState.eTarget) &&
             (ADI_WIL_TARGET_ALL_MANAGERS != pInternals->UserRequestState.eTarget))
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_ClearPendingResponse (pInternals, iDeviceId))
    {
        /* Return success but don't stop waiting for the other manager
         * to reset */
        rc = ADI_WIL_ERR_SUCCESS;
    }
    else
    {
        (void) wb_wil_api_CheckToken (pInternals, pInternals->ResetDeviceState.iToken, true);
        /* Connection to manager(s) lost so move to complete stage */
        wb_wil_ResetComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

void wb_wil_HandleResetResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (wb_wil_api_CheckToken (pInternals, pResponse->iToken, false) != ADI_WIL_ERR_SUCCESS)
    {
        /* Do nothing : there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_ResetComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            wb_wil_ResetComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

static void wb_wil_ResetComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    if ((ADI_WIL_TARGET_SINGLE_MANAGER == pInternals->UserRequestState.eTarget) ||
        (ADI_WIL_TARGET_ALL_MANAGERS == pInternals->UserRequestState.eTarget))
    {
        pInternals->bMaintainConnection = true;
    }

    /* Generate Callback with the return code received (as an input
     * parameter) */
    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_RESET_DEVICE, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

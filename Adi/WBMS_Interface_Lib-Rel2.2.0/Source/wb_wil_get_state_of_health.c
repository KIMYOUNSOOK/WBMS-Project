/******************************************************************************
 * @file    wb_wil_get_stateofhealth.c
 *
 * @brief   Get the State of health on a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_state_of_health.h"
#include "wb_wil_request.h"
#include "wb_req_generic.h"
#include "wb_rsp_get_state_of_health.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetStateOfHealthFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_GetStateOfHealthComplete (adi_wil_pack_internals_t * const pInternals,
                                             adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetStateOfHealthAPI (adi_wil_pack_internals_t * const pInternals,
                                          uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_NODE };
    bool bReleaseLock = false;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Validate system mode */
        rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set up the correct bit masks */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetStateOfHealthFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send state of health request packet */
            wb_wil_GetStateOfHealthFunc (pInternals);
        }

        /* Release lock if any of the above */
        /* steps returns a failure */
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

void wb_wil_HandleGetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals,
                                            uint64_t iDeviceId,
                                            wbms_cmd_resp_get_state_of_health_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        pInternals->StateOfHealthState.iPercentage = pResponse->iPercentage;
        wb_wil_GetStateOfHealthComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}


/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetStateOfHealthFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetStateOfHealthComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_GET_STATE_OF_HEALTH, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetStateOfHealthComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetStateOfHealthComplete (adi_wil_pack_internals_t * const pInternals,
                                             adi_wil_err_t rc)
{
    uint8_t iPercentage;

    /* Update the internal variables as 'get state of health' API has
     * completed */
    iPercentage = pInternals->StateOfHealthState.iPercentage;
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* 'get state of health' API has succeeded, so, generate a callback
         * with a valid and populated return structure */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_STATE_OF_HEALTH, rc, (void *) &iPercentage);
    }
    else
    {
        /* Because 'get state of health' API has not succeeded, generate a
         * callback with a null return structure */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_STATE_OF_HEALTH, rc, (void *) 0);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

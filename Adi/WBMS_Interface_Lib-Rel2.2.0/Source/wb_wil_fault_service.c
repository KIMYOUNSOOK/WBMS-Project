/******************************************************************************
 * @file    wb_wil_fault_service.c
 *
 * @brief   Erase the black-box log on a manager or a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_fault_service.h"
#include "wb_wil_request.h"
#include "wb_req_fault_service.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_FaultServicingFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_FaultServicingComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_EnableFaultServicingAPI (adi_wil_pack_internals_t * const pInternals,
                                              uint64_t iDeviceId,
                                              bool bEnable)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY, ADI_WIL_MODE_ACTIVE };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER };
    bool bReleaseLock = false;

    /* Validate input parameters */
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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_FaultServicingFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send 'Fault servicing' request packet */
            pInternals->EnvMonState.bEnableFaultServicing = bEnable;
            wb_wil_FaultServicingFunc (pInternals);
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

void wb_wil_HandleFaultServicingResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        wb_wil_FaultServicingComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_FaultServicingFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_fault_service_t Request = { 0 };

    Request.iEnable = (pInternals->EnvMonState.bEnableFaultServicing == true) ? 1u : 0u;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_FaultServicingComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetFaultModeRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_FaultServicingComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_FaultServicingComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_ENABLE_FAULT_SERVICING, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

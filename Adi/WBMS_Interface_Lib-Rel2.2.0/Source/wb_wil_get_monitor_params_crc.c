/******************************************************************************
 * @file    wb_wil_get_monitor_parameters_crc.c
 *
 * @brief   Retrieves the CRC of the monitor parameters from a devices
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_monitor_params_crc.h"
#include "wb_wil_ui.h"
#include "wb_wil_request.h"
#include "wb_req_generic.h"
#include "wb_rsp_get_mon_params_crc.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetMonitorParametersCRCFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_GetMonitorParametersCRCComplete (adi_wil_pack_internals_t * const pInternals,
                                                    adi_wil_err_t rc,
                                                    void const * const pResponse);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetMonitorParametersCRCAPI (adi_wil_pack_internals_t * const pInternals,
                                                 uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE };
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
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetMonitorParametersCRCFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send 'Monitor Parameters CRC' request packet */
            wb_wil_GetMonitorParametersCRCFunc (pInternals);
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

void wb_wil_HandleGetMonitorParametersCRCResponse (adi_wil_pack_internals_t * const pInternals,
                                                   wbms_cmd_resp_get_mon_params_crc_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_GetMonitorParametersCRCComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc), (void *) 0);
    }
    else
    {
        wb_wil_GetMonitorParametersCRCComplete (pInternals, ADI_WIL_ERR_SUCCESS, &pResponse->iCRC);
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetMonitorParametersCRCFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetMonitorParametersCRCComplete (pInternals, ADI_WIL_ERR_TIMEOUT, (void *) 0);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_GET_MON_PARAMS_CRC, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetMonitorParametersCRCComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetMonitorParametersCRCComplete (adi_wil_pack_internals_t * const pInternals,
                                                    adi_wil_err_t rc,
                                                    void const * const pResponse)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_MONITOR_PARAMETERS_CRC, rc, pResponse);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

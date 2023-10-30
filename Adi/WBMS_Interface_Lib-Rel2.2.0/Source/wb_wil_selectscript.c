/******************************************************************************
 * @file    wb_wil_selectscript.c
 *
 * @brief   State machine to set BMS/PMS script id on node/manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_selectscript.h"
#include "wb_wil_request.h"
#include "wb_req_select_script.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/*****************************************************************************/
/* Static function declarations                                              */
/*****************************************************************************/

static void wb_wil_SelectScriptFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_SelectScriptComplete (adi_wil_pack_internals_t * const pInternals,
                                         adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SelectScriptAPI (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      adi_wil_sensor_id_t eSensorId,
                                      uint8_t iScriptId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_ACTIVE };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_NODES };
    uint8_t iSensorId;
    bool bReleaseLock;

    /* Initialize local variables */
    iSensorId = 0u;
    bReleaseLock = false;

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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_SelectScriptFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Map sensor ID type from adi_wil_sensor_id_t to uint8_t* */
            rc = wb_wil_SensorIdToUint (eSensorId, &iSensorId);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->SelectScriptState.iScriptId = iScriptId;
            pInternals->SelectScriptState.iSensorId = iSensorId;
            /* Send 'select script' request packet */
            wb_wil_SelectScriptFunc (pInternals);
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

void wb_wil_HandleSelectScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with not a SUCCESS. Complete the 'Select
         * script' API with the return code received from the end device */
        wb_wil_SelectScriptComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            /* Select script API has been completed successfully.
             * Generate call back to the host application with return code of
             * SUCCESS */
            wb_wil_SelectScriptComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SelectScriptFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_select_script_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iScriptId = pInternals->SelectScriptState.iScriptId;
    Request.iSensorId = pInternals->SelectScriptState.iSensorId;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_SelectScriptComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (wb_wil_SelectScriptRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS) != ADI_WIL_ERR_SUCCESS)
    {
        wb_wil_SelectScriptComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA else */
    }
}

static void wb_wil_SelectScriptComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SELECT_SCRIPT, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

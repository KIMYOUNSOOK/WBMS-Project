/******************************************************************************
 * @file    wb_wil_modifyscript.c
 *
 * @brief   State machine to set BMS/PMS script entry on node/manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_modifyscript.h"
#include "wb_wil_request.h"
#include "wb_req_modify_script.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/*****************************************************************************/
/* Static function declarations                                              */
/*****************************************************************************/

static void wb_wil_ModifyScriptFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_ModifyScriptComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ModifyScriptAPI (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      adi_wil_sensor_id_t eSensorId,
                                      adi_wil_script_change_t const * const pData)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_ACTIVE };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE };
    void const * const NullableParams[] = { pData };
    uint8_t iSensorId;
    bool bReleaseLock = false;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Validate input parameters */
        rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Check if data length is within limits */
            if (pData->iChangeDataLength > ADI_WIL_MAX_SCRIPT_CHANGE_DATA_LENGTH)
            {
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
            }
            else
            {
                /* Verify if the mode of the system is valid */
                rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);

                if (ADI_WIL_ERR_SUCCESS == rc)
                {
                    rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_ModifyScriptFunc);

                    if (ADI_WIL_ERR_SUCCESS == rc)
                    {
                        rc = wb_wil_SensorIdToUint (eSensorId, &iSensorId);

                        if (ADI_WIL_ERR_SUCCESS == rc)
                        {
                            /* Populate internal structure that would be used
                             * in preparing the packet */
                            pInternals->ModifyScriptState.iSensorId = iSensorId;
                            pInternals->ModifyScriptState.ScriptChange.iActivationTime = pData->iActivationTime;
                            pInternals->ModifyScriptState.ScriptChange.iChangeDataLength = pData->iChangeDataLength;
                            pInternals->ModifyScriptState.ScriptChange.iEntryOffset = pData->iEntryOffset;
                            (void) memcpy (&pInternals->ModifyScriptState.ScriptChange.iChangeData [0], &pData->iChangeData [0], pData->iChangeDataLength);
                            /* Send Modify script request packet */
                            wb_wil_ModifyScriptFunc (pInternals);
                        }
                    }
                }
            }
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

void wb_wil_HandleModifyScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active Request with this token */
    }
    else
    {
        wb_wil_ModifyScriptComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_ModifyScriptFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_modify_script_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iSensorId = pInternals->ModifyScriptState.iSensorId;
    Request.iActivationTime = pInternals->ModifyScriptState.ScriptChange.iActivationTime;
    Request.iOffset = pInternals->ModifyScriptState.ScriptChange.iEntryOffset;
    Request.iLength = pInternals->ModifyScriptState.ScriptChange.iChangeDataLength;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_ModifyScriptComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (wb_wil_ModifyScriptRequest (pInternals, &Request, &pInternals->ModifyScriptState.ScriptChange.iChangeData [0], Request.iLength, ADI_WIL_RESPONSE_TIMEOUT_MS) != ADI_WIL_ERR_SUCCESS)
    {
        wb_wil_ModifyScriptComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA else */
    }
}

static void wb_wil_ModifyScriptComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_MODIFY_SCRIPT, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

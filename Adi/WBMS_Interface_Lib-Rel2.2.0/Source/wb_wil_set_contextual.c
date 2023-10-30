/******************************************************************************
 * @file    wb_wil_set_contextual.c
 *
 * @brief   Set the Contextual Data on a Manager or Node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_set_contextual.h"
#include "wb_wil_request.h"
#include "wb_req_set_contextual.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetContextualFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_SetContextualComplete (adi_wil_pack_internals_t * const pInternals,
                                          adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetContextualDataAPI (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId,
                                           adi_wil_contextual_id_t eContextualDataId,
                                           adi_wil_contextual_data_t const * const pData)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE };
    void const * const NullableParams[] = { pInternals , pData };
    uint8_t iContextualId;
    bool bReleaseLock;

    /* Initialize local variables */
    iContextualId = 0u;
    bReleaseLock = false;

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
            /* Validate system mode */
            rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Validate length of data */
            rc = (pData->iLength <= ADI_WIL_CONTEXTUAL_DATA_SIZE) ? ADI_WIL_ERR_SUCCESS : ADI_WIL_ERR_INVALID_PARAMETER;
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Map contextual type from adi_wil_contextual_id_t to uint8_t* */
            rc = wb_wil_ContextualIDToUint (eContextualDataId, &iContextualId);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_SetContextualFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set up the internal variables required to send the request
             * packet */
            pInternals->ContextualState.ContextualData.iLength = pData->iLength;
            pInternals->ContextualState.iContextualId = iContextualId;
            (void) memcpy (&pInternals->ContextualState.ContextualData.Data [0], pData->Data, pData->iLength);
            /* Send 'set contextual data' request packet */
            wb_wil_SetContextualFunc (pInternals);
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

void wb_wil_HandleSetContextualResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        /* End device has responded with a WAIT. So, resend the 'Set
         * Contextual' packet */
        wb_wil_SetContextualFunc (pInternals);
    }
    else
    {
        /* Complete the 'Set Contextual' API with the return code received from
         * the end device */
        wb_wil_SetContextualComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetContextualFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_contextual_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iLength = pInternals->ContextualState.ContextualData.iLength;
    Request.iContextualId = pInternals->ContextualState.iContextualId;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_SetContextualComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetContextualRequest (pInternals, &Request, &pInternals->ContextualState.ContextualData.Data [0], Request.iLength, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_SetContextualComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_SetContextualComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SET_CONTEXTUAL_DATA, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

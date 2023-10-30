/******************************************************************************
 * @file    wb_wil_get_file.c
 *
 * @brief   Get the a file on a device
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_file.h"
#include "wb_wil_request.h"
#include "wb_req_get_file.h"
#include "wb_rsp_get_file.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "adi_wil_file.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetFileFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_GetFileComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc, adi_wil_file_t const * const pData);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetFileAPI (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 adi_wil_file_type_t eFileType)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_SINGLE_MANAGER };
    const adi_wil_file_type_t ValidManagerFileTypes[] = { ADI_WIL_FILE_TYPE_PMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, ADI_WIL_FILE_TYPE_CONFIGURATION, 
                                                          ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_ENV_MON };
    const adi_wil_file_type_t ValidNodeFileTypes[] = { ADI_WIL_FILE_TYPE_BMS_CONTAINER, ADI_WIL_FILE_TYPE_PMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, 
                                                       ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_INVENTORY_LOG, ADI_WIL_FILE_TYPE_ENV_MON };

    bool bReleaseLock;

    /* Stores the integer converted file type */
    uint8_t iFileType;

    /* Initialize local variables */
    iFileType = 0u;
    bReleaseLock = false;

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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetFileFunc);
        }

        /* Validate file type for manager */
        if ((ADI_WIL_ERR_SUCCESS == rc) && (ADI_WIL_TARGET_SINGLE_MANAGER == pInternals->UserRequestState.eTarget))
        {
            rc = adi_wil_CheckFileType (pInternals, eFileType, (sizeof (ValidManagerFileTypes) / sizeof (adi_wil_file_type_t)), ValidManagerFileTypes);
        }

        /* Validate file type for node */
        if ((ADI_WIL_ERR_SUCCESS == rc) && (ADI_WIL_TARGET_SINGLE_NODE == pInternals->UserRequestState.eTarget))
        {
            rc = adi_wil_CheckFileType (pInternals, eFileType, (sizeof (ValidNodeFileTypes) / sizeof (adi_wil_file_type_t)), ValidNodeFileTypes);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Store the integer version of the file type in the temporary
             * variable */
            rc = wb_wil_GetUintFromFileType (eFileType, &iFileType);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* If we're not in a current transfer or the input parameters have
             * changed, this is a new request. Reset the offset back to 0 and
             * update the internally stored parameters */
            if (!pInternals->GetFileState.bInProgress ||
                (pInternals->GetFileState.iFileType != iFileType) ||
                (pInternals->GetFileState.iDeviceId != iDeviceId))
            {
                pInternals->GetFileState.bInProgress = true;
                pInternals->GetFileState.iFileType = iFileType;
                pInternals->GetFileState.iDeviceId = iDeviceId;
                pInternals->GetFileState.iOffset = 0u;
            }

            /* Send 'get file' request packet */
            wb_wil_GetFileFunc (pInternals);
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

void wb_wil_HandleGetFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_get_file_t const * const pResponse, uint8_t * const pData)
{
    adi_wil_file_t Response;

    (void) iDeviceId;

    /* Initialize request structure */
    (void) memset (&Response, 0, sizeof (Response));

    Response.pData = pData;
    Response.iByteCount = pResponse->iLength;
    Response.iRemainingBytes = pResponse->iRemainingBytes;
    Response.iOffset = pInternals->GetFileState.iOffset;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* Return as failed */
        wb_wil_GetFileComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc), (void *) 0);
    }
    else if ((pInternals->GetFileState.iOffset + pResponse->iLength) > (uint16_t) UINT16_MAX)
    {
        /* Return as failed */
        wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else if (pResponse->iLength > (uint8_t) WBMS_GET_FILE_BLOCK_SIZE)
    {
        /* Return as failed */
        wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else
    {
        /* Update the internal offset before invoking the callback */
        pInternals->GetFileState.iOffset += pResponse->iLength;

        if (pResponse->iRemainingBytes > 0u)
        {
            /* Invoke the "complete" stage with ADI_WIL_ERR_IN_PROGRESS */
            wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_IN_PROGRESS, &Response);
        }
        else
        {
            /* Return as completed */
            wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_SUCCESS, &Response);
        }
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetFileFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_get_file_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iFileType = pInternals->GetFileState.iFileType;
    Request.iOffset = pInternals->GetFileState.iOffset;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT, (void *) 0);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GetFileRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetFileComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetFileComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc, adi_wil_file_t const * const pData)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    /* Retain the in progress flag if this was an intermediate step in the
     * transfer, otherwise mark the transfer as complete */
    if (rc != ADI_WIL_ERR_IN_PROGRESS)
    {
        pInternals->GetFileState.bInProgress = false;
    }

    /* Generate a callback to the user application */
    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_FILE, rc, pData);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

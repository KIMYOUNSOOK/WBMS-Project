/******************************************************************************
 * @file    wb_wil_erase_file.c
 *
 * @brief   Erase a script container on a manager or a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_erase_file.h"
#include "wb_wil_request.h"
#include "wb_req_erase_file.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_api.h"
#include "wb_wil_utils.h"
#include <string.h>

 /******************************************************************************
  * Static functions
  *****************************************************************************/

static void wb_wil_EraseFileFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_EraseFileComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_EraseFileAPI (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   adi_wil_file_type_t eFileType)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_ALL_NODES, ADI_WIL_TARGET_SINGLE_NODE };

    const adi_wil_file_type_t ValidManagerFileTypes[] = { ADI_WIL_FILE_TYPE_PMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, ADI_WIL_FILE_TYPE_CONFIGURATION, 
                                                          ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_ENV_MON };
    const adi_wil_file_type_t ValidNodeFileTypes[] = { ADI_WIL_FILE_TYPE_BMS_CONTAINER, ADI_WIL_FILE_TYPE_PMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, 
                                                       ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_INVENTORY_LOG, ADI_WIL_FILE_TYPE_ENV_MON };
    bool bReleaseLock = false;

    /* Validate input parameter. */
    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Check for invalid state. If the system is in invalid state, skip the system mode checking to allow file erasure functionality
           since one of the ways to recover an invalid configured system is to erase the configuration file. */
        if (!pInternals->bInvalidState)
        {
            rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_EraseFileFunc);
        }

        /* Handle manager and node cases separately */
        if ((ADI_WIL_ERR_SUCCESS == rc) &&
            ((ADI_WIL_TARGET_SINGLE_MANAGER == pInternals->UserRequestState.eTarget) ||
             (ADI_WIL_TARGET_ALL_MANAGERS == pInternals->UserRequestState.eTarget)))
        {
            /* Verify if the input file type is allowed for manager(s) */
            rc = adi_wil_CheckFileType (pInternals, eFileType, (sizeof (ValidManagerFileTypes) / sizeof (adi_wil_file_type_t)), ValidManagerFileTypes);
        }

        if ((ADI_WIL_ERR_SUCCESS == rc) &&
            ((ADI_WIL_TARGET_SINGLE_NODE == pInternals->UserRequestState.eTarget) ||
             (ADI_WIL_TARGET_ALL_NODES == pInternals->UserRequestState.eTarget)))
        {
            /* Verify if the input file type is allowed for node(s) */
            rc = adi_wil_CheckFileType (pInternals, eFileType, (sizeof (ValidNodeFileTypes) / sizeof (adi_wil_file_type_t)), ValidNodeFileTypes);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Map file type from adi_wil_file_type_t to uint8_t* */
            rc = wb_wil_GetUintFromFileType (eFileType, &pInternals->EraseFileState.iFileType);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send 'erase file' request packet */
            wb_wil_EraseFileFunc (pInternals);
        }

        /* Release lock if any of the above steps returns a failure */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;
        }
    }

    if (bReleaseLock)
    {
        /* Release lock */
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    return rc;
}

void wb_wil_HandleEraseFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        /* Do nothing, timeout this request */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_EraseFileComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            wb_wil_EraseFileComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_EraseFileFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_erase_file_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iFileType = pInternals->EraseFileState.iFileType;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_EraseFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_EraseFileRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_EraseFileComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_EraseFileComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_ERASE_FILE, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

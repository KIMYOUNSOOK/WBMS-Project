/******************************************************************************
 * @file    wb_wil_get_file_crc.c
 *
 * @brief   Erase the black-box log on a manager or a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_file_crc.h"
#include "wb_wil_request.h"
#include "wb_req_get_file_crc.h"
#include "wb_rsp_get_file_crc.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "adi_wil_file_crc.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Defines
 *****************************************************************************/

#define WB_GET_FILE_CRC_LIST_MANAGER_0_INDEX    (62u)
#define WB_GET_FILE_CRC_LIST_MANAGER_1_INDEX    (63u)

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetFileCRCFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_GetFileCRCComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc, void const * const pResponse);
static void wb_wil_GetFileCRCUpdateCRCList (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, uint32_t iCRC, bool bFileExists);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetFileCRCAPI (adi_wil_pack_internals_t * const pInternals,
                                    uint64_t iDeviceId,
                                    adi_wil_file_type_t eFileType)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_ALL_NODES};
    const adi_wil_file_type_t ValidManagerFileTypes[] = { ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_FILE_TYPE_PMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, 
                                                          ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_ENV_MON };
    const adi_wil_file_type_t ValidNodeFileTypes[] = { ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_FILE_TYPE_BMS_CONTAINER, ADI_WIL_FILE_TYPE_EMS_CONTAINER, 
                                                       ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_FILE_TYPE_BLACK_BOX_LOG, ADI_WIL_FILE_TYPE_INVENTORY_LOG, ADI_WIL_FILE_TYPE_ENV_MON };

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
        /* Validate system mode */
        rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);
        
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Map file type from adi_wil_file_type_t to uint8_t* */
            rc = wb_wil_GetUintFromFileType (eFileType, &pInternals->GetFileCRCState.iFileType);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetFileCRCFunc);
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
            /* Reset result structure */
            (void) memset (&pInternals->GetFileCRCState.FileCRCList, 0, sizeof(pInternals->GetFileCRCState.FileCRCList));

            /* Send 'get CRC' request packet */
            wb_wil_GetFileCRCFunc (pInternals);
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

void wb_wil_HandleGetFileCRCResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_get_file_crc_t const * const pResponse)
{
    adi_wil_err_t rc;
    bool bComplete;

    rc = ADI_WIL_ERR_SUCCESS;
    bComplete = false;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        rc = wb_wil_ClearPendingResponse (pInternals, iDeviceId);

        /* Received response from valid device */
        if ((ADI_WIL_ERR_SUCCESS == rc) || (ADI_WIL_ERR_IN_PROGRESS == rc))
        {
            if (WBMS_CMD_RC_SUCCESS == pResponse->rc)
            {
                /* File CRC returned successfully */
                wb_wil_GetFileCRCUpdateCRCList (pInternals, iDeviceId, pResponse->iCRC, true);
            }
            else if (WBMS_CMD_RC_CRC_ERROR == pResponse->rc)
            {
                /* File not present on remote device */
                wb_wil_GetFileCRCUpdateCRCList (pInternals, iDeviceId, 0u, false);
            }
            else
            {
                /* Any other error condition */
                bComplete = true;
                rc = ADI_WIL_ERR_FAIL;
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Received responses from all targets */
            bComplete = true;
            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    if (bComplete)
    {
        wb_wil_GetFileCRCComplete (pInternals, 
                                   rc, 
                                   (rc == ADI_WIL_ERR_SUCCESS) ? (void*) (&pInternals->GetFileCRCState.FileCRCList) : (void*)(0u));
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetFileCRCFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_get_file_crc_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iFileType = pInternals->GetFileCRCState.iFileType;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetFileCRCComplete (pInternals, ADI_WIL_ERR_TIMEOUT, (void *) 0);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GetFileCRCRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetFileCRCComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetFileCRCComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc, void const * const pResponse)
{
    /* Clear the valid flag since this API is completed */
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Generate a callback with the correct response pointer */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_FILE_CRC, rc, pResponse);
    }
    else
    {
        /* Because return code is not a success, return null as a response */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_FILE_CRC, rc, (void *) 0);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

static void wb_wil_GetFileCRCUpdateCRCList (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, uint32_t iCRC, bool bFileExists)
{
    /* Used to store the position of the response in the response array/map */
    uint8_t iDeviceIndex;

    /* Validate device ID before use */
    if (iDeviceId != 0u)
    {
        /* Check device ID to determine which entry to update */
        if (iDeviceId <= ADI_WIL_DEV_NODE_61)
        {
            iDeviceIndex = 0u;

            /* Seek through bitmap to find the first set bit - that's the index */
            while (((iDeviceId >> iDeviceIndex) & 1ULL) != 1ULL)
            {
                iDeviceIndex++;
            }
        }
        else
        {
            /* Index of this device in the output CRC list */
            iDeviceIndex = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? WB_GET_FILE_CRC_LIST_MANAGER_0_INDEX : WB_GET_FILE_CRC_LIST_MANAGER_1_INDEX;
        }

        /* Update bitmap and store CRC if existing */
        if (bFileExists)
        {
            pInternals->GetFileCRCState.FileCRCList.eFileExists |= (1ULL << iDeviceIndex);
            pInternals->GetFileCRCState.FileCRCList.iCRC [iDeviceIndex] = iCRC;
        }
    }
}

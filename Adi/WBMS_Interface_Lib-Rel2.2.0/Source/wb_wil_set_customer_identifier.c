/******************************************************************************
 * @file    wb_wil_set_customer_identifier.c
 *
 * @brief   Sets the customer identifier data for all devices on the network
 *
 * Copyright(c) 2022 Analog Devices, Inc.All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.and its licensors.
 *****************************************************************************/

#include "wb_wil_set_customer_identifier.h"
#include "wb_wil_ui.h"
#include "wb_wil_request.h"
#include "adi_wil_types.h"
#include "wb_req_set_customer_identifier.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "wb_wil_device.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetCustomerIdentifierFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_SetCustomerIdentifierComplete (adi_wil_pack_internals_t * const pInternals,
                                                  adi_wil_err_t rc);

/******************************************************************************
 * Static variables
 *****************************************************************************/

static const adi_wil_target_t ValidCustomerIdentifierTargets[] = { ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_ALL_NODES };

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetCustomerIdentifierDataAPI (adi_wil_pack_internals_t * const pInternals,
                                                   adi_wil_customer_identifier_t eCustomerIdentifier,
                                                   uint8_t const * const pData,
                                                   uint8_t iLength)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    uint64_t iDeviceId;
    bool bReleaseLock = false;

    /* Used to store a bitmap of all nodes in the ACL */
    uint64_t iNodeMap;

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
            rc = (iLength <= ADI_WIL_MAX_CUSTOMER_IDENTIFIER_DATA_LEN) ? ADI_WIL_ERR_SUCCESS : ADI_WIL_ERR_INVALID_PARAMETER;
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Ensure all nodes are connected before invoking this command */
            if (pInternals->NodeState.iCount <= ADI_WIL_MAX_NODES)
            {
                /* Generate a 64-bit bitmap with n = NodeCount bits set */
                iNodeMap = 0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount);

                /* Check our generated map matches the map of connected nodes */
                if (iNodeMap != pInternals->NodeState.iConnectState)
                {
                    rc = ADI_WIL_ERR_INVALID_STATE;
                }
            }
            /* ...else, we've got an invalid number of nodes. Mark as invalid state */
            else
            {
                rc = ADI_WIL_ERR_INVALID_STATE;
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Skip to updating the managers if no nodes in the ACL */
            iDeviceId = (0u == pInternals->NodeState.iCount) ? ADI_WIL_DEV_ALL_MANAGERS : ADI_WIL_DEV_ALL_NODES;

            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidCustomerIdentifierTargets) / sizeof (adi_wil_target_t)), ValidCustomerIdentifierTargets, &wb_wil_SetCustomerIdentifierFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->CustomerIdentifierState.iCustomerIdentifier = wb_wil_GetUintFromCustomerIdentifier (eCustomerIdentifier);
            pInternals->CustomerIdentifierState.iLength = iLength;
            (void) memcpy (&pInternals->CustomerIdentifierState.Data, pData, iLength);

            /* Send 'set customer ID' request packet */
            wb_wil_SetCustomerIdentifierFunc (pInternals);
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

static void wb_wil_SetCustomerIdentifierFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_customer_identifier_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iLength = pInternals->CustomerIdentifierState.iLength;
    Request.iCustomerIdentifierId = pInternals->CustomerIdentifierState.iCustomerIdentifier;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_SetCustomerIdentifierComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetCustomerIdentifierRequest (pInternals, &Request, &pInternals->CustomerIdentifierState.Data [0], Request.iLength, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_SetCustomerIdentifierComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

void wb_wil_HandleSetCustomerIdentifierResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (wb_wil_api_CheckToken (pInternals, pResponse->iToken, false) != ADI_WIL_ERR_SUCCESS)
    {
        /* Do nothing : there is no active request with this token */
    }
    else if (pResponse->rc != WBMS_CMD_RC_SUCCESS)
    {
        wb_wil_SetCustomerIdentifierComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
    {
        /* If in the node state, setup the request for the manager */
        if (ADI_WIL_TARGET_ALL_NODES == pInternals->UserRequestState.eTarget)
        {
            if (ADI_WIL_ERR_SUCCESS != wb_wil_SetupRequest (pInternals, ADI_WIL_DEV_ALL_MANAGERS, (sizeof (ValidCustomerIdentifierTargets) / sizeof (adi_wil_target_t)), ValidCustomerIdentifierTargets, &wb_wil_SetCustomerIdentifierFunc))
            {
                wb_wil_SetCustomerIdentifierComplete (pInternals, ADI_WIL_ERR_FAIL);
            }
            else
            {
                wb_wil_SetCustomerIdentifierFunc (pInternals);
            }
        }
        else
        /* If in the manager state, notify user of completion */
        {
            wb_wil_SetCustomerIdentifierComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
    else
    {
        /* MISRA else */
    }
}

static void wb_wil_SetCustomerIdentifierComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SET_CUSTOMER_IDENTIFIER_DATA, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

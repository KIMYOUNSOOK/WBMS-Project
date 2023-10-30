/******************************************************************************
 * @file    wb_wil_getacl.c
 *
 * @brief   Get the ACL list from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "wb_wil_get_acl.h"
#include "wb_wil_ui.h"
#include "wb_wil_request.h"
#include "wb_req_get_acl.h"
#include "wb_rsp_get_acl.h"
#include "adi_wil_port.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "adi_wil_pack_internals.h"
#include <string.h>

/******************************************************************************
 * Static Function declarations
 *****************************************************************************/

static void wb_wil_GetACLFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_GetACLComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetACLAPI (adi_wil_pack_internals_t * const pInternals)
{
    uint64_t iDeviceId;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY, ADI_WIL_MODE_ACTIVE, ADI_WIL_MODE_MONITORING, ADI_WIL_MODE_COMMISSIONING, ADI_WIL_MODE_OTAP };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER };
    adi_wil_err_t rc;
    bool bReleaseLock;
   
    /* Initialize local variables */
    iDeviceId = 0ULL;
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
            /* If neither manager is the primary manager, then the system is in
             * an invalid state */
            if (ADI_WIL_MGR_PRIMARY == pInternals->pManager0Port->Internals.Role)
            {
                iDeviceId = ADI_WIL_DEV_MANAGER_0;
            }
            else if ((void *) 0 != pInternals->pManager1Port)
            {
                if (ADI_WIL_MGR_PRIMARY == pInternals->pManager1Port->Internals.Role)
                {
                    iDeviceId = ADI_WIL_DEV_MANAGER_1;
                }
                else
                {
                    rc = ADI_WIL_ERR_INVALID_STATE;
                }
            }
            else
            {
                /* Either manager 0 or manager 1 need to be taking a 'manager
                 * primary role' */
                rc = ADI_WIL_ERR_INVALID_STATE;
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Check for connection status of the primary manager */
            if ((iDeviceId == ADI_WIL_DEV_MANAGER_0) && (!pInternals->pManager0Port->Internals.bConnected))
            {
                rc = ADI_WIL_ERR_NOT_CONNECTED;
            }
            else if ((iDeviceId == ADI_WIL_DEV_MANAGER_1) && (!pInternals->pManager1Port->Internals.bConnected))
            {
                rc = ADI_WIL_ERR_NOT_CONNECTED;
            }
            else
            {
                /* MISRA else */
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetACLFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->ACLState.iCount = 0u;
            /* Send 'get ACL' request packet */
            wb_wil_GetACLFunc (pInternals);
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

static void wb_wil_GetACLFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_get_acl_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iIndex = pInternals->ACLState.iCount;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetACLComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GetACLRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

void wb_wil_HandleGetACLResponse (adi_wil_pack_internals_t * const pInternals, wbms_cmd_resp_get_acl_t const * const pResponse, uint8_t const * const pData)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    /* Manager is busy, so resend request */
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        wb_wil_GetACLFunc (pInternals);
    }
    /* Non-success RC received */
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_GetACLComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    /* If the response entry count exceeds the max allowed per transaction */
    else if (WBMS_MAX_ACL_ENTRIES_PER_REQ < pResponse->iCount)
    {
        wb_wil_GetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    /* If the accumulative number of entries has exceeded the max allowed */
    else if (ADI_WIL_MAX_NODES < (pResponse->iCount + pInternals->ACLState.iCount))
    {
        wb_wil_GetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* Make a copy of the received ACL entries and increment the total ACL
         * entry count */
        (void) memcpy (&pInternals->ACLState.Data [pInternals->ACLState.iCount * ADI_WIL_MAC_ADDR_SIZE], pData, ((size_t) ADI_WIL_MAC_ADDR_SIZE * pResponse->iCount));
        pInternals->ACLState.iCount += pResponse->iCount;

        /* If the response count is 0, it means there are no more entries in
         * the ACL */
        if (pResponse->iCount == 0u)
        {
            wb_wil_GetACLComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
        else
        {
            /* Send a request to fetch the next block of entries */
            /* Increment the token */
            wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);

            /* If we rolled over, increment it again so the token is now non-zero */
            if (pInternals->UserRequestState.iToken == 0u)
            {
                wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);
            }

            pInternals->UserRequestState.iRetries = 0u;
            wb_wil_GetACLFunc (pInternals);
        }
    }
}

static void wb_wil_GetACLComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    /* Update the internal variables as 'get ACL' API has completed */
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* 'Get ACL' has succeeded, generate callback with a valid and
         * populated return structure */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_ACL, rc, &pInternals->ACLState);
    }
    else
    {
        /* Generate callback with NULL data pointer for non-successful RC */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_ACL, rc, (void *) 0);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

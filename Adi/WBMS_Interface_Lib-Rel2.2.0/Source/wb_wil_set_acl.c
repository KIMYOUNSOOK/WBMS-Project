/******************************************************************************
 * @file    wb_wil_set_acl.c
 *
 * @brief   Send SetACL API Command to retrieve ACL list from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_set_acl.h"
#include "wb_wil_ui.h"
#include "wb_wil_request.h"
#include "wb_req_set_acl.h"
#include "wb_req_generic.h"
#include "wb_rsp_generic.h"
#include "adi_wil_port.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "adi_wil_pack_internals.h"
#include "wb_assl.h"
#include <string.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

/* Used to set the maximum number of requests attempts this API can perform */
#define WB_WIL_SET_ACL_REQUEST_LIMIT    (15u)

/* Used to set the maximum timeout for each individual request */
#define WB_WIL_SET_ACL_REQUEST_TIMEOUT  (200u)

/******************************************************************************
 * Static Function declarations
 *****************************************************************************/

static void wb_wil_ClearACLFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_SetACLFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_SetACLComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetACLAPI (adi_wil_pack_internals_t * const pInternals,
                                uint8_t const * const pData,
                                uint8_t iCount)
{
    uint64_t iDeviceId;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER };
    void const * const NullableParams[] = { pData };
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
        if (iCount > pInternals->iMaxNodeCount)
        {
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
        }

        if ((ADI_WIL_ERR_SUCCESS == rc) && (iCount != 0u))
        {
            rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Validate system mode */
            rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);
        }

        /* Make sure that the ACL count is within limits */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = (iCount > ADI_WIL_MAX_NODES) ? ADI_WIL_ERR_INVALID_PARAMETER : ADI_WIL_ERR_SUCCESS;
        }

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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_ClearACLFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->ACLState.iCount = iCount;

            /* Set the request count to 0u at the start of API invocation */
            pInternals->UserRequestState.iRequestCount = 0u;

            if (iCount > 0u)
            {
                (void) memcpy (&pInternals->ACLState.Data [0], pData, ((size_t) ADI_WIL_MAC_ADDR_SIZE * iCount));
            }

            wb_wil_ClearACLFunc (pInternals);
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

static void wb_wil_ClearACLFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    /* Increment the request count each time this method is entered */
    wb_wil_IncrementWithRollover8 (&pInternals->UserRequestState.iRequestCount);

    /* Check that the request count has not exceeded limit */
    if (pInternals->UserRequestState.iRequestCount > WB_WIL_SET_ACL_REQUEST_LIMIT)
    {
        wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_CLEAR_ACL, WB_WIL_SET_ACL_REQUEST_TIMEOUT))
    {
        wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_SetACLFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_acl_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    if ((pInternals->NodeState.iCount + WBMS_MAX_ACL_ENTRIES_PER_REQ) > pInternals->ACLState.iCount)
    {
        Request.iCount = (pInternals->ACLState.iCount % WBMS_MAX_ACL_ENTRIES_PER_REQ);

        if (Request.iCount == 0u)
        {
            Request.iCount = WBMS_MAX_ACL_ENTRIES_PER_REQ;
        }
    }
    else
    {
        Request.iCount = WBMS_MAX_ACL_ENTRIES_PER_REQ;
    }

    /* Increment the request count each time this method is entered */
    wb_wil_IncrementWithRollover8 (&pInternals->UserRequestState.iRequestCount);

    /* Check that the request count has not exceeded limit */
    if (pInternals->UserRequestState.iRequestCount > WB_WIL_SET_ACL_REQUEST_LIMIT)
    {
        wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetACLRequest (pInternals,
                                                          &Request,
                                                          &pInternals->ACLState.Data [pInternals->NodeState.iCount * ADI_WIL_MAC_ADDR_SIZE],
                                                          (uint8_t) (Request.iCount * ADI_WIL_MAC_ADDR_SIZE),
                                                          WB_WIL_SET_ACL_REQUEST_TIMEOUT))
    {
        wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

void wb_wil_HandleClearACLResponse (adi_wil_pack_internals_t * const pInternals, wbms_cmd_resp_generic_t const * const pResponse)
{
    adi_wil_device_t eDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with not a SUCCESS.
         * Complete the 'Set ACL' API with the return code received from the
         * end device */
        wb_wil_SetACLComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        /* Clear internal connected state for all nodes on clear ACL success */
        for (uint8_t i = 0u; i < pInternals->NodeState.iCount; i++)
        {
            /* If node was previously connected generate disconnection event */
            if ( ((pInternals->NodeState.iConnectState >> i) & 1ULL) == 1ULL )
            {
                eDeviceId = 1ULL << i;
                wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_NODE_DISCONNECTED, &eDeviceId);
            }
        }

        pInternals->NodeState.iConnectState = 0ULL;

        pInternals->NodeState.iCount = 0u;

        if (pInternals->ACLState.iCount != 0u)
        {
            /* Increment the token */
            wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);

            /* If we rolled over, increment it again so the token is now non-zero */
            if (pInternals->UserRequestState.iToken == 0u)
            {
                wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);
            }

            pInternals->UserRequestState.iRetries = 0u;
            pInternals->UserRequestState.pfRequestFunc = &wb_wil_SetACLFunc;
            wb_wil_SetACLFunc (pInternals);
        }
        else
        {
            /* End device has responded with a SUCCESS.
             * All the members of ACL have completed sending a request.
             * So complete the 'Set ACL' API with the return code of SUCCESS */
            wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

void wb_wil_HandleSetACLResponse (adi_wil_pack_internals_t * const pInternals, wbms_cmd_resp_generic_t const * const pResponse)
{
    uint8_t iCount; /* Count of entries just submitted */

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        /* End device has responded with a WAIT. So, resend the 'Set ACL'
         * packet */
        wb_wil_SetACLFunc (pInternals);
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with neither a WAIT, nor a SUCCESS.
         * Complete the 'Set ACL' API with the return code received from the
         * end device */
        wb_wil_SetACLComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if ((pInternals->NodeState.iCount + WBMS_MAX_ACL_ENTRIES_PER_REQ) > pInternals->ACLState.iCount)
        {
            iCount = (pInternals->ACLState.iCount % WBMS_MAX_ACL_ENTRIES_PER_REQ);

            if (iCount == 0u)
            {
                iCount = WBMS_MAX_ACL_ENTRIES_PER_REQ;
            }
        }
        else
        {
            iCount = WBMS_MAX_ACL_ENTRIES_PER_REQ;
        }

        /* Check if node count is within limits */
        if ((((uint16_t) pInternals->NodeState.iCount) + ((uint16_t) iCount)) > ((uint16_t) ADI_WIL_MAX_NODES))
        {
            wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
        else
        {
            pInternals->NodeState.iCount += iCount;

            if (pInternals->NodeState.iCount < pInternals->ACLState.iCount)
            {
                /* Still 'Set ACL' request packet needs to be sent out (for the
                 * remaining node(s)) */
                pInternals->UserRequestState.iRetries = 0u;

                /* Increment the token */
                wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);

                /* If we rolled over, increment it again so the token is now non-zero */
                if (pInternals->UserRequestState.iToken == 0u)
                {
                    wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);
                }

                wb_wil_SetACLFunc (pInternals);
            }
            else
            {
                /* All the nodes have been sent the 'Set ACL' request packet.
                 * Complete the 'Set ACL' API with a return code of SUCCESS */
                wb_wil_SetACLComplete (pInternals, ADI_WIL_ERR_SUCCESS);
            }
        }
    }
}

static void wb_wil_SetACLComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    /* Refresh the XMS parameters if we have a valid number of nodes */
    if (pInternals->NodeState.iCount <= ADI_WIL_MAX_NODES)
    {
        /* Generate a 64-bit bitmap with n = NodeCount bits set */
        pInternals->XmsMeasurementParameters.iBMSDevices = 0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount);

        /* Supply new bitmap to XMS module */
        wb_assl_SetMeasurementParameters(pInternals->pPack, &pInternals->XmsMeasurementParameters);
    }

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SET_ACL, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}


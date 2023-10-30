/******************************************************************************
 * @file    wb_wil_rotate_key.c
 *
 * @brief   Rotate the  key on one or all nodes
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_rotate_key.h"
#include "wb_wil_request.h"
#include "wb_req_rotate_key.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_RotateKeyFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_RotateKeyComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_RotateKeyAPI (adi_wil_pack_internals_t * const pInternals,
                                   adi_wil_key_t eKeyType,
                                   uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER };
    uint64_t iManagerDevice;
    bool bReleaseLock = false;

    /* Validate input parameters */
    if ((eKeyType == ADI_WIL_KEY_SESSION) || (eKeyType == ADI_WIL_KEY_NETWORK))
    {
        rc = ADI_WIL_ERR_NOT_SUPPORTED;
    }
    else
    {
        rc = adi_wil_ValidateInstance (pInternals, true);
    }

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Check if Device ID is in range*/
            if ((iDeviceId == 0ULL) ||
                (iDeviceId > ADI_WIL_DEV_NODE_61))
            {
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
            }
            /* Check if the Device ID is only for a single node device */
            else if ((iDeviceId & (iDeviceId - 1u)) != 0u)
            {
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
            }
            /* Check node is connected before issuing rotate key */
            else if ((iDeviceId & pInternals->NodeState.iConnectState) == 0ULL)
            {
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
            }
            else
            {
                /* Init 8-bit Device ID state variable to 0 */
                pInternals->RotateKeyState.iDeviceId = 0u;

                /* Seek through bitmap to find the first set bit - that's the index */
                while (((iDeviceId >> pInternals->RotateKeyState.iDeviceId) & 1ULL) != 1ULL)
                {
                    pInternals->RotateKeyState.iDeviceId++;
                }
            }
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Default to manager 0 if manager 1 does not exist */
            if ((void *) 0 == pInternals->pManager1Port)
            {
                iManagerDevice = ADI_WIL_DEV_MANAGER_0;
            }
            else
            {
                iManagerDevice = (ADI_WIL_MGR_PRIMARY == pInternals->pManager0Port->Internals.Role) ? ADI_WIL_DEV_MANAGER_0 : ADI_WIL_DEV_MANAGER_1;
            }

            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iManagerDevice, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_RotateKeyFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send Rotate Key request packet */
            wb_wil_RotateKeyFunc (pInternals);
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

void wb_wil_HandleRotateKeyResponse (adi_wil_pack_internals_t * const pInternals, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        wb_wil_RotateKeyComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_RotateKeyFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_rotate_key_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iDeviceId = pInternals->RotateKeyState.iDeviceId;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_RotateKeyComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_RotateKeyRequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_RotateKeyComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_RotateKeyComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_ROTATE_KEY, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

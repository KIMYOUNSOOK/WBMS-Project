/******************************************************************************
 * @file    wb_wil_update_monitor_parameters.c
 *
 * @brief   Updates the monitor parameters on one or more devices
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_ui.h"
#include "wb_wil_update_monitor_params.h"
#include "wb_wil_request.h"
#include "wb_req_generic.h"
#include "wb_req_set_mon_params_data.h"
#include "wb_rsp_get_mon_params_crc.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_utils.h"
#include "wb_crc_16.h"
#include "wb_crc_32.h"
#include "wb_crc_config.h"
#include "wb_wil_api.h"
#include <string.h>


/******************************************************************************
 * #defines
 *****************************************************************************/

/* Used to set the maximum number of requests attempts this API can perform */
#define WB_WIL_SET_MONITOR_PARAMS_REQUEST_LIMIT    (15u)

/* Used to set the maximum timeout for each individual request */
#define WB_WIL_SET_MONITOR_PARAMS_REQUEST_TIMEOUT  (200u)

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_UpdateMonitorParametersDataFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_UpdateMonitorParametersCommitFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_UpdateMonitorParametersComplete (adi_wil_pack_internals_t * const pInternals,
                                                    adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_UpdateMonitorParametersAPI (adi_wil_pack_internals_t * const pInternals,
                                                 uint64_t iDeviceId,
                                                 uint8_t * const pData,
                                                 uint16_t iLength)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_NODES };
    void const * const NullableParams[] = { pData };
    bool bReleaseLock = false;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Validate input length */
    if (iLength == 0u)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }

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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_UpdateMonitorParametersDataFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->UpdateMonParamsState.iLength = iLength;
            pInternals->UpdateMonParamsState.pData = pData;
            pInternals->UpdateMonParamsState.iOffset = 0u;

            /* Store expected response mask as this is a multi-stage API */
            pInternals->UpdateMonParamsState.iUpdateMap = pInternals->UserRequestState.iPendingResponses;

            /* Set the request count to 0u at the start of API invocation */
            pInternals->UserRequestState.iRequestCount = 0u;

            wb_wil_UpdateMonitorParametersDataFunc (pInternals);
        }
        else
        {
            /* Release lock if any of the above steps returns a failure */
            bReleaseLock = true;
        }
    }

    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    return rc;
}

void wb_wil_HandleUpdateMonParamDataResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with not a SUCCESS.
         * Complete the 'Update monitor parameter' API with the return code
         * received from the end device */
        wb_wil_UpdateMonitorParametersComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            pInternals->UserRequestState.iRetries = 0u;

            /* Restore original device map */
            pInternals->UserRequestState.iPendingResponses = pInternals->UpdateMonParamsState.iUpdateMap;

            if ((pInternals->UpdateMonParamsState.iOffset + pInternals->UpdateMonParamsState.iPreviousRequestLength) > (uint16_t) UINT16_MAX)
            {
                pInternals->UpdateMonParamsState.iOffset = (uint16_t) UINT16_MAX;
            }
            else
            {
                pInternals->UpdateMonParamsState.iOffset += pInternals->UpdateMonParamsState.iPreviousRequestLength;
            }

            /* Send 'Update monitor parameters' request packet */
            if (pInternals->UpdateMonParamsState.iOffset >= pInternals->UpdateMonParamsState.iLength)
            {
                pInternals->UserRequestState.pfRequestFunc = &wb_wil_UpdateMonitorParametersCommitFunc;
                wb_wil_UpdateMonitorParametersCommitFunc (pInternals);
            }
            else
            {
                wb_wil_UpdateMonitorParametersDataFunc (pInternals);
            }
        }
    }
}

void wb_wil_HandleUpdateMonParamCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with not a SUCCESS.
         * Complete the 'Update monitor parameters' API with the return code
         * received from the end device */
        wb_wil_UpdateMonitorParametersComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else
    {
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            /* If all devices have been serviced, complete API with return code
             * as SUCCESS */
            wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_UpdateMonitorParametersDataFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_mon_params_data_t Request;
    uint16_t iLength;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    /* CERT-C Guard against subtraction underflow */
    if (pInternals->UpdateMonParamsState.iLength < pInternals->UpdateMonParamsState.iOffset)
    {
        wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        iLength = pInternals->UpdateMonParamsState.iLength - pInternals->UpdateMonParamsState.iOffset;

        if (iLength > WBMS_CMD_REQ_SET_MON_PARAMS_PAYLOAD_MAX_LEN)
        {
            iLength = WBMS_CMD_REQ_SET_MON_PARAMS_PAYLOAD_MAX_LEN;
        }

        Request.iOffset = pInternals->UpdateMonParamsState.iOffset;
        Request.iLength = (uint8_t) (iLength & (uint16_t) 0x00FF);
        Request.iCRC = wb_crc_ComputeCRC16 (&pInternals->UpdateMonParamsState.pData [Request.iOffset], Request.iLength, WB_CRC_SEED);

        pInternals->UpdateMonParamsState.iPreviousRequestLength = Request.iLength;

        /* Increment the request count each time this method is entered */
        wb_wil_IncrementWithRollover8 (&pInternals->UserRequestState.iRequestCount);

        /* Check that the request count has not exceeded limit */
        if (pInternals->UserRequestState.iRequestCount > WB_WIL_SET_MONITOR_PARAMS_REQUEST_LIMIT)
        {
            wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
        }
        else if (ADI_WIL_ERR_SUCCESS != wb_wil_UpdateMonitorParametersDataRequest (pInternals, &Request, &pInternals->UpdateMonParamsState.pData [Request.iOffset], Request.iLength, WB_WIL_SET_MONITOR_PARAMS_REQUEST_TIMEOUT))
        {
            wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
        else
        {
            /* MISRA Else */
        }
    }
}

static void wb_wil_UpdateMonitorParametersCommitFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    /* Increment the request count each time this method is entered */
    wb_wil_IncrementWithRollover8 (&pInternals->UserRequestState.iRequestCount);

    /* Check that the request count has not exceeded limit */
    if (pInternals->UserRequestState.iRequestCount > WB_WIL_SET_MONITOR_PARAMS_REQUEST_LIMIT)
    {
        wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals,
                                                           &Request,
                                                           WBMS_CMD_SET_MON_PARAMS_COMMIT,
                                                           WB_WIL_SET_MONITOR_PARAMS_REQUEST_TIMEOUT))
    {
        wb_wil_UpdateMonitorParametersComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_UpdateMonitorParametersComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    uint32_t iCRC;

    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    if (ADI_WIL_ERR_SUCCESS != rc)
    {
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_UPDATE_MONITOR_PARAMETERS, rc, (void *) 0);
    }
    else
    {
        /* Compute 32-bit CRC over entire data for user to store */
        iCRC = wb_crc_ComputeCRC32 (pInternals->UpdateMonParamsState.pData, pInternals->UpdateMonParamsState.iLength, WB_CRC_SEED);
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_UPDATE_MONITOR_PARAMETERS, rc, &iCRC);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

/******************************************************************************
 * @file    wb_wil_set_contextual.c
 *
 * @brief   Set the Contextual Data on a Manager or Node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_version.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_request.h"
#include "wb_rsp_get_version.h"
#include "wb_req_generic.h"
#include "wb_wil_ui.h"
#include "wb_wil_version.h"
#include "adi_wil_version.h"
#include "adi_wil_dev_version.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetVersionFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_GetVersionComplete (adi_wil_pack_internals_t * const pInternals,
                                       adi_wil_err_t rc,
                                       void const * const pResponse);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetWILSoftwareVersionAPI (adi_wil_version_t * const pVersion)
{
    adi_wil_err_t rc;
    void const * const NullableParams[] = { pVersion };

    /* Validate input parameter */
    rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Populate return structure with WIL version numbers */
        pVersion->iVersionMajor = WIL_VERSION_MAJOR;
        pVersion->iVersionMinor = WIL_VERSION_MINOR;
        pVersion->iVersionPatch = WIL_VERSION_PATCH;
        pVersion->iVersionBuild = WIL_VERSION_BUILD;
    }

    return rc;
}

adi_wil_err_t wb_wil_GetDeviceVersionAPI (adi_wil_pack_internals_t * const pInternals,
                                          uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY, ADI_WIL_MODE_OTAP };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE };
    bool bReleaseLock = false;

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
            /* Set up the correct bit mask */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetVersionFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Send 'get version' request packet */
            wb_wil_GetVersionFunc (pInternals);
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

void wb_wil_HandleGetVersionResponse (adi_wil_pack_internals_t * const pInternals,
                                      uint64_t iDeviceId,
                                      wbms_cmd_resp_get_version_t const * const pResponse)
{
    adi_wil_dev_version_t UserResponse;

    (void) iDeviceId;

    /* Initialize request structure */
    (void) memset (&UserResponse, 0, sizeof (UserResponse));

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* End device has responded with not a SUCCESS.
         * Complete the 'Get Version' API with the return code received from
         * the end device */
        wb_wil_GetVersionComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc), (void *) 0);
    }
    else
    {
        /* End device has responded with a SUCCESS. Populate the
         * received version numbers in the return structure. Complete the 'Get
         * Version' API with the return code of SUCCESS */
        UserResponse.MainProcSWVersion.iVersionMajor = pResponse->iVersionMajor;
        UserResponse.MainProcSWVersion.iVersionMinor = pResponse->iVersionMinor;
        UserResponse.MainProcSWVersion.iVersionPatch = pResponse->iVersionPatch;
        UserResponse.MainProcSWVersion.iVersionBuild = pResponse->iVersionBuild;
        UserResponse.iMainProcSiVersion = pResponse->iSiliconVersion;
        UserResponse.CoProcSWVersion.iVersionMajor = pResponse->iCPVersionMajor;
        UserResponse.CoProcSWVersion.iVersionMinor = pResponse->iCPVersionMinor;
        UserResponse.CoProcSWVersion.iVersionPatch = pResponse->iCPVersionPatch;
        UserResponse.CoProcSWVersion.iVersionBuild = pResponse->iCPVersionBuild;
        UserResponse.iCoProcSiVersion = pResponse->iCPSiliconVersion;
        UserResponse.iLifeCycleInfo = pResponse->iLifeCycleInfo;

        wb_wil_GetVersionComplete (pInternals, ADI_WIL_ERR_SUCCESS, &UserResponse);
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetVersionFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetVersionComplete (pInternals, ADI_WIL_ERR_TIMEOUT, (void *) 0);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_GET_VERSION, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetVersionComplete (pInternals, ADI_WIL_ERR_FAIL, (void *) 0);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetVersionComplete (adi_wil_pack_internals_t * const pInternals,
                                       adi_wil_err_t rc,
                                       void const * const pResponse)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* End device has responded with a SUCCESS.
         * Generate a callback to the host device with the return code of
         * SUCCESS and with the return structure that is populated with version
         * numbers */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_DEVICE_VERSION, rc, pResponse);
    }
    else
    {
        /* End device has responded with not a SUCCESS.
         * Generate a callback to the host device with the return code received
         * from the end device */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_DEVICE_VERSION, rc, (void *) 0);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

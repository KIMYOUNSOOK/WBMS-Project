/******************************************************************************
 * @file    wb_wil_get_configuration.c
 *
 * @brief   Get configuration from a network manager
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_query_device.h"
#include "adi_wil_port.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_hal_ticker.h"
#include "wb_wil_request.h"
#include "wb_req_generic.h"
#include "wb_rsp_query_device.h"
#include "adi_wil_configuration_data.h"
#include "wb_wil_ui.h"
#include "wb_nil.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"

#include <string.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

#define ADI_WIL_QUERY_DEVICE_TIMER_JITTER_MAX (5u)
#define ADI_WIL_QUERY_DEVICE_TIMEOUT_MS       (100u)
#define ADI_WIL_QUERY_DEVICE_RETRIES          (30u)

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_QueryDeviceFunc (adi_wil_port_t * const pPort);
static void wb_wil_QueryDeviceComplete (adi_wil_port_t * const pPort, adi_wil_err_t rc, void const * pResponse);

/******************************************************************************
* Public functions
*****************************************************************************/

adi_wil_err_t wb_wil_QueryDeviceAPI (adi_wil_port_t * const pPort)
{
    adi_wil_err_t rc;
    adi_wil_pack_t * pPortLockId;
    bool bPortLockAcquired = false;

    (void) memset (&pPortLockId, 0xFF, sizeof (void *));

    if (NULL == pPort)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        pPort->Internals.QueryDeviceState.bPackLockAcquired = false;

        /* Acquire lock for the port */
        rc = wb_wil_ui_AcquireLock (NULL, pPortLockId);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            bPortLockAcquired = true;

            /* Check to see if the port is associated with a pack and if so
             * acquire the pack's lock as well, lock out any pack level
             * operation when QueryDevice is in progress */
            if (pPort->Internals.pPackInternals != NULL)
            {
                rc = wb_wil_ui_AcquireLock (pPort->Internals.pPackInternals->pPack, pPort->Internals.pPackInternals->pPack);

                /* If the pack lock returns API_IN_PROGRESS, then some pack
                 * level API is in progress, discontinue and release the
                 * current port level lock. If the RC is success or
                 * NOT_CONNECTED (which means no connect call has taken place,
                 * so pack level locks don't yet exist), then we proceed with
                 * QueryDevice */
                if (ADI_WIL_ERR_API_IN_PROGRESS != rc)
                {
                    pPort->Internals.QueryDeviceState.bPackLockAcquired = true;
                    rc = ADI_WIL_ERR_SUCCESS;
                }
            }
        }
        /* This condition can only occur if adi_wil_Initialize hasn't been
         * called or successful */
        else if (ADI_WIL_ERR_NOT_CONNECTED == rc)
        {
            rc = ADI_WIL_ERR_FAIL;
        }
        else
        {
            /* MISRA else */
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pPort->Internals.QueryDeviceState.bDeviceExists = wb_nil_CheckForPort (pPort);

            /* If device not already initialized, initialize now */
            if (!pPort->Internals.QueryDeviceState.bDeviceExists)
            {
                rc = wb_nil_InitPort (NULL, pPort);
            }

            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                pPort->Internals.QueryDeviceState.bInProgress = true;
                pPort->Internals.QueryDeviceState.iRetries = 0u;

                wb_wil_QueryDeviceFunc (pPort);
            }
        }

        /* Check the result from the previous operation. If it's non success
         * and it's not in progress, then something else went wrong, so we
         * release all the locks */
        if ((ADI_WIL_ERR_SUCCESS != rc) &&
            (ADI_WIL_ERR_API_IN_PROGRESS != rc))
        {
            /* Check we acquired the pack level lock previously */
            if (pPort->Internals.QueryDeviceState.bPackLockAcquired && 
                (NULL != pPort->Internals.pPackInternals))
            {
                wb_wil_ui_ReleaseLock (pPort->Internals.pPackInternals->pPack, pPort->Internals.pPackInternals->pPack);
            }

            /* Release port level lock */
            if (bPortLockAcquired)
            {
                wb_wil_ui_ReleaseLock (NULL, pPortLockId);
            }
        }
    }

    return rc;
}

void wb_wil_HandleQueryDeviceResponse (adi_wil_port_t * const pPort, wbms_cmd_resp_query_device_t const * const pResponse)
{
    adi_wil_configuration_t UserResponse;

    (void) memset (&UserResponse, 0, sizeof (UserResponse));

    if (NULL == pPort)
    {
        /* Do nothing - invalid parameter */
    }
    else if (!pPort->Internals.QueryDeviceState.bInProgress)
    {
        /* Ignore : QueryDevice not in progress */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        wb_wil_QueryDeviceComplete (pPort, wb_wil_GetErrFromUint (pResponse->rc), NULL);
    }
    /* Ensure sensor packet count won't overflow before assigning */
    else if ((uint16_t) UINT16_MAX < (((uint16_t) pResponse->iMaxBMSPacketsPerNode * pResponse->iMaxNodeCount) +
                                      ((uint16_t) pResponse->iMaxPMSPackets * pResponse->iPMSEnabledManagers) +
                                      pResponse->iMaxEnvironmentalPackets))
    {
        wb_wil_QueryDeviceComplete (pPort, ADI_WIL_ERR_INVALID_PARAMETER, (void *) 0);
    }
    else
    {
        /* Populate the response structure to be returned from this API */
        (void) memcpy (&UserResponse.MAC [0], &pResponse->MAC [0], sizeof (pResponse->MAC));
        (void) memcpy (&UserResponse.PeerMAC [0], &pResponse->PeerMAC [0], sizeof (pResponse->PeerMAC));
        (void) memcpy (&UserResponse.Nonce [0], &pResponse->Nonce [0], sizeof (UserResponse.Nonce));

        UserResponse.bEncryptionEnabled = (pResponse->iEncryptionEnabledFlag == 1u);
        UserResponse.bDualConfiguration = (pResponse->isStandalone != 1u);
        UserResponse.MainProcSWVersion.iVersionMajor = pResponse->iVersionMajor;
        UserResponse.MainProcSWVersion.iVersionMinor = pResponse->iVersionMinor;
        UserResponse.MainProcSWVersion.iVersionPatch = pResponse->iVersionPatch;
        UserResponse.MainProcSWVersion.iVersionBuild = pResponse->iVersionBuild;
        UserResponse.iConfigurationHash = pResponse->iConfigurationHash;


        UserResponse.iSensorPacketCount = ((uint16_t) pResponse->iMaxBMSPacketsPerNode * pResponse->iMaxNodeCount) +
                                          ((uint16_t) pResponse->iMaxPMSPackets * pResponse->iPMSEnabledManagers) +
                                          pResponse->iMaxEnvironmentalPackets;

        /* Completed API - return data to user */
        wb_wil_QueryDeviceComplete (pPort, ADI_WIL_ERR_SUCCESS, &UserResponse);
    }
}

void wb_wil_QueryDeviceCheckTimeout (adi_wil_port_t * const pPort)
{
    uint32_t iCurrentTime;
    uint32_t iElapsedTime;

    if (NULL != pPort)
    {
        /* If we're currently executing a query device command... */
        if (pPort->Internals.QueryDeviceState.bInProgress)
        {
            iCurrentTime = adi_wil_hal_TickerGetTimestamp ();
            iElapsedTime = (iCurrentTime - pPort->Internals.QueryDeviceState.iTimeStamp);

            if (iElapsedTime < iCurrentTime)
            {
                /* CERT-C - Allow rollover as timers are expected to wrap */
            }

            /* Check if the job has elapsed,
             * ...but don't trigger a timeout if we've potentially underflowed
             * by 5ms to allow for timer jitter */
            if ((iElapsedTime > (uint32_t)ADI_WIL_QUERY_DEVICE_TIMEOUT_MS) &&
                (iElapsedTime < (UINT32_MAX - ADI_WIL_QUERY_DEVICE_TIMER_JITTER_MAX)))
            {
                wb_wil_IncrementWithRollover8 (&pPort->Internals.QueryDeviceState.iRetries);
                wb_wil_QueryDeviceFunc (pPort);
            }
        }
    }
}

/******************************************************************************
* Static functions
*****************************************************************************/

static void wb_wil_QueryDeviceFunc (adi_wil_port_t * const pPort)
{
    wbms_cmd_req_generic_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iToken = 0u;
    pPort->Internals.QueryDeviceState.iTimeStamp = adi_wil_hal_TickerGetTimestamp ();

    if (pPort->Internals.QueryDeviceState.iRetries >= ADI_WIL_QUERY_DEVICE_RETRIES)
    {
        wb_wil_QueryDeviceComplete (pPort, ADI_WIL_ERR_TIMEOUT, NULL);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_QueryDeviceRequest (pPort, &Request))
    {
        wb_wil_QueryDeviceComplete (pPort, ADI_WIL_ERR_FAIL, NULL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_QueryDeviceComplete (adi_wil_port_t * const pPort, adi_wil_err_t rc, void const * pResponse)
{
    adi_wil_pack_t * pPortLockId;

    (void) memset (&pPortLockId, 0xFF, sizeof (pPortLockId));

    /* Disconnect if port wasn't connected before QueryDevice API
     * invoked */
    if (!pPort->Internals.QueryDeviceState.bDeviceExists)
    {
        (void) wb_nil_ClosePort (pPort);
    }

    pPort->Internals.QueryDeviceState.bInProgress = false;

    wb_wil_ui_GeneratePortCb (pPort, ADI_WIL_API_QUERY_DEVICE, rc, pResponse);

    /* Release pack level lock */
    if ((pPort->Internals.QueryDeviceState.bPackLockAcquired) &&
        (NULL != pPort->Internals.pPackInternals))
    {
        wb_wil_ui_ReleaseLock (pPort->Internals.pPackInternals->pPack, pPort->Internals.pPackInternals->pPack);
    }

    /* Release port level lock */
    wb_wil_ui_ReleaseLock (NULL, pPortLockId);
}

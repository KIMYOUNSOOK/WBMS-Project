/******************************************************************************
 * @file    wb_wil_connect.c
 *
 * @brief   Connect to network managers in a pack
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_connect.h"
#include "wb_wil_request.h"
#include "wb_nil.h"
#include "wb_req_connect.h"
#include "wb_rsp_connect.h"
#include "adi_wil_port.h"
#include "wb_wil_initialize.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_xms_parameters.h"
#include "wb_wil_ui.h"
#include "wb_assl.h"
#include "adi_wil_connection_details.h"

#include <string.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

#define ADI_WIL_PRIMARY_MGR_ID          (1u)
#define ADI_WIL_SECONDARY_MGR_ID        (2u)
#define ADI_WIL_CONNECT_TIMEOUT_MS      (100u)
#define ADI_WIL_CONNECT_RETRIES         (29u)
#define ADI_WIL_CONNECT_TIMER_JITTER_MAX (5u)
#define ADI_WIL_MGR_CONFIG_PROT_BASE_VER (5u)

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_ProcessConnectResponse (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId,
                                           wbms_cmd_resp_connect_t const * const pResponse);

static void wb_wil_ConnectFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_ConnectComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);
static adi_wil_mode_t wb_wil_GetWILModeFromUint (uint8_t uint_mode);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ConnectAPI (adi_wil_pack_t const * const pPack,
                                 adi_wil_pack_internals_t * const pInternals,
                                 adi_wil_port_t * const pManager0Port,
                                 adi_wil_port_t * const pManager1Port)
{
    adi_wil_err_t rc;

    rc = adi_wil_InitializePack (pPack, pInternals, pManager0Port, pManager1Port);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        pInternals->ConnectState.bReconnection = false;
        pInternals->ConnectState.bInProgress = true;
        pInternals->ConnectState.iRetries = 0u;
        pInternals->ConnectState.bDMHActive = false;
        wb_wil_IncrementWithRollover16 (&pInternals->ConnectState.iToken);
        /* Check for the retry count and the connect state of the manager(s) */
        wb_wil_ConnectFunc (pInternals);
    }

    return rc;
}

void adi_wil_Reconnect (adi_wil_pack_internals_t * const pInternals)
{
    if (!pInternals->ConnectState.bInProgress)
    {
        /* Reset ReconnectPending flag as reconnecting would be taken
         * care here */
        pInternals->bReconnectPending = false;
        pInternals->ConnectState.bReconnection = true;
        pInternals->ConnectState.bInProgress = true;
        pInternals->ConnectState.iRetries = 0u;
        pInternals->ConnectState.bDMHActive = false;
        wb_wil_IncrementWithRollover16 (&pInternals->ConnectState.iToken);
        /* Check for the retry count and the connect state of the manager(s) */
        wb_wil_ConnectFunc (pInternals);
    }
}

static void wb_wil_ConnectFunc (adi_wil_pack_internals_t * const pInternals)
{
    /* If connect has been retried for more than the max count, abort connect
     * API with return code = TIMEOUT */
    if (pInternals->ConnectState.iRetries >= ADI_WIL_CONNECT_RETRIES)
    {
        wb_wil_ConnectComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if ((pInternals->pManager0Port != (void *) 0) &&
             (pInternals->pManager0Port->Internals.bConnected) &&
             (pInternals->pManager1Port != (void *) 0) &&
             (pInternals->pManager1Port->Internals.bConnected))
    {
        /* If manager 0 and manager 1 both are connected, then return from
         * connect API with return code = SUCCESS */
        wb_wil_ConnectComplete (pInternals, ADI_WIL_ERR_SUCCESS);
    }
    else
    {
        /* If neither/only one manager is connected, update internal
         * connection pending variable */
        pInternals->ConnectState.iTimeStamp = adi_wil_hal_TickerGetTimestamp ();

        if (pInternals->pManager0Port != (void *) 0)
        {
            if (!pInternals->pManager0Port->Internals.bConnected)
            {
                pInternals->pManager0Port->Internals.bConnectionRequestPending = true;
            }
        }

        if (pInternals->pManager1Port != (void *) 0)
        {
            if (!pInternals->pManager1Port->Internals.bConnected)
            {
                pInternals->pManager1Port->Internals.bConnectionRequestPending = true;
            }
        }
    }
}

void wb_wil_TriggerConnectRequest (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * pPort)
{
    wbms_cmd_req_connect_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iToken = pInternals->ConnectState.iToken;
    
    /* Use WBMS SPI API version of Device ID (8-bit) */
    Request.iDeviceId = (pPort == pInternals->pManager0Port) ? WBMS_MANAGER_0_DEVICE_ID : WBMS_MANAGER_1_DEVICE_ID;

    /* Send connect request packet */
    (void) wb_wil_ConnectRequest (pPort, &Request);

    pPort->Internals.bConnectionRequestPending = false;
}


void wb_wil_CheckConnectionTimeout (adi_wil_pack_internals_t * const pInternals,
                                    uint32_t iCurrentTime)
{
    uint32_t iElapsedTime;

    if (pInternals != (void *) 0)
    {
        if (pInternals->ConnectState.bInProgress)
        {
            iElapsedTime = (iCurrentTime - pInternals->ConnectState.iTimeStamp);

            if (iElapsedTime < iCurrentTime)
            {
                /* CERT-C - Allow rollover as timers are expected to wrap */
            }

            /* Check if the job has elapsed,
             * ...but don't trigger a timeout if we've potentially underflowed
             * by 5ms to allow for timer jitter */
            if ((iElapsedTime > (uint32_t)ADI_WIL_CONNECT_TIMEOUT_MS) &&
                (iElapsedTime < (UINT32_MAX - ADI_WIL_CONNECT_TIMER_JITTER_MAX)))
            {
                wb_wil_IncrementWithRollover8 (&pInternals->ConnectState.iRetries);
                wb_wil_ConnectFunc (pInternals);
            }
        }
    }
}

void wb_wil_HandleConnectResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_connect_t const * const pResponse)
{
    bool bComplete;

    bComplete = true;

    if ((!pInternals->ConnectState.bInProgress) || (pResponse->iToken != pInternals->ConnectState.iToken))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        /* Prevent a NULL Port pointer being used in
         * wb_wil_ProcessConnectResponse() */
        if (((iDeviceId == ADI_WIL_DEV_MANAGER_0) && (pInternals->pManager0Port != (void *) 0)) ||
            ((iDeviceId == ADI_WIL_DEV_MANAGER_1) && (pInternals->pManager1Port != (void *) 0)))
        {
            wb_wil_ProcessConnectResponse (pInternals, iDeviceId, pResponse);
        }

        /* Check completed on manager 0 */
        if (pInternals->pManager0Port != (void *) 0)
        {
            /* If we're not connected or our role is unknown and it's
             * not our last attempt, mark as not complete */
            if ((!pInternals->pManager0Port->Internals.bConnected) ||
                ((pInternals->pManager0Port->Internals.bConnected) &&
                 (pInternals->pManager0Port->Internals.Role == ADI_WIL_MGR_UNKNOWN) &&
                 (pInternals->ConnectState.iRetries < (ADI_WIL_CONNECT_RETRIES - 1u))))
            {
                bComplete = false;
            }
        }

        /* Check completed on manager 1 */
        if (bComplete &&
            (pInternals->pManager1Port != (void *) 0))
        {
            /* If we're not connected or our role is unknown and it's not
             * our last attempt, mark as not complete */
            if ((!pInternals->pManager1Port->Internals.bConnected) ||
                ((pInternals->pManager1Port->Internals.bConnected) &&
                 (pInternals->pManager1Port->Internals.Role == ADI_WIL_MGR_UNKNOWN) &&
                 (pInternals->ConnectState.iRetries < (ADI_WIL_CONNECT_RETRIES - 1u))))
            {
                bComplete = false;
            }
        }

        if (bComplete)
        {
            wb_wil_ConnectComplete (pInternals, ADI_WIL_ERR_SUCCESS);
        }
    }
}

static void wb_wil_ProcessConnectResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_connect_t const * const pResponse)
{
    adi_wil_port_t * pPort;
    uint8_t iBytePos;
    uint8_t iBitPos;

    /* Login only if the role has been established. Otherwise if by the
     * time max number of retries has already been reached, we should login
     * and declare connected so reduced functionality is possible */
    if ((WBMS_CMD_RC_SUCCESS == pResponse->rc) ||
        ((pResponse->rc != WBMS_CMD_RC_FAILED) && (pInternals->ConnectState.iRetries >= (ADI_WIL_CONNECT_RETRIES - 1u))))
    {
        if (iDeviceId == ADI_WIL_DEV_MANAGER_0)
        {
            pPort = pInternals->pManager0Port;
            pInternals->ConnectState.iManager0ConfigurationHash = pResponse->iConfigurationHash;
        }
        else
        {
            pPort = pInternals->pManager1Port;
            pInternals->ConnectState.iManager1ConfigurationHash = pResponse->iConfigurationHash;
        }

        if (pResponse->iManagerNumber == ADI_WIL_PRIMARY_MGR_ID)
        {
            pPort->Internals.Role = ADI_WIL_MGR_PRIMARY;
            pInternals->eMode = wb_wil_GetWILModeFromUint (pResponse->iMode);
            pInternals->NodeState.iCount = (pResponse->iNodeCount <= ADI_WIL_MAX_NODES) ? pResponse->iNodeCount : ADI_WIL_MAX_NODES;
            pInternals->iMaxNodeCount = (pResponse->iMaxNodeCount <= ADI_WIL_MAX_NODES) ? pResponse->iMaxNodeCount : ADI_WIL_MAX_NODES;

            (void) memset (&pInternals->XmsMeasurementParameters, 0, sizeof (pInternals->XmsMeasurementParameters));

            /* Refresh the XMS parameters if we have a valid number of nodes */
            /* Ensure the node count is in range... */
            if ((pInternals->NodeState.iCount <= ADI_WIL_MAX_NODES) &&
                (pInternals->NodeState.iCount != 0ULL))
            {
                /* Generate a 64-bit bitmap with n = NodeCount bits set */
                pInternals->XmsMeasurementParameters.iBMSDevices = 0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount);
            }

            if (pResponse->iPMSEnabledManagers != 0u)
            {
                pInternals->XmsMeasurementParameters.iPMSDevices = (pResponse->iPMSEnabledManagers == 1u) ? ADI_WIL_DEV_MANAGER_0 : ADI_WIL_DEV_ALL_MANAGERS;
            }

            /* Initially assume the sender to be manager 0. If this is
             * incorrect, it will be modified in nil_response.c upon receipt
             * of data from another source */
            if (pResponse->iMaxEnvironmentalPackets > 0u)
            {
                pInternals->XmsMeasurementParameters.iEMSDevices = ADI_WIL_DEV_MANAGER_0;
            }

            pInternals->XmsMeasurementParameters.iBMSPackets = pResponse->iMaxBMSPacketsPerNode;
            pInternals->XmsMeasurementParameters.iPMSPackets = pResponse->iMaxPMSPackets;
            pInternals->XmsMeasurementParameters.iEMSPackets = pResponse->iMaxEnvironmentalPackets;

            pInternals->NodeState.iConnectState = 0ULL;

            for (uint8_t i = 0u; i < pResponse->iNodeCount; i++)
            {
                iBytePos = (uint8_t) (i / 8u);
                iBitPos = (i % 8u);

                if ((pResponse->iNodeStatusMask [iBytePos] & (1u << iBitPos)) > 0u)
                {
                    pInternals->NodeState.iConnectState |= (1ULL << i);
                }
            }
        }
        else if (pResponse->iManagerNumber == ADI_WIL_SECONDARY_MGR_ID)
        {
            pPort->Internals.Role = ADI_WIL_MGR_SECONDARY;

            /* Default to manager 0 if manager 1 does not exist */
            if ((void *) 0 == pInternals->pManager1Port)
            {
                /* Make sure mode has a valid value in an event of a
                 * mis-configured manager, so that config file download
                 * is possible */
                pInternals->eMode = wb_wil_GetWILModeFromUint (pResponse->iMode);
            }
        }
        else
        {
            pPort->Internals.Role = ADI_WIL_MGR_UNKNOWN;
        }

        /* Extract bits 0..3 for the SPI protocol version */
        pPort->Internals.iProtocolVersion = pResponse->iProtocolVersion & (uint8_t) 0x0Fu;

        /* If we haven't already had a connection with DMH marked as active,
         * check this response */
        if (!pInternals->ConnectState.bDMHActive)
        {
            /* Extract bit 6 for the DMH active/inactive flag*/
            pInternals->ConnectState.bDMHActive = ((pResponse->iProtocolVersion & (1u << 6u)) != 0u);
        }

        /* Extract bit 7 for the dual manager flag and check for
         * "dual manager" response */
        if (pPort->Internals.iProtocolVersion >= ADI_WIL_MGR_CONFIG_PROT_BASE_VER)
        {
            if ((((pResponse->iProtocolVersion & ((uint32_t) 1u << 7u)) >> 7u) & (uint8_t) 0xffu) == WBMS_MANAGER_DUAL)
            {
                /* Dual manager : Check second manager port is not enabled */
                if ((void *) 0 == pInternals->pManager1Port)
                {
                    /* If it is, flag this connection as invalid */
                    pInternals->bInvalidState = true;
                }
            }

            /* Note: a separate if statement is used here for
             * MISRA/CERT compliance */
            if ((((pResponse->iProtocolVersion & ((uint32_t) 1u << 7u)) >> 7u) & (uint8_t) 0xffu) == WBMS_MANAGER_SINGLE)
            {
                /* Single manager: Check second manager port is not disabled */
                if ((void *) 0 != pInternals->pManager1Port)
                {
                    /* If it is, flag this connection as invalid */
                    pInternals->bInvalidState = true;
                }
            }
        }

        /* If known role or the last attempt, login */
        if ((pPort->Internals.Role != ADI_WIL_MGR_UNKNOWN) ||
            (pInternals->ConnectState.iRetries >= (ADI_WIL_CONNECT_RETRIES - 1u)))
        {
            (void) wb_nil_Login (pPort, pResponse->iSessionId);
            pPort->Internals.bLinkAvailable = true;

            /* If the response wasn't success, flag for invalid state */
            if (pResponse->rc != WBMS_CMD_RC_SUCCESS)
            {
                pInternals->bInvalidState = true;
            }
        }
    }
}

/* Different cases can occur during connection:
   For single manager,
   if rc == ADI_WIL_ERR_INVALID_STATE, it can be due to:
   1. Incorrect total number of sensor buffers;
   2. Manager role is not ADI_WIL_MGR_PRIMARY;
   3. The manager indicates it's in single manager configuration, yet two port
      objects are provided to the connect function.
   if rc == ADI_WIL_ERR_TIMEOUT, the manager didn't respond.
   if rc == ADI_WIL_ERR_INVALID_PARAMETER, the user provided buffer count is
   less than the required number by the configuration.

   For dual manager,
   if rc == ADI_WIL_INVALID_STATE, it can be due to:
   1. Incorrect total number of sensor buffers;
   2. Both manager roles are the same;
   3. Either manager's role is not established (i.e. ADI_WIL_MGR_UNKNOWN);
   4. One or both managers returned a non successful error code after the max
      number of retries;
   5. Only one port object is provided to the connect function for a dual
      manager configuration.
   if one or both of the managers did not connect (i.e. timed out),
   ADI_WIL_ERR_TIMEOUT is returned.
   if rc == ADI_WIL_ERR_CONFIGURATION_MISMATCH, this means the hashes from
   both managers don't match;
   if rc == ADI_WIL_ERR_INVALID_PARAMETER, the user provided buffer count is
   less than the required number by the configuration.


   In any cases where the RC is not ADI_WIL_ERR_SUCCESS, a hash of 0
   is returned.
 */

static void wb_wil_ConnectComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    adi_wil_err_t UserRC;
    adi_wil_connection_details_t ConnectionDetails;
    adi_wil_device_t eTempDeviceId;

    UserRC = rc;

    /* Initialize response structure values based on the known manager 0
     * state */
    ConnectionDetails.iHash = pInternals->ConnectState.iManager0ConfigurationHash;

    /* Populate DMH Active status from ConnectState boolean */
    ConnectionDetails.bDMHActive = pInternals->ConnectState.bDMHActive;

    if (ADI_WIL_ERR_SUCCESS != UserRC)
    {
        ConnectionDetails.iHash = 0u;
    }
    else if ((pInternals->pManager0Port != (void *) 0) && (pInternals->pManager1Port != (void *) 0))
    {
        if (pInternals->pManager0Port->Internals.iProtocolVersion != pInternals->pManager1Port->Internals.iProtocolVersion)
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        else if (pInternals->bInvalidState)
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        /* Check for one primary manager */
        else if ((pInternals->pManager0Port->Internals.Role != ADI_WIL_MGR_PRIMARY) &&
                 (pInternals->pManager1Port->Internals.Role != ADI_WIL_MGR_PRIMARY))
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        /* Check for one secondary manager */
        else if ((pInternals->pManager0Port->Internals.Role != ADI_WIL_MGR_SECONDARY) &&
                 (pInternals->pManager1Port->Internals.Role != ADI_WIL_MGR_SECONDARY))
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        else if (pInternals->ConnectState.iManager0ConfigurationHash != pInternals->ConnectState.iManager1ConfigurationHash)
        {
            UserRC = ADI_WIL_ERR_CONFIGURATION_MISMATCH;
            ConnectionDetails.iHash = 0u;
        }
        else
        {
            /*  Propagate original RC */
        }
    }
    else /* Else : Single manager */
    {
        if (pInternals->bInvalidState)
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        else if ((void *) 0 == pInternals->pManager0Port)
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        else if (pInternals->pManager0Port->Internals.Role != ADI_WIL_MGR_PRIMARY)
        {
            UserRC = ADI_WIL_ERR_INVALID_STATE;
        }
        else
        {
            /* Propagate original RC */
        }
    }

    /* Make sure bInvalidState flag is set if the error condition is invalid state. */
    pInternals->bInvalidState = (UserRC == ADI_WIL_ERR_INVALID_STATE);

    if ((UserRC == ADI_WIL_ERR_SUCCESS) || (UserRC == ADI_WIL_ERR_INVALID_STATE) || (UserRC == ADI_WIL_ERR_CONFIGURATION_MISMATCH) || (UserRC == ADI_WIL_ERR_INVALID_PARAMETER))
    {
        pInternals->bMaintainConnection = true;
    }

    /* Allocate and initialize the XMS buffers if we're in a valid state */
    if (ADI_WIL_ERR_SUCCESS == UserRC)
    {
        wb_assl_InitializeAllocation (pInternals->pPack,
                                      ((uint16_t) pInternals->XmsMeasurementParameters.iPMSPackets * ((pInternals->XmsMeasurementParameters.iPMSDevices == ADI_WIL_DEV_MANAGER_0) ? 1u : 2u)),
                                      (pInternals->XmsMeasurementParameters.iEMSPackets));

        wb_assl_SetMeasurementParameters(pInternals->pPack, &pInternals->XmsMeasurementParameters);
    }

    pInternals->ConnectState.bInProgress = false;

    if (!pInternals->ConnectState.bReconnection)
    {
        if (UserRC == ADI_WIL_ERR_SUCCESS)
        {
            wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_CONNECT, UserRC, &ConnectionDetails);
        }
        else
        {
            wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_CONNECT, UserRC, (void *) 0);
        }

        /* Release lock */
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    /* Trigger events after API callback to clear any potential race conditions
     * if the user is unlocking based on manager connect notifications */
    if ((void *) 0 != pInternals->pManager0Port)
    {
        if (pInternals->pManager0Port->Internals.bConnected)
        {
            eTempDeviceId = ADI_WIL_DEV_MANAGER_0;
            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_MGR_CONNECTED, &eTempDeviceId);
        }
    }

    if ((void *) 0 != pInternals->pManager1Port)
    {
        if (pInternals->pManager1Port->Internals.bConnected)
        {
            eTempDeviceId = ADI_WIL_DEV_MANAGER_1;
            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_MGR_CONNECTED, &eTempDeviceId);
        }
    }
}

static adi_wil_mode_t wb_wil_GetWILModeFromUint (uint8_t uint_mode)
{
    /* This code supports 6 modes (standby, active, commission, OTAP,
     * monitor, sleep */
    adi_wil_mode_t eMode;

    /* Map mode from type uint8_t to type adi_wil_mode_t */
    if (WBMS_MODE_STANDBY == uint_mode)
    {
        eMode = ADI_WIL_MODE_STANDBY;
    }
    else if (WBMS_MODE_ACTIVE == uint_mode)
    {
        eMode = ADI_WIL_MODE_ACTIVE;
    }
    else if (WBMS_MODE_COMMISSIONING == uint_mode)
    {
        eMode = ADI_WIL_MODE_COMMISSIONING;
    }
    else if (WBMS_MODE_OTAP == uint_mode)
    {
        eMode = ADI_WIL_MODE_OTAP;
    }
    else if (WBMS_MODE_SLEEP == uint_mode)
    {
        eMode = ADI_WIL_MODE_SLEEP;
    }
    else
    {
        /* If none of the above modes match, then mode must be
         * in MONITORING mode */
        eMode = ADI_WIL_MODE_MONITORING;
    }

    return eMode;
}

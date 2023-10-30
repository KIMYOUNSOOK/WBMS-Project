/******************************************************************************
 * @file    wb_wil_dmh.c
 *
 * @brief   Contains definitions for DMH related APIs and handlers
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_dmh.h"
#include "wb_wil_request.h"
#include "wb_req_generic.h"
#include "wb_req_dmh_apply.h"
#include "wb_rsp_generic.h"
#include "wb_ntf_dmh_apply.h"
#include "wb_ntf_dmh_assess.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "wb_wil_ack.h"
#include "adi_wil_topology_assessment.h"

#include <string.h>

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static void wb_wil_AssessNetworkTopologyFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_ApplyNetworkTopologyFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_AssessNetworkTopologyComplete (adi_wil_pack_internals_t * const pInternals,
                                                  adi_wil_err_t rc);

static void wb_wil_ApplyNetworkTopologyComplete (adi_wil_pack_internals_t * const pInternals,
                                                 adi_wil_err_t rc);

static adi_wil_err_t wb_wil_ValidatePrimaryManagerConnection (adi_wil_pack_internals_t const * const pInternals,
                                                              uint64_t * const pDeviceId);

static uint8_t wb_wil_TopologyToUint (adi_wil_topology_t eTopology);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_AssessNetworkTopologyAPI (adi_wil_pack_internals_t * const pInternals)
{
    /* Storage for intended recipient of request (primary manager) */
    uint64_t iDeviceId;

    /* List of valid system modes this API can be invoked in */
    const adi_wil_mode_t ValidModes [] = { ADI_WIL_MODE_ACTIVE };

    /* List of valid device targets this request can be sent to */
    const adi_wil_target_t ValidTargets [] = { ADI_WIL_TARGET_SINGLE_MANAGER };

    /* Storage for whether the lock was acquired and needs to be released */
    bool bReleaseLock;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Initialize local variables */
    iDeviceId = 0ULL;
    bReleaseLock = false;

    /* Validate the instance */
    rc = adi_wil_ValidateInstance (pInternals,
                                   true);

    /* If instance valid, acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack,
                                    pInternals->pPack);
    }

    /* If lock acquired, continue setup... */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Check system mode is correct... */
        rc = adi_wil_CheckSystemMode (pInternals,
                                      (sizeof (ValidModes) / sizeof (adi_wil_mode_t)),
                                      ValidModes);

        /* If system mode correct, continue setup */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Check primary manager is connected... */
            rc = wb_wil_ValidatePrimaryManagerConnection (pInternals,
                                                          &iDeviceId);
        }

        /* If primary manager is connected, attempt to set up the request */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = wb_wil_SetupRequest (pInternals,
                                      iDeviceId,
                                      (sizeof (ValidTargets) / sizeof (adi_wil_target_t)),
                                      ValidTargets,
                                      &wb_wil_AssessNetworkTopologyFunc);
        }

        /* If the request was able to be set up and buffered, invoke the sender
         * method */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            wb_wil_AssessNetworkTopologyFunc (pInternals);
        }

        /* Flag lock release if any of the setup steps failed */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;
        }
    }

    /* If lock was acquired, release it before returning */
    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack,
                               pInternals->pPack);
    }

    /* Return rc to application */
    return rc;
}

adi_wil_err_t wb_wil_ApplyNetworkTopologyAPI (adi_wil_pack_internals_t * const pInternals,
                                              adi_wil_topology_t eTopology)
{
    /* Storage for intended recipient of request (primary manager) */
    uint64_t iDeviceId;

    /* List of valid system modes this API can be invoked in */
    const adi_wil_mode_t ValidModes [] = { ADI_WIL_MODE_STANDBY };

    /* List of valid device targets this request can be sent to */
    const adi_wil_target_t ValidTargets [] = { ADI_WIL_TARGET_SINGLE_MANAGER };

    /* Storage for whether the lock was acquired and needs to be released */
    bool bReleaseLock;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Initialize local parameters*/
    iDeviceId = 0ULL;
    bReleaseLock = false;

    /* Validate the instance */
    rc = adi_wil_ValidateInstance (pInternals,
                                   true);

    /* If instance valid, acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack,
                                    pInternals->pPack);
    }

    /* If lock acquired, continue setup... */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Check system mode is correct... */
        rc = adi_wil_CheckSystemMode (pInternals,
                                      (sizeof (ValidModes) / sizeof (adi_wil_mode_t)),
                                      ValidModes);

        /* If system mode correct, continue setup */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Check primary manager is connected... */
            rc = wb_wil_ValidatePrimaryManagerConnection (pInternals,
                                                          &iDeviceId);
        }

        /* If primary manager is connected, attempt to set up the request */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = wb_wil_SetupRequest (pInternals,
                                      iDeviceId,
                                      (sizeof (ValidTargets) / sizeof (adi_wil_target_t)),
                                      ValidTargets,
                                      &wb_wil_ApplyNetworkTopologyFunc);
        }

        /* If the request was able to be set up and buffered, invoke the sender
         * method */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Store the eTopology parameter in the ApplyNetworkTopology
             * state */
            pInternals->ApplyNetworkTopologyState.eTopology = eTopology;

            wb_wil_ApplyNetworkTopologyFunc (pInternals);
        }

        /* Flag lock release if any of the setup steps failed */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;
        }
    }

    /* If lock was acquired, release it before returning */
    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack,
                               pInternals->pPack);
    }

    /* Return rc to application */
    return rc;
}

void wb_wil_HandleAssessNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals,
                                                 wbms_cmd_resp_generic_t const * const pResponse)
{
    /* Ensure the response token matches the current user request */
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals,
                                                      pResponse->iToken,
                                                      true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        /* Complete the API with the RC from the response */
        wb_wil_AssessNetworkTopologyComplete (pInternals,
                                              wb_wil_GetErrFromUint (pResponse->rc));
    }
}

void wb_wil_HandleApplyNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals,
                                                wbms_cmd_resp_generic_t const * const pResponse)
{
    /* Ensure the response token matches the current user request */
    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals,
                                                      pResponse->iToken,
                                                      true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        /* Complete the API with the RC from the response */
        wb_wil_ApplyNetworkTopologyComplete (pInternals,
                                             wb_wil_GetErrFromUint (pResponse->rc));

    }
}

void wb_wil_HandleAssessTopologyNotif (adi_wil_pack_internals_t const * const pInternals,
                                       adi_wil_port_t * const pPort,
                                       wbms_notif_dmh_assess_t const * const pNotif)
{
    /* Storage for event data */
    adi_wil_topology_assessment_t TopologyAssessment;

    /* Validate input parameters */
    if (((void *) 0 != pInternals) &&
        ((void *) 0 != pPort) &&
        ((void *) 0 != pNotif))
    {
        /* Populate event data structure */
        (void) memcpy (&TopologyAssessment.iRssiDeltas [0],
                       &pNotif->iRssiDeltas [0],
                       sizeof (TopologyAssessment.iRssiDeltas));

        TopologyAssessment.iSignalFloorImprovement = pNotif->iSignalFloorImprovement;
        TopologyAssessment.rc = wb_wil_GetErrFromUint (pNotif->rc);

        /* Generate the event */
        wb_wil_ui_GenerateEvent (pInternals->pPack,
                                 ADI_WIL_EVENT_TOPOLOGY_ASSESSMENT_COMPLETE,
                                 &TopologyAssessment);

        /* Only send an acknowledgment if notification id is non-zero */
        if (pNotif->iNotifId != 0u)
        {
            /* Add acknowledgment to queue */
            if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue,
                                                       pNotif->iNotifId,
                                                       WBMS_NOTIF_DMH_ASSESS))
            {
                /* Increment internal statistic if we failed to queue
                 * acknowledgment */
                wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
            }
        }
    }
}

void wb_wil_HandleApplyTopologyNotif (adi_wil_pack_internals_t const * const pInternals,
                                      adi_wil_port_t * const pPort,
                                      wbms_notif_dmh_apply_t const * const pNotif)
{
    /* Storage for event data */
    adi_wil_err_t rc;

    /* Validate input parameters */
    if (((void *) 0 != pInternals) &&
        ((void *) 0 != pPort) &&
        ((void *) 0 != pNotif))
    {
        /* Populate event data structure */
        rc = wb_wil_GetErrFromUint (pNotif->rc);

        /* Generate the event */
        wb_wil_ui_GenerateEvent (pInternals->pPack,
                                 ADI_WIL_EVENT_TOPOLOGY_APPLICATION_COMPLETE,
                                 &rc);

        /* Only send an acknowledgment if notification id is non-zero */
        if (pNotif->iNotifId != 0u)
        {
            /* Add acknowledgment to queue */
            if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue,
                                                       pNotif->iNotifId,
                                                       WBMS_NOTIF_DMH_APPLY))
            {
                /* Increment internal statistic if we failed to queue
                 * acknowledgment */
                wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
            }
        }
    }
}


/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_AssessNetworkTopologyFunc (adi_wil_pack_internals_t * const pInternals)
{
    /* Request structure instance memory */
    wbms_cmd_req_generic_t Request = { 0 };

    /* Ensure we have not exceeded reties */
    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_AssessNetworkTopologyComplete (pInternals,
                                              ADI_WIL_ERR_TIMEOUT);
    }
    /* Ensure we were able to write the request to the buffer */
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals,
                                                           &Request,
                                                           WBMS_CMD_DMH_ASSESS,
                                                           ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_AssessNetworkTopologyComplete (pInternals,
                                              ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_ApplyNetworkTopologyFunc (adi_wil_pack_internals_t * const pInternals)
{
    /* Request structure instance memory */
    wbms_cmd_req_dmh_apply_t Request = { 0 };

    /* Set parameters of the request from state variable */
    Request.iType = wb_wil_TopologyToUint (pInternals->ApplyNetworkTopologyState.eTopology);

    /* Ensure we have not exceeded reties */
    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_ApplyNetworkTopologyComplete (pInternals,
                                             ADI_WIL_ERR_TIMEOUT);
    }
    /* Ensure we were able to write the request to the buffer */
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_ApplyNetworkTopologyRequest (pInternals,
                                                                        &Request,
                                                                        ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_ApplyNetworkTopologyComplete (pInternals,
                                             ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_AssessNetworkTopologyComplete (adi_wil_pack_internals_t * const pInternals,
                                                  adi_wil_err_t rc)
{
    /* Invalidate request state */
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    /* Generate callback */
    wb_wil_ui_GenerateCb (pInternals->pPack,
                          ADI_WIL_API_ASSESS_NETWORK_TOPOLOGY,
                          rc,
                          (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack,
                           pInternals->pPack);
}

static void wb_wil_ApplyNetworkTopologyComplete (adi_wil_pack_internals_t * const pInternals,
                                                 adi_wil_err_t rc)
{
    /* Invalidate request state */
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    /* Generate callback */
    wb_wil_ui_GenerateCb (pInternals->pPack,
                          ADI_WIL_API_APPLY_NETWORK_TOPOLOGY,
                          rc,
                          (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack,
                           pInternals->pPack);
}

static adi_wil_err_t wb_wil_ValidatePrimaryManagerConnection (adi_wil_pack_internals_t const * const pInternals,
                                                              uint64_t * const pDeviceId)
{
    /* Return value of this function*/
    adi_wil_err_t rc;

    /* Initialize local variable */
    rc = ADI_WIL_ERR_SUCCESS;

    /* Check if manager 0 is the primary manager... */
    if (ADI_WIL_MGR_PRIMARY == pInternals->pManager0Port->Internals.Role)
    {
        *pDeviceId = ADI_WIL_DEV_MANAGER_0;

        /* Check for connection status */
        if (!pInternals->pManager0Port->Internals.bConnected)
        {
            rc = ADI_WIL_ERR_NOT_CONNECTED;
        }
    }
    /* else, check if manager 1 exists before dereferencing ... */
    else if ((void *) 0 != pInternals->pManager1Port)
    {
        /* Check if manager 1 is the primary manager... */
        if (ADI_WIL_MGR_PRIMARY == pInternals->pManager1Port->Internals.Role)
        {
            *pDeviceId = ADI_WIL_DEV_MANAGER_1;

            /* Check for connection status */
            if (!pInternals->pManager1Port->Internals.bConnected)
            {
                rc = ADI_WIL_ERR_NOT_CONNECTED;
            }
        }
        else
        {
            /* If neither manager is the primary manager, then the system
             * is in an invalid state */
            rc = ADI_WIL_ERR_INVALID_STATE;
        }
    }
    else
    {
        /* If manager 0 isn't the primary manager and manager 1 doesn't
         * exist, then the system is in an invalid state */
        rc = ADI_WIL_ERR_INVALID_STATE;
    }

    return rc;
}

static uint8_t wb_wil_TopologyToUint (adi_wil_topology_t eTopology)
{
    /* Return value of this function */
    uint8_t iValue;

    /* Set iValue of ADI_WIL_TOPOLOGY_SAVED to WBMS_DMH_APPLY_SAVED */
    if (ADI_WIL_TOPOLOGY_SAVED == eTopology)
    {
        iValue = WBMS_DMH_APPLY_SAVED;
    }
    /* Set iValue of ADI_WIL_TOPOLOGY_NEW to WBMS_DMH_APPLY_NEW */
    else if (ADI_WIL_TOPOLOGY_NEW == eTopology)
    {
        iValue = WBMS_DMH_APPLY_NEW;
    }
    /* ...else eTopology is ADI_WIL_TOPOLOGY_STAR, set to
     * WBMS_DMH_APPLY_STAR */
    else
    {
        iValue = WBMS_DMH_APPLY_STAR;
    }

    /* Return the uint representation */
    return iValue;
}

/******************************************************************************
 * @file    wb_assl.c
 *
 * @brief   WIL FE Application Specific Safety Layer
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_assl_fusa.h"
#include "wb_assl.h"
#include "wb_scl_fusa.h"
#include "wb_wil_msg_header.h"
#include "wb_scl_msg_defs.h"
#include "wb_xms_fusa.h"
#include "wb_wil_configure_cell_balancing.h"
#include "wb_wil_get_cell_balancing_status.h"
#include "wb_wil_ui_fusa.h"
#include "wb_wil_device.h"
#include "adi_wil_pack.h"
#include "adi_wil_assl_internals.h"
#include "adi_wil_fault_safety_cpu.h"
#include "adi_wil_connect_safety_cpu.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_xms_parameters.h"
#include "adi_wil_cell_balancing_status.h"
#include <string.h>

/******************************************************************************
*   Defines
******************************************************************************/

#define WB_SCL_SAFETY_CPU_SUCCESS_RC               (0u)
#define WB_ASSL_TIMER_JITTER_TOLERANCE             (2u)
#define WB_ASSL_NODE_DISCONNECTION_TIMEOUT         (300u)
#define WB_ASSL_NODE_ALIVE_TIMEOUT                 (1000u)

/******************************************************************************
 *   Static function declarations
 *****************************************************************************/

static void wb_assl_CheckForDisconnectedNodes (adi_wil_safety_internals_t * const pInternals,
                                               uint32_t iCurrentTicks);

static void wb_assl_CheckForAPITimeout (adi_wil_safety_internals_t * const pInternals,
                                        uint32_t iCurrentTicks);

static void wb_assl_ReactivateRequest (adi_wil_safety_internals_t * const pInternals,
                                       uint32_t iCurrentTicks);

static void wb_assl_TimeoutRequest (adi_wil_safety_internals_t * const pInternals);

static void wb_assl_CheckForHeartbeatOffState (adi_wil_safety_internals_t * const pInternals,
                                               uint32_t iCurrentTicks);

static void wb_assl_CheckForHeartbeatTimeout (adi_wil_safety_internals_t * const pInternals,
                                              uint32_t iCurrentTicks);

static void wb_assl_HandleFuSaMeasurement (adi_wil_safety_internals_t * const pInternals,
                                           wb_msg_header_t * const pMsgHeader,
                                           adi_wil_xms_measurement_cmd_id_t eCmdId,
                                           uint8_t const * const pData);

static void wb_assl_HandleSafetyConnectResponse (adi_wil_safety_internals_t * const pInternals,
                                                 wb_msg_header_t const * const pMsgHeader,
                                                 uint8_t const * const pData);

static void wb_assl_ParseConnectResponse (uint8_t const * const pData,
                                          adi_wil_connect_safety_cpu_t * const pConnectResponse);

static void wb_assl_HandleSensorCommandResponse (adi_wil_safety_internals_t * const pInternals,
                                                 wb_msg_header_t const * const pMsgHeader,
                                                 uint8_t const * const pData);

static void wb_assl_HandleConfigureCellBalancingResponse (adi_wil_safety_internals_t * const pInternals,
                                                          wb_msg_header_t const * const pMsgHeader,
                                                          uint8_t const * const pData);

static void wb_assl_HandleGetCellBalancingStatusResponse (adi_wil_safety_internals_t * const pInternals,
                                                          wb_msg_header_t const * const pMsgHeader,
                                                          uint8_t const * const pData);

static void wb_assl_HandleSafetyFaultMessage (adi_wil_safety_internals_t const * const pInternals,
                                              wb_msg_header_t const * const pMsgHeader,
                                              uint8_t const * const pData);

static bool wb_assl_WriteHeartbeatMessage (adi_wil_assl_internals_t * const pASSLInternals);

static void wb_assl_WriteHeartbeatRequest (uint64_t iConnectedMask,
                                           uint8_t * const pBuffer);

static void wb_assl_WriteConfigureCellBalancingRequest (adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                        uint16_t iDuration,
                                                        uint32_t iUVThreshold,
                                                        uint8_t * const pBuffer);

static void wb_assl_WriteGetCellBalancingStatusRequest (uint8_t * const pBuffer);

static void wb_assl_UpdateNodeMask (uint64_t * const pNodeMask,
                                    uint8_t iDeviceId,
                                    bool bEnabled);

static bool wb_assl_CheckNodeConnected (adi_wil_assl_internals_t const * const pASSLInternals,
                                        uint8_t iDeviceId);

static void wb_assl_UpdateLastMessageReceivedTime (adi_wil_assl_internals_t * const pASSLInternals,
                                                   uint8_t iDeviceId,
                                                   uint32_t iCurrentTicks);

static bool wb_assl_CheckForTimeout (uint32_t iStartTime,
                                     uint32_t iCurrentTime,
                                     uint32_t iTimeoutValue);

static void wb_assl_IncrementWithRollover8 (uint8_t * pValue);

static adi_wil_safety_internals_t * wb_assl_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack);

/******************************************************************************
 *   Public functions
 *****************************************************************************/

void wb_assl_Process (adi_wil_safety_internals_t * const pInternals,
                      uint32_t iCurrentTicks)
{
    /* Validate input pointer */
    if (pInternals != (void *) 0)
    {
        /* Check if message not received from a node exceeds limit */
        wb_assl_CheckForDisconnectedNodes (pInternals, iCurrentTicks);

        /* Process API request timeout */
        wb_assl_CheckForAPITimeout (pInternals, iCurrentTicks);
        
        /* Check if we should stop sending heartbeat messages */
        wb_assl_CheckForHeartbeatOffState (pInternals, iCurrentTicks);

        /* Check if it is time to send a heartbeat message */
        wb_assl_CheckForHeartbeatTimeout (pInternals, iCurrentTicks);

        /* Invoke XMS method to check for timeouts */
        (void) wb_xms_CheckTimeouts (pInternals, iCurrentTicks);
    }
}

adi_wil_err_t wb_assl_Initialize (adi_wil_pack_t const * const pPack,
                                  adi_wil_sensor_data_t * const pDataBuffer,
                                  uint16_t iDataBufferCount)
{
    /* Return error code */
    adi_wil_err_t rc;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_assl_GetSafetyInternalsPointer (pPack);

    /* Check input parameters */
    if ((pInternals == (void *) 0) || (pDataBuffer == (void *) 0))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Initialize XMS measurement parameters */
        (void) memset (&pInternals->XMSParameters, 0, sizeof (adi_wil_xms_parameters_t));
        (void) memset (&pInternals->FuSaXMSParameters, 0, sizeof (adi_wil_xms_parameters_t));

        /* Initialize XMS Storage module */
        rc = wb_xms_Initialize (pInternals, pDataBuffer, iDataBufferCount);
    }

    if (rc == ADI_WIL_ERR_SUCCESS)
    {
        /* Initialize ASSL struct elements to known initial values */
        (void) memset (&pInternals->ASSL, 0, sizeof (adi_wil_assl_internals_t));

        /* Store pointer to pack instance in the internals struct */
        pInternals->pPack = pPack;
    }

    return rc;
}

adi_wil_err_t wb_assl_WriteConfigureCellBalancingFrame (adi_wil_safety_internals_t * const pInternals,
                                                        uint8_t iDeviceId,
                                                        adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                        uint16_t iDuration,
                                                        uint32_t iUVThreshold)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* SCL header parameters */
    wb_msg_header_t MsgHeader;

    /* Validate input pointers */
    /* Check if the device ID belongs to a valid node */
    if ((pInternals == (void *) 0) || (pDischargeDutyCycle == (void *) 0) || (iDeviceId >= ADI_WIL_MAX_NODES))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Attempt to write the request message */
    else
    {
        /* SCL header parameters */
        MsgHeader.iSourceDeviceId = iDeviceId;
        MsgHeader.iSequenceNumber = pInternals->ASSL.iRequestSequenceNumber[iDeviceId];
        MsgHeader.iMessageType = WB_SCL_MSG_TYPE_SENSOR_COMMAND;
        MsgHeader.iPayloadLength = WB_SCL_CELL_BALANCING_REQUEST_LEN;

        /* Zero out the request frame before writing the request */
        (void) memset (&pInternals->ASSL.UserRequestMessage [0], 0,
                       sizeof (pInternals->ASSL.UserRequestMessage));

        /* Write the cell balancing request into the user request frame */
        wb_assl_WriteConfigureCellBalancingRequest (pDischargeDutyCycle,
                                                    iDuration,
                                                    iUVThreshold,
                                                    &pInternals->ASSL.UserRequestMessage [WB_SCL_MSG_HDR_LEN]);

       /* Wrap the request to the Safety CPU with the SCL header and CRC */
       if (!wb_scl_WrapSCLFrame (&MsgHeader, &pInternals->ASSL.UserRequestMessage [0u]))
        {
            /* Could not write request */
            rc = ADI_WIL_ERR_FAIL;
        }
        else
        {
            /* Store device ID parameter */
            pInternals->ASSL.UserRequestState.iDeviceId = iDeviceId;

            /* Mark request as valid */
            pInternals->ASSL.UserRequestState.bActive = true;

            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    return rc;
}

adi_wil_err_t wb_assl_WriteGetCellBalancingStatusFrame (adi_wil_safety_internals_t * const pInternals)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* SCL header parameters */
    wb_msg_header_t MsgHeader;

    /* Validate input pointers */
    /* Check if the device ID belongs to a valid node */
    if ((void *) 0 == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Attempt to write the request message */
    else
    {
        /* SCL header parameters */
        MsgHeader.iSourceDeviceId = WB_SCL_ALL_NODES_DEVICE_ID;
        MsgHeader.iSequenceNumber = pInternals->ASSL.iBroadcastRequestSequenceNumber;
        MsgHeader.iMessageType = WB_SCL_MSG_TYPE_SENSOR_COMMAND;
        MsgHeader.iPayloadLength = WB_SCL_GET_CELL_BALANCING_STATUS_REQUEST_LEN;

        /* Zero out the request frame before writing the request */
        (void) memset (&pInternals->ASSL.UserRequestMessage [0], 0,
                       sizeof (pInternals->ASSL.UserRequestMessage));

        /* Write the cell balancing status request into the frame */
        wb_assl_WriteGetCellBalancingStatusRequest (&pInternals->ASSL.UserRequestMessage [WB_SCL_MSG_HDR_LEN]);

        /* Wrap the request to the Safety CPU with the SCL header and CRC */
        if (!wb_scl_WrapSCLFrame (&MsgHeader,
                                  &pInternals->ASSL.UserRequestMessage [0u]))
        {
            /* Could not write request */
            rc = ADI_WIL_ERR_FAIL;
        }
        else
        {
            /* Store destination parameter for transmission by the WIL */
            pInternals->ASSL.UserRequestState.iDeviceId = WB_WIL_DEV_ALL_NODES;

            /* Mark request as valid */
            pInternals->ASSL.UserRequestState.bActive = true;

            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    return rc;
}

adi_wil_err_t wb_assl_ActivateRequest (adi_wil_safety_internals_t * const pInternals,
                                       adi_wil_api_t eAPI,
                                       uint8_t iRetries,
                                       uint32_t iRetryInterval)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input parameters */
    if (pInternals == (void *) 0)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Check if there is a valid request to activate */
    else if (!pInternals->ASSL.UserRequestState.bActive)
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else
    {
        /* Store user request parameters */
        pInternals->ASSL.UserRequestState.eAPI = eAPI;
        pInternals->ASSL.UserRequestState.iRemainingRetries = iRetries;
        pInternals->ASSL.UserRequestState.iRetryInterval = iRetryInterval;

        /* Note down start time */
        pInternals->ASSL.UserRequestState.iStartTime = adi_wil_hal_TickerGetTimestamp ();

        /* Update state variables to indicate that the frame is ready for
         * transmission */
        pInternals->ASSL.bUserRequestMessageTxReady = true;

        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

void wb_assl_DeactivateRequest (adi_wil_safety_internals_t * const pInternals)
{
    /* Validate input parameters */
    /* Check if there is a valid request to activate */
    if (pInternals != (void *) 0)
    {
        /* Update state variables to indicate that the request has been
         * deactivated */
        pInternals->ASSL.UserRequestState.bActive = false;
        pInternals->ASSL.bUserRequestMessageTxReady = false;

        /* Check whether it was a unicast or broadcast request and increment
         * the appropriate sequence number */
        if (pInternals->ASSL.UserRequestState.iDeviceId < ADI_WIL_MAX_NODES)
        {
            /* Increment sequence number for the next API request */
            wb_assl_IncrementWithRollover8 (&pInternals->ASSL.iRequestSequenceNumber[pInternals->ASSL.UserRequestState.iDeviceId]);
        }
        else if (pInternals->ASSL.UserRequestState.iDeviceId == WB_WIL_DEV_ALL_NODES)
        {
            /* Increment sequence number for the next API request */
            wb_assl_IncrementWithRollover8 (&pInternals->ASSL.iBroadcastRequestSequenceNumber);
        }
        else
        {
            /* do nothing */
        }
    }
}

void wb_assl_HandleValidatedSCLFrame (adi_wil_safety_internals_t * const pInternals,
                                      wb_msg_header_t * const pMsgHeader,
                                      uint8_t const * const pData)
{
    /* Valid input pointers */
    if ((pInternals != (void *) 0) && (pMsgHeader != (void *) 0))
    {
        /* Update the last packet received timestamp for nodes */
        wb_assl_UpdateLastMessageReceivedTime (&pInternals->ASSL, pMsgHeader->iSourceDeviceId, adi_wil_hal_TickerGetTimestamp());

        /* Process the input frame based on the message type */
        switch (pMsgHeader->iMessageType)
        {
            case WB_SCL_MSG_TYPE_CONNECT:
                /* Handle connect response from Safety CPU */
                wb_assl_HandleSafetyConnectResponse (pInternals, pMsgHeader, pData);
                break;

            case WB_SCL_MSG_TYPE_MEASUREMENT_DATA_START:
                /* Start XMS measurement data received */
                wb_assl_HandleFuSaMeasurement (pInternals, pMsgHeader, ADI_WIL_XMS_START_MEASUREMENT, pData);
                break;

            case WB_SCL_MSG_TYPE_MEASUREMENT_DATA_SUPPLEMENTAL:
                /* Supplemental XMS measurement data received */
                wb_assl_HandleFuSaMeasurement (pInternals, pMsgHeader, ADI_WIL_XMS_SUPPLEMENTAL_MEASUREMENT, pData);
                break;

            case WB_SCL_MSG_TYPE_SENSOR_COMMAND:
                /* Response to sensor command received */
                wb_assl_HandleSensorCommandResponse (pInternals, pMsgHeader, pData);
                break;

            case WB_SCL_MSG_TYPE_FAULT:
                /* Fault data received from Safety CPU */
                wb_assl_HandleSafetyFaultMessage (pInternals, pMsgHeader, pData);
                break;

            case WB_SCL_MSG_TYPE_HEARTBEAT:
            default:
                /* Invalid message type */
                wb_assl_ReportValidationError (pInternals->pPack);
                break;
        }
    }
}

void wb_assl_ReportValidationError (adi_wil_pack_t const * const pPack)
{
    /* Validate pack struct pointer */
    if (pPack != (void *) 0)
    {
        /* Notify user that an error occurred while validating a message
         * from a Safety CPU */
        wb_wil_ui_GenerateFuSaEvent (pPack, ADI_WIL_EVENT_FAULT_SCL_VALIDATION, (void *) 0u);
    }
}

bool wb_assl_GetPendingMessage (adi_wil_pack_t const * const pPack,
                                uint8_t ** const pMessage,
                                uint8_t * const pDeviceId)
{
    /* Return value of this function */
    bool bMessageAvailable;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_assl_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters */
    if ((pInternals == (void *) 0) || (pMessage == (void *) 0) || (pDeviceId == (void *) 0))
    {
        bMessageAvailable = false;
    }
    /* Check if user request message is ready to be sent */
    else if (pInternals->ASSL.bUserRequestMessageTxReady)
    {
        *pMessage = &pInternals->ASSL.UserRequestMessage [0];
        *pDeviceId = pInternals->ASSL.UserRequestState.iDeviceId;

        /* Set return code to indicate a message is ready to be sent */
        bMessageAvailable = true;
    }
    /* Check if heartbeat message is ready to be sent */
    else if (pInternals->ASSL.bHeartbeatMessageTxReady)
    {
        *pMessage = &pInternals->ASSL.HeartbeatMessage [0];
        *pDeviceId = WB_WIL_DEV_ALL_NODES;

        /* Set return code to indicate a message is ready to be sent */
        bMessageAvailable = true;
    }
    /* No messages ready */
    else
    {
        bMessageAvailable = false;
    }

    return bMessageAvailable;
}

void wb_assl_ReleaseBuffer (adi_wil_pack_t const * const pPack,
                            uint8_t const * const pMessage)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_assl_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters */
    if ((pInternals != (void *) 0) && (pMessage != (void *) 0))
    {
        /* Check if the input pointer matches the user request message */
        if (pMessage == &pInternals->ASSL.UserRequestMessage [0])
        {
            /* Message sent - mark ready as false to avoid resending the same
             * message next time */
            pInternals->ASSL.bUserRequestMessageTxReady = false;
        }
        /* Check if the input pointer matches the Heartbeat request
         * message */
        else if (pMessage == &pInternals->ASSL.HeartbeatMessage [0])
        {
            /* Message sent - mark ready as false to avoid resending the same
             * message next time */
            pInternals->ASSL.bHeartbeatMessageTxReady = false;
        }
        else
        {
            /* No other buffers to free up */
        }
    }
}

void wb_assl_SetMeasurementParameters (adi_wil_pack_t const * const pPack,
                                       adi_wil_xms_parameters_t const * const pXMSParameters)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_assl_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters */
    if ((pInternals != (void *) 0) && (pXMSParameters != (void *) 0))
    {
        /* Copy the non-FuSa xms parameters to the internal non-FuSa
         * parameter storage */
        (void) memcpy (&pInternals->XMSParameters,
                       pXMSParameters,
                       sizeof (adi_wil_xms_parameters_t));

        /* Update non-FuSa XMS allocation (bFuSaContext == false) */
        wb_xms_UpdateXMSAllocation (pInternals, false);
    }
}

void wb_assl_InitializeAllocation (adi_wil_pack_t const * const pPack,
                                   uint16_t iPMSPackets,
                                   uint16_t iEMSPackets)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_assl_GetSafetyInternalsPointer (pPack);

    /* Check we were handed valid parameters before dereferencing */
    if ((void *) 0 != pInternals)
    {
        /* Invoke the XMS allocation initializer function */
        wb_xms_InitializeAllocation (pInternals,
                                     iPMSPackets,
                                     iEMSPackets);
    }
}

/******************************************************************************
*   Static function definitions
******************************************************************************/

static void wb_assl_CheckForDisconnectedNodes (adi_wil_safety_internals_t * const pInternals,
                                               uint32_t iCurrentTicks)
{
    adi_wil_device_t iNodeDeviceId;

    /* Loop over all nodes in the network */
    for (uint8_t iNodeId = 0; iNodeId < ADI_WIL_MAX_NODES; iNodeId++)
    {
        /* Check if the node is connected */
        if (wb_assl_CheckNodeConnected (&pInternals->ASSL, iNodeId))
        {
            /* If node is connected, calculate the time elapsed since a valid
             * packet was last received from the node */
            if (wb_assl_CheckForTimeout (pInternals->ASSL.iLastMessageReceivedTime [iNodeId],
                                         iCurrentTicks,
                                         WB_ASSL_NODE_DISCONNECTION_TIMEOUT))
            {
                /* Timed out waiting for a packet from the node */
                /* Mark node as disconnected */
                wb_assl_UpdateNodeMask (&pInternals->ASSL.iConnectedNodes, iNodeId, false);

                /* Notify user that a node was disconnected */
                iNodeDeviceId = 1ULL << iNodeId;
                wb_wil_ui_GenerateFuSaEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_SAFETY_CPU_DISCONNECTED, &iNodeDeviceId);
            }
        }
    }
}

static void wb_assl_CheckForAPITimeout (adi_wil_safety_internals_t * const pInternals,
                                        uint32_t iCurrentTicks)
{
    /* Check if there is an active request */
    if (pInternals->ASSL.UserRequestState.bActive)
    {
        /* Check if timed out while waiting on a response to the API request */
        if (wb_assl_CheckForTimeout (pInternals->ASSL.UserRequestState.iStartTime, 
                                     iCurrentTicks,
                                     pInternals->ASSL.UserRequestState.iRetryInterval))
        {

            /* If we've had an API timeout and our request is still pending Tx,
             * generate a event to notify application hang in process thread */
            if (pInternals->ASSL.bUserRequestMessageTxReady)
            {
                wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                             ADI_WIL_EVENT_FAULT_SCL_TX_MESSAGE_EXPIRED,
                                             (void *) 0);
            }

            /* Timeout occurred */
            if (pInternals->ASSL.UserRequestState.iRemainingRetries == 0u)
            {
                /* No more retries left */
                wb_assl_TimeoutRequest (pInternals);
            }
            /* Allowed to retry the API request */
            else
            {
                /* Decrement number of retries left */
                pInternals->ASSL.UserRequestState.iRemainingRetries--;

                /* Reactivate the request to retry */
                wb_assl_ReactivateRequest (pInternals, iCurrentTicks);
            }
        }
    }
}

static void wb_assl_ReactivateRequest (adi_wil_safety_internals_t * const pInternals,
                                       uint32_t iCurrentTicks)
{
    /* Note down start time */
    pInternals->ASSL.UserRequestState.iStartTime = iCurrentTicks;

    /* Update state variables to indicate that the frame is ready for
     * transmission */
    pInternals->ASSL.bUserRequestMessageTxReady = true;
}

static void wb_assl_TimeoutRequest (adi_wil_safety_internals_t * const pInternals)
{
    /* Deactivate the current request */
    wb_assl_DeactivateRequest (pInternals);

    /* Call the timeout routine of the specific API function */
    switch (pInternals->ASSL.UserRequestState.eAPI)
    {
        case ADI_WIL_API_CONFIGURE_CELL_BALANCING:
            /* Call timeout method for Config Cell balancing */
            wb_wil_TimeoutConfigureCellBalancing (pInternals);
            break;
        case ADI_WIL_API_GET_CELL_BALANCING_STATUS:
            /* Call timeout method for Config Cell balancing */
            wb_wil_TimeoutGetCellBalancingStatus (pInternals);
            break;
        default:
            /* Nothing to do */
            break;
    }
}

static void wb_assl_CheckForHeartbeatOffState (adi_wil_safety_internals_t * const pInternals,
                                               uint32_t iCurrentTicks)
{
    /* Heartbeat is OFF if no node has sent a message in the last 1000ms */
    if (pInternals->ASSL.bHeartbeatTimerOn &&
        (wb_assl_CheckForTimeout (pInternals->ASSL.iLastMessageTime,
                                  iCurrentTicks,
                                  WB_ASSL_NODE_ALIVE_TIMEOUT)))
    {
        pInternals->ASSL.bHeartbeatTimerOn = false;
    }
}

static void wb_assl_CheckForHeartbeatTimeout (adi_wil_safety_internals_t * const pInternals,
                                              uint32_t iCurrentTicks)
{
    /* Check if heartbeat timer is ON and it is time to send a heartbeat message */
    if (pInternals->ASSL.bHeartbeatTimerOn && 
        wb_assl_CheckForTimeout (pInternals->ASSL.iHeartbeatTimerStartTime, 
                                 iCurrentTicks,
                                 ADI_WIL_SAFETY_HEARTBEAT_TIME))
    {
        /* If we've had an HB timeout and our message is still pending Tx,
         * generate a event to notify application hang in process thread */
        if (pInternals->ASSL.bHeartbeatMessageTxReady)
        {
            wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                         ADI_WIL_EVENT_FAULT_SCL_TX_MESSAGE_EXPIRED,
                                         (void *) 0);
        }

        /* Submit a heartbeat message for transmission */
        if (wb_assl_WriteHeartbeatMessage (&pInternals->ASSL))
        {
            /* Heartbeat Frame is ready for transmission */
            pInternals->ASSL.bHeartbeatMessageTxReady = true;

            /* Update the heartbeat start time for next time */
            pInternals->ASSL.iHeartbeatTimerStartTime = iCurrentTicks;

            /* Increment sequence number for the next message */
            wb_assl_IncrementWithRollover8 (&pInternals->ASSL.iHeartbeatSequenceNumber);
        }
    }
}

static void wb_assl_HandleSafetyConnectResponse (adi_wil_safety_internals_t * const pInternals,
                                                 wb_msg_header_t const * const pMsgHeader,
                                                 uint8_t const * const pData)
{
    /* Connect response structure to store the connect response received from
     * the Safety CPU */
    adi_wil_connect_safety_cpu_t ConnectResponse;

    /* Check received message parameters and data pointer */
    if ((pMsgHeader->iSourceDeviceId >= ADI_WIL_MAX_NODES) || 
        (pMsgHeader->iPayloadLength != WB_SCL_CONNECT_RESPONSE_LEN) ||
         (pData == (void *) 0))
    {
        /* Report a validation error */
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* Store safety CPU device ID */
        ConnectResponse.eDeviceId = 1ULL << pMsgHeader->iSourceDeviceId;

        /* If the node is connecting, clear the the device from the FuSa XMS
         * parameters regardless of connection status. This allows any
         * measurement sequence number to be used from this device */
        wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iPMSDevices, pMsgHeader->iSourceDeviceId, false);
        wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iBMSDevices, pMsgHeader->iSourceDeviceId, false);
        wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iEMSDevices, pMsgHeader->iSourceDeviceId, false);

        /* Update FuSa XMS allocation (bFuSaContext == true) */
        wb_xms_UpdateXMSAllocation (pInternals, true);

        /* Unpack connect response from message buffer */
        wb_assl_ParseConnectResponse (pData, &ConnectResponse);

        /* Mark node as connected if rc is SUCCESS */
        if (ConnectResponse.rc == ADI_WIL_ERR_SUCCESS)
        {
            wb_assl_UpdateNodeMask (&pInternals->ASSL.iConnectedNodes, pMsgHeader->iSourceDeviceId, true);

            /* Mark node as PMS Enabled if the node will send non-zero PMS
            * Packets */
            wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iPMSDevices, pMsgHeader->iSourceDeviceId,
                                    (ConnectResponse.iMaxPMSPackets != 0u));

            /* Mark node as BMS Enabled if the node will send non-zero BMS
            * Packets */
            wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iBMSDevices, pMsgHeader->iSourceDeviceId, 
                                    (ConnectResponse.iMaxBMSPackets != 0u));

            /* Mark node as EMS Enabled if the node will send non-zero EMS
            * Packets */
            wb_assl_UpdateNodeMask (&pInternals->FuSaXMSParameters.iEMSDevices, pMsgHeader->iSourceDeviceId,
                                    (ConnectResponse.iMaxEMSPackets != 0u));

            /* Check if maximum number of PMS packets needs to be updated */
            if (ConnectResponse.iMaxPMSPackets > pInternals->FuSaXMSParameters.iPMSPackets)
            {
                /* Update state to reflect the current highest number of PMS
                * packets */
                pInternals->FuSaXMSParameters.iPMSPackets = ConnectResponse.iMaxPMSPackets;
            }

            /* Check if maximum number of BMS packets needs to be updated */
            if (ConnectResponse.iMaxBMSPackets > pInternals->FuSaXMSParameters.iBMSPackets)
            {
                /* Update state to reflect the current highest number of BMS
                * packets */
                pInternals->FuSaXMSParameters.iBMSPackets = ConnectResponse.iMaxBMSPackets;
            }

            /* Check if maximum number of EMS packets needs to be updated */
            if (ConnectResponse.iMaxEMSPackets > pInternals->FuSaXMSParameters.iEMSPackets)
            {
                /* Update state to reflect the current highest number of EMS
                * packets */
                pInternals->FuSaXMSParameters.iEMSPackets = ConnectResponse.iMaxEMSPackets;
            }

            /* Update FuSa XMS allocation (bFuSaContext == true) */
            wb_xms_UpdateXMSAllocation (pInternals, true);
        }

        /* Pass on the connect response to the user */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_SAFETY_CPU_CONNECTED, &ConnectResponse);
    }
}

static void wb_assl_ParseConnectResponse (uint8_t const * const pData,
                                          adi_wil_connect_safety_cpu_t * const pConnectResponse)
{
    /* Populate connect response structure */
    /* Extract the MAC address of the node to which the Safety CPU is 
     * connected */
    (void) memcpy (&pConnectResponse->NodeMAC [0], &pData [WB_SCL_CONNECT_MSG_NODE_MAC_OFFSET], ADI_WIL_MAC_ADDR_SIZE);

    /* Unpack the Serial Id of the Safety CPU */
    (void) memcpy (&pConnectResponse->SafetyCPUSerialId [0], &pData [WB_SCL_CONNECT_MSG_SAFETY_SERIAL_ID_OFFSET], ADI_WIL_MAC_ADDR_SIZE);

    /* Unpack the version information for the Safety CPU software */
    pConnectResponse->iSoftwareVersionMajor = ((((pData [WB_SCL_CONNECT_MSG_SOFTWARE_MAJOR_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                                (((pData [WB_SCL_CONNECT_MSG_SOFTWARE_MAJOR_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    pConnectResponse->iSoftwareVersionMinor = ((((pData [WB_SCL_CONNECT_MSG_SOFTWARE_MINOR_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                                (((pData [WB_SCL_CONNECT_MSG_SOFTWARE_MINOR_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    pConnectResponse->iSoftwareVersionPatch = ((((pData [WB_SCL_CONNECT_MSG_SOFTWARE_PATCH_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                                (((pData [WB_SCL_CONNECT_MSG_SOFTWARE_PATCH_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    pConnectResponse->iSoftwareVersionBuild = ((((pData [WB_SCL_CONNECT_MSG_SOFTWARE_BUILD_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                                (((pData [WB_SCL_CONNECT_MSG_SOFTWARE_BUILD_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    /* Unpack the CRC checksum values for various containers on the Safety
     * CPU */
    pConnectResponse->iConfigurationCRC = ((((pData [WB_SCL_CONNECT_MSG_CONFIG_CRC_OFFSET + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                                            (((pData [WB_SCL_CONNECT_MSG_CONFIG_CRC_OFFSET + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                                            (((pData [WB_SCL_CONNECT_MSG_CONFIG_CRC_OFFSET + 2u] & 0xFFFFFFFFu) << 8u) & 0x0000FF00u) |
                                            (((pData [WB_SCL_CONNECT_MSG_CONFIG_CRC_OFFSET + 3u] & 0xFFFFFFFFu) << 0u) & 0x000000FFu)) & 0xFFFFFFFFu;

    pConnectResponse->iPMSContainerCRC = ((((pData [WB_SCL_CONNECT_MSG_PMS_CONTAINER_CRC_OFFSET + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_PMS_CONTAINER_CRC_OFFSET + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_PMS_CONTAINER_CRC_OFFSET + 2u] & 0xFFFFFFFFu) << 8u) & 0x0000FF00u) |
                                        (((pData [WB_SCL_CONNECT_MSG_PMS_CONTAINER_CRC_OFFSET + 3u] & 0xFFFFFFFFu) << 0u) & 0x0000000FFu)) & 0xFFFFFFFFu;

    pConnectResponse->iBMSContainerCRC = ((((pData [WB_SCL_CONNECT_MSG_BMS_CONTAINER_CRC_OFFSET + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_BMS_CONTAINER_CRC_OFFSET + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_BMS_CONTAINER_CRC_OFFSET + 2u] & 0xFFFFFFFFu) << 8u) & 0x0000FF00u) |
                                        (((pData [WB_SCL_CONNECT_MSG_BMS_CONTAINER_CRC_OFFSET + 3u] & 0xFFFFFFFFu) << 0u) & 0x0000000FFu)) & 0xFFFFFFFFu;

    pConnectResponse->iEMSContainerCRC = ((((pData [WB_SCL_CONNECT_MSG_EMS_CONTAINER_CRC_OFFSET + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_EMS_CONTAINER_CRC_OFFSET + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                                        (((pData [WB_SCL_CONNECT_MSG_EMS_CONTAINER_CRC_OFFSET + 2u] & 0xFFFFFFFFu) << 8u) & 0x0000FF00u) |
                                        (((pData [WB_SCL_CONNECT_MSG_EMS_CONTAINER_CRC_OFFSET + 3u] & 0xFFFFFFFFu) << 0u) & 0x0000000FFu)) & 0xFFFFFFFFu;

    /* Unpack the measurement intervals configured on the Safety CPU */
    pConnectResponse->iPMSInterval = ((((pData [WB_SCL_CONNECT_MSG_PMS_INTERVAL_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                    (((pData [WB_SCL_CONNECT_MSG_PMS_INTERVAL_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    pConnectResponse->iBMSInterval = ((((pData [WB_SCL_CONNECT_MSG_BMS_INTERVAL_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                    (((pData [WB_SCL_CONNECT_MSG_BMS_INTERVAL_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    pConnectResponse->iEMSInterval = ((((pData [WB_SCL_CONNECT_MSG_EMS_INTERVAL_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                    (((pData [WB_SCL_CONNECT_MSG_EMS_INTERVAL_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

    /* Unpack the maximum packets per measurement interval configured on
     * the Safety CPU */
    pConnectResponse->iMaxPMSPackets = pData [WB_SCL_CONNECT_MSG_MAX_PMS_PACKETS_OFFSET];
    pConnectResponse->iMaxBMSPackets = pData [WB_SCL_CONNECT_MSG_MAX_BMS_PACKETS_OFFSET];
    pConnectResponse->iMaxEMSPackets = pData [WB_SCL_CONNECT_MSG_MAX_EMS_PACKETS_OFFSET];
    pConnectResponse->iOperatingMode = pData [WB_SCL_CONNECT_MSG_OPERATING_MODE_OFFSET];

    /* Unpack the return code for the connect request, any non-zero RC means FAIL */
    if (pData [WB_SCL_CONNECT_MSG_RC_OFFSET] == 0u)
    {
        pConnectResponse->rc = ADI_WIL_ERR_SUCCESS;
    }
    else
    {
        pConnectResponse->rc = ADI_WIL_ERR_FAIL;
    }
}

static void wb_assl_HandleFuSaMeasurement (adi_wil_safety_internals_t * const pInternals,
                                           wb_msg_header_t * const pMsgHeader,
                                           adi_wil_xms_measurement_cmd_id_t eCmdId,
                                           uint8_t const * const pData)
{
    /* Sensor type of the data: BMS, PMS or EMS */
    uint8_t iSensorType;
    wb_xms_metadata_t XmsMetadata;
    const uint8_t *pPayload;

    /* Check received message parameters and check node connected status */
    if (!wb_assl_CheckNodeConnected (&pInternals->ASSL, pMsgHeader->iSourceDeviceId) ||
        (pMsgHeader->iPayloadLength < WB_SCL_XMS_MESSAGE_LEN_MIN) ||
        (pMsgHeader->iPayloadLength > WB_SCL_PAYLOAD_LEN_MAX) ||
        (pData == (void *) 0))
    {
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* Read sensor type from SCL frame payload */
        iSensorType = pData[WB_SCL_XMS_MSG_SENSOR_TYPE_OFFSET];

        /* Decrement the length of payload sent to XMS handler */
        pMsgHeader->iPayloadLength -= WB_SCL_XMS_MSG_SENSOR_TYPE_LEN;

        /* Set payload pointer as NULL if message length is 0 */
        pPayload = (pMsgHeader->iPayloadLength == 0u) ? ((void *) 0) : (&pData [WB_SCL_XMS_MSG_MEASUREMENT_DATA_OFFSET]);

        /* Initialize XMS meta data */
        XmsMetadata.eCmdId = eCmdId;

        /* Process data as per sensor type */
        switch (iSensorType)
        {
            case WB_SCL_SENSOR_TYPE_BMS:
                /* BMS measurement data received - call XMS measurement
                 * handler */
                XmsMetadata.eType = ADI_WIL_XMS_BMS;
                (void) wb_xms_HandleFuSaMeasurement (pInternals, pMsgHeader, &XmsMetadata, pPayload);
                break;

            case WB_SCL_SENSOR_TYPE_PMS:
                /* PMS measurement data received - call XMS measurement
                 * handler */
                XmsMetadata.eType = ADI_WIL_XMS_PMS;
                (void) wb_xms_HandleFuSaMeasurement (pInternals, pMsgHeader, &XmsMetadata, pPayload);
                break;

            case WB_SCL_SENSOR_TYPE_EMS:
                /* EMS measurement data received - call XMS measurement
                 * handler */
                XmsMetadata.eType = ADI_WIL_XMS_EMS;
                (void) wb_xms_HandleFuSaMeasurement (pInternals, pMsgHeader, &XmsMetadata, pPayload);
                break;

            default:
                /* Invalid sensor type */
                wb_assl_ReportValidationError (pInternals->pPack);
                break;
        }
    }
}

static void wb_assl_HandleSensorCommandResponse (adi_wil_safety_internals_t * const pInternals,
                                                 wb_msg_header_t const * const pMsgHeader,
                                                 uint8_t const * const pData)
{
    /* Sensor command type unpacked from the Safety CPU response */
    uint8_t iSensorCmdType;

    /* Check if response is from a connected node */
    /* Validate payload length */
    /* Check if data pointer is valid */
    if (!wb_assl_CheckNodeConnected (&pInternals->ASSL, pMsgHeader->iSourceDeviceId) ||
        (pMsgHeader->iPayloadLength < WB_SCL_SENSOR_CMD_RSP_LEN_MIN) ||
        (pData == (void *) 0))
    {
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* Read the command ID from the response payload */
        iSensorCmdType = pData [WB_SCL_SENSOR_CMD_RSP_CMDID_OFFSET];

        /* Process response based on sensor command type */
        switch (iSensorCmdType)
        {
            /* Configure cell balancing response */
            case WB_SCL_SENSOR_CMD_CONFIG_CELL_BALANCING:
                wb_assl_HandleConfigureCellBalancingResponse (pInternals, pMsgHeader, pData);
                break;
            /* Get cell balancing status response */
            case WB_SCL_SENSOR_CMD_GET_CELL_BALANCING_STATUS:
                wb_assl_HandleGetCellBalancingStatusResponse (pInternals, pMsgHeader, pData);
                break;
            /* Unsupported command */
            default:
                wb_assl_ReportValidationError (pInternals->pPack);
                break;
        }
    }
}

static void wb_assl_HandleConfigureCellBalancingResponse (adi_wil_safety_internals_t * const pInternals,
                                                          wb_msg_header_t const * const pMsgHeader,
                                                          uint8_t const * const pData)
{
    /* Sequence number of the Safety CPU response */
    uint8_t iSequenceNumber;

    /* uint8_t error code returned by the Safety CPU */
    uint8_t iReturnCode;

    /* adi_wil_err_t enum equivalent of uint8_t error code returned by the
     * Safety CPU */
    adi_wil_err_t eReturnCode;

    /* Validate SCL header parameters against expected values */
    if (pMsgHeader->iPayloadLength != WB_SCL_CELL_BALANCING_RESPONSE_LEN)
    {
        /* Incorrect response length - report a validation error */
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* De-serialize response payload */
        iSequenceNumber = pData [WB_SCL_CELL_BALANCING_RSP_SEQNUM_OFFSET];
        iReturnCode = pData [WB_SCL_CELL_BALANCING_RSP_RC_OFFSET];

        /* Check if message sequence number matches request sequence number */
        if (iSequenceNumber == pInternals->ASSL.iRequestSequenceNumber [pInternals->ASSL.UserRequestState.iDeviceId])
        {
            /* Translate error code received from Safety CPU into type
             * adi_wil_err_t */
            eReturnCode = (iReturnCode == WB_SCL_SAFETY_CPU_SUCCESS_RC) ? ADI_WIL_ERR_SUCCESS : ADI_WIL_ERR_FAIL;

            /* Pass on the response to the cell balancing module */
            wb_wil_HandleConfigureCellBalancingResponse (pInternals, pMsgHeader->iSourceDeviceId, eReturnCode);
        }
    }
}

static void wb_assl_HandleGetCellBalancingStatusResponse (adi_wil_safety_internals_t * const pInternals,
                                                          wb_msg_header_t const * const pMsgHeader,
                                                          uint8_t const * const pData)
{
    /* Structure of cell balancing status response */
    adi_wil_cell_balancing_status_t Status;

    /* adi_wil_err_t enum equivalent of uint8_t error code */
    adi_wil_err_t eReturnCode;

    /* Clear local variable before de-serializing into structure */
    (void) memset (&Status, 0, sizeof (Status));

    /* Validate SCL header parameters against expected values */
    if ((pMsgHeader->iSourceDeviceId >= ADI_WIL_MAX_NODES) || 
        (pMsgHeader->iPayloadLength != WB_SCL_GET_CELL_BALANCING_STATUS_RESPONSE_LEN))
    {
        /* Incorrect response length - report a validation error */
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* NOTE: A one byte BMIC count exists in this message prior to the
         * list of enabled cells. This is unused in this implementation */

        /* Read a uint64_t from the buffer to the variable */
        Status.iEnabledCells = ((((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 0u] & 0xFFFFFFFFFFFFFFFFu) << 56u) & 0xFF00000000000000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 1u] & 0xFFFFFFFFFFFFFFFFu) << 48u) & 0x00FF000000000000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 2u] & 0xFFFFFFFFFFFFFFFFu) << 40u) & 0x0000FF0000000000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 3u] & 0xFFFFFFFFFFFFFFFFu) << 32u) & 0x000000FF00000000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 4u] & 0xFFFFFFFFFFFFFFFFu) << 24u) & 0x00000000FF000000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 5u] & 0xFFFFFFFFFFFFFFFFu) << 16u) & 0x0000000000FF0000u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 6u] & 0xFFFFFFFFFFFFFFFFu) << 8u)  & 0x000000000000FF00u) |
                                (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET + 7u] & 0xFFFFFFFFFFFFFFFFu) << 0u)  & 0x00000000000000FFu)) & 0xFFFFFFFFFFFFFFFFu;

        /* Read a uint16_t from the buffer to the variable */
        Status.iRemainingDuration = ((((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_DURATION_OFFSET + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                                     (((pData [WB_SCL_CELL_BALANCING_STATUS_RSP_DURATION_OFFSET + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;

        /* Read the one-byte thermal shutdown flag and interpret as a boolean */
        Status.bThermalShutdown = (pData [WB_SCL_CELL_BALANCING_STATUS_RSP_THERMAL_OFFSET] != 0u);

        /* Initialize the device ID of message*/
        Status.eDeviceId = (1ULL << pMsgHeader->iSourceDeviceId);

        /* Check if message sequence number matches request sequence number */
        if (pData [WB_SCL_CELL_BALANCING_STATUS_RSP_SEQNUM_OFFSET] == pInternals->ASSL.iBroadcastRequestSequenceNumber)
        {
            /* Translate error code received from Safety CPU into enum */
            eReturnCode = (pData [WB_SCL_CELL_BALANCING_STATUS_RSP_RC_OFFSET] == WB_SCL_SAFETY_CPU_SUCCESS_RC) ? ADI_WIL_ERR_SUCCESS :
                                                                                                                 ADI_WIL_ERR_FAIL;

            /* Pass on the response to the cell balancing module */
            wb_wil_HandleGetCellBalancingStatusResponse (pInternals,
                                                         &Status,
                                                         eReturnCode);
        }
    }
}

static void wb_assl_HandleSafetyFaultMessage (adi_wil_safety_internals_t const * const pInternals,
                                              wb_msg_header_t const * const pMsgHeader,
                                              uint8_t const * const pData)
{
    /* Structure containing fault summary sent by the Safety CPU */
    adi_wil_fault_safety_cpu_t FaultMessage;

    /* Check if message is from a connected node */
    /* Ensure message length is in the valid range */
    if ((pMsgHeader->iPayloadLength > WB_SCL_PAYLOAD_LEN_MAX) ||
        (!wb_assl_CheckNodeConnected (&pInternals->ASSL, pMsgHeader->iSourceDeviceId)))
    {
        wb_assl_ReportValidationError (pInternals->pPack);
    }
    else
    {
        /* Populate the fault summary structure */
        FaultMessage.eDeviceId = 1ULL << (pMsgHeader->iSourceDeviceId);
        FaultMessage.iLength = pMsgHeader->iPayloadLength;

        /* Copy the fault data into the fault summary structure */
        (void) memset (&FaultMessage.iData[0], 0, sizeof(FaultMessage.iData));
        (void) memcpy (&FaultMessage.iData[0], pData, pMsgHeader->iPayloadLength);        

        /* Pass on the fault summary to the user */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack, ADI_WIL_EVENT_FAULT_SAFETY_CPU, &FaultMessage);
    }
}

static bool wb_assl_WriteHeartbeatMessage (adi_wil_assl_internals_t * const pASSLInternals)
{
    /* Return value of this function */
    bool bComplete;

    /* SCL header parameters */
    wb_msg_header_t MsgHeader = {
        .iSourceDeviceId = WB_WIL_DEV_ALL_NODES,
        .iSequenceNumber = pASSLInternals->iHeartbeatSequenceNumber,
        .iMessageType = WB_SCL_MSG_TYPE_HEARTBEAT,
        .iPayloadLength = WB_SCL_HEARTBEAT_REQUEST_LEN,
    };

    /* Zero out the request frame before writing the request */
    (void) memset (&pASSLInternals->HeartbeatMessage [0], 0,
                   sizeof (pASSLInternals->HeartbeatMessage));

    /* Write the heartbeat request */
    wb_assl_WriteHeartbeatRequest (pASSLInternals->iConnectedNodes,
                                   &pASSLInternals->HeartbeatMessage [WB_SCL_MSG_HDR_LEN]);

    /* Wrap up the request in SCL header and CRC */
    bComplete = wb_scl_WrapSCLFrame (&MsgHeader, &pASSLInternals->HeartbeatMessage [0]);

    return bComplete;
}

static void wb_assl_WriteHeartbeatRequest (uint64_t iConnectedMask,
                                           uint8_t * const pBuffer)
{
    /* Write the heartbeat message into the given buffer */
    /* Pack 64-bit node connected mask */
    /* Store MSB to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 0u] = (uint8_t) (((iConnectedMask & 0xFF00000000000000u) >> 56u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 1u] = (uint8_t) (((iConnectedMask & 0x00FF000000000000u) >> 48u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 2u] = (uint8_t) (((iConnectedMask & 0x0000FF0000000000u) >> 40u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 3u] = (uint8_t) (((iConnectedMask & 0x000000FF00000000u) >> 32u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 4u] = (uint8_t) (((iConnectedMask & 0x00000000FF000000u) >> 24u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 5u] = (uint8_t) (((iConnectedMask & 0x0000000000FF0000u) >> 16u) & 0xFFu);

    /* Store next byte to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 6u] = (uint8_t) (((iConnectedMask & 0x000000000000FF00u) >> 8u) & 0xFFu);

    /* Store LSB to buffer */
    pBuffer [WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET + 7u] = (uint8_t) (((iConnectedMask & 0x00000000000000FFu) >> 0u) & 0xFFu);

    /* Write the reserved field */
    (void) memset (&pBuffer [WB_SCL_HEARTBEAT_MSG_RESERVED_OFFSET], 0, WB_SCL_HEARTBEAT_MSG_RESERVED_LEN);
}

static void wb_assl_WriteConfigureCellBalancingRequest (adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                        uint16_t iDuration,
                                                        uint32_t iUVThreshold,
                                                        uint8_t * const pBuffer)
{
    /* Write the cell balancing request */
    /* Pack the sensor command type */
    pBuffer [WB_SCL_SENSOR_COMMAND_ID_OFFSET] = WB_SCL_SENSOR_CMD_CONFIG_CELL_BALANCING;

    /* Pack the discharge duty cycle */
    (void) memcpy (&pBuffer [WB_SCL_CELL_BALANCING_REQ_DDC_OFFSET], pDischargeDutyCycle, ADI_WIL_MAX_CELLS);

    /* Pack 16-bit iDuration value */
    pBuffer [WB_SCL_CELL_BALANCING_REQ_DURATION_OFFSET + 0u] = (uint8_t) (((iDuration & 0xFF00u) >> 8u) & 0xFFu);
    pBuffer [WB_SCL_CELL_BALANCING_REQ_DURATION_OFFSET + 1u] = (uint8_t) (((iDuration & 0x00FFu) >> 0u) & 0xFFu);

    /* Pack 32-bit iUnderVoltageThreshold value */
    pBuffer [WB_SCL_CELL_BALANCING_REQ_UV_OFFSET + 0u] = (uint8_t) (((iUVThreshold & 0xFF000000u) >> 24u) & 0xFFu);
    pBuffer [WB_SCL_CELL_BALANCING_REQ_UV_OFFSET + 1u] = (uint8_t) (((iUVThreshold & 0x00FF0000u) >> 16u) & 0xFFu);
    pBuffer [WB_SCL_CELL_BALANCING_REQ_UV_OFFSET + 2u] = (uint8_t) (((iUVThreshold & 0x0000FF00u) >> 8u)  & 0xFFu);
    pBuffer [WB_SCL_CELL_BALANCING_REQ_UV_OFFSET + 3u] = (uint8_t) (((iUVThreshold & 0x000000FFu) >> 0u)  & 0xFFu);
}

static void wb_assl_WriteGetCellBalancingStatusRequest (uint8_t * const pBuffer)
{
    /* Pack the sensor command type */
    pBuffer [WB_SCL_SENSOR_COMMAND_ID_OFFSET] = WB_SCL_SENSOR_CMD_GET_CELL_BALANCING_STATUS;
}

static void wb_assl_UpdateNodeMask (uint64_t * const pNodeMask,
                                    uint8_t iDeviceId,
                                    bool bEnabled)
{
    /* Check if node is valid */
    if (iDeviceId < ADI_WIL_MAX_NODES)
    {
        if (bEnabled)
        {
            /* Enable the bit corresponding to the node */
            *pNodeMask |= (((uint64_t) 1u) << iDeviceId);
        }
        else
        {
            /* Disable the bit corresponding to the node */
            *pNodeMask &= ~(((uint64_t) 1u) << iDeviceId);
        }
    }
}

static bool wb_assl_CheckNodeConnected (adi_wil_assl_internals_t const * const pASSLInternals,
                                        uint8_t iDeviceId)
{
    /* Check if the device ID belongs to a valid node */
    /* Check if the node is connected */
    return ((iDeviceId < ADI_WIL_MAX_NODES) &&
            ((pASSLInternals->iConnectedNodes & (((uint64_t) 1u) << iDeviceId)) != 0u));
}


static void wb_assl_UpdateLastMessageReceivedTime (adi_wil_assl_internals_t * const pASSLInternals,
                                                   uint8_t iDeviceId,
                                                   uint32_t iCurrentTicks)
{
    /* Check if device is a valid node */
    if (iDeviceId < ADI_WIL_MAX_NODES)
    {
        /* Update the last message timestamp for the given node */
        pASSLInternals->iLastMessageReceivedTime [iDeviceId] = iCurrentTicks;

        /* Update the last message timestamp for any node in the network */
        pASSLInternals->iLastMessageTime = iCurrentTicks;

        /* If transitioning to ON state, then reset the timer */
        if (!pASSLInternals->bHeartbeatTimerOn)
        {
            /* Start the heartbeat timer */
            pASSLInternals->iHeartbeatTimerStartTime = iCurrentTicks;
        }

        /* Switch the heartbeat timer to ON state */
        pASSLInternals->bHeartbeatTimerOn = true;
    }
}

static bool wb_assl_CheckForTimeout (uint32_t iStartTime,
                                     uint32_t iCurrentTime,
                                     uint32_t iTimeoutValue)
{
    /* Time elapsed since the API request was sent out without receiving a
     * response */
    uint32_t iElapsedTime;

    /* Boolean to indicate if timeout has happened or not */
    bool bTimedOut;

    /* Assume not timed out */
    bTimedOut = false;

    /* Check if time out occurred */
    iElapsedTime = (iCurrentTime - iStartTime);

    /* Not yet timed out */
    if (iElapsedTime < iCurrentTime)
    {
        /* CERT-C - Allow rollover as timers are expected to wrap */
    }

    /* Expected wait time was exceeded.
     * Check if the job has elapsed
     * ...but don't trigger a timeout if we've potentially underflowed by 2ms
     * to allow for timer jitter */
    if ((iElapsedTime > iTimeoutValue) &&
        (iElapsedTime < (UINT32_MAX - WB_ASSL_TIMER_JITTER_TOLERANCE)))
    {
        /* Timeout occurred */
        bTimedOut = true;
    }
    return bTimedOut;
}

static void wb_assl_IncrementWithRollover8 (uint8_t * pValue)
{
    /* Validate input pointer */
    if (pValue != (void *) 0)
    {
        /* Increment if less than maximum value */
        if (*pValue < ((uint8_t) UINT8_MAX))
        {
            (*pValue)++;
        }
        /* Else rollover to 0 */
        else
        {
            *pValue = 0u;
        }
    }
}

static adi_wil_safety_internals_t * wb_assl_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack)
{
    /* Return value of this method */
    adi_wil_safety_internals_t * pInternals;

    /* Initialize local variable to NULL */
    pInternals = (void *) 0;

    /* Check pack instance before dereferencing */
    if ((void *) 0 != pPack)
    {
        /* Assign safety internals pointer to local variable */
        pInternals = pPack->pSafetyInternals;
    }

    /* Return local variable pointer to safety internals */
    return pInternals;
}

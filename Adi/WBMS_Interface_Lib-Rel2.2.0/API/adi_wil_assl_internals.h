/******************************************************************************
 * @file     adi_wil_assl_internals.h
 *
 * @brief    WIL FE ASSL internals structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_ASSL_INTERNALS__H
#define ADI_WIL_ASSL_INTERNALS__H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

/**
 * @brief   Structure for storing state of user request
 */
typedef struct
{
    adi_wil_api_t eAPI;                                                 /*!< Enum indicating which API function is activating a request */
    uint32_t iStartTime;                                                /*!< Time when the request was posted */
    uint32_t iRetryInterval;                                            /*!< Variable to store retry interval value */
    uint8_t iDeviceId;                                                  /*!< Target device ID for the request */
    uint8_t iRemainingRetries;                                          /*!< Number of retries remaining */
    volatile bool bActive;                                              /*!< Boolean to mark request as active or not */
} adi_wil_user_request_state_t;

/**
 * @brief   ASSL Internals structure
 */
 typedef struct {
    adi_wil_user_request_state_t UserRequestState;                    /*!< State of User request API */
    uint64_t iConnectedNodes;                                         /*!< Bitmap of nodes that are connected */
    uint32_t iHeartbeatTimerStartTime;                                /*!< Time when the heartbeat timer was started */
    uint32_t iLastMessageReceivedTime[ADI_WIL_MAX_NODES];             /*!< Timestamp of last packet received from each node */
    uint32_t iLastMessageTime;                                        /*!< Timestamp of last packet received from a node in the network */
    uint8_t UserRequestMessage [ADI_WIL_MAX_APP_PAYLOAD_SIZE];        /*!< Buffer for storing user context request message */
    uint8_t HeartbeatMessage [ADI_WIL_MAX_APP_PAYLOAD_SIZE];          /*!< Buffer for storing process task context heartbeat message */
    uint8_t iRequestSequenceNumber[ADI_WIL_MAX_NODES];                /*!< Running counter used to track unicast requests to nodes */
    uint8_t iSCLFrame [ADI_WIL_MAX_APP_PAYLOAD_SIZE];                 /*!< Buffer to store validated SCL frame received from NIL */
    uint8_t iBroadcastRequestSequenceNumber;                          /*!< Running counter used to track broadcast requests to nodes */
    uint8_t iHeartbeatSequenceNumber;                                 /*!< Running counter used in heartbeat messages  */
    volatile bool bUserRequestMessageTxReady;                         /*!< Is user request message ready for transmission */
    bool bHeartbeatMessageTxReady;                                    /*!< Is process task message ready for transmission */
    bool bHeartbeatTimerOn;                                           /*!< Boolean to indicate if we are sending heartbeat messages */
} adi_wil_assl_internals_t;


#endif //ADI_WIL_ASSL_INTERNALS__H

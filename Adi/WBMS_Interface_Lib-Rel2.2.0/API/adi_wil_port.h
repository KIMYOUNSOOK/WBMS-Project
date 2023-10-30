/******************************************************************************
 * @file     adi_wil_port.h
 *
 * @brief    WBMS port structure definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_PORT__H
#define ADI_WIL_PORT__H

#include "adi_wil_types.h"
#include <stdint.h>
#include <stdbool.h>


/* The number of received frames that can be buffered  */
#define ADI_WIL_PORT_RX_FRAME_COUNT (2u)

/**
 * @struct adi_wil_port_stats_t
 * @brief Port statistics
 *
 */
typedef struct
{
    uint32_t iTxFrameCount;                  /*!< Total number of SPI frames sent */
    uint32_t iRxFrameCount;                  /*!< Total number of valid, non-idle frames processed */
    uint32_t iTxErrorCount;                  /*!< Total number of transmission failures */
    uint32_t iRxErrorCount;                  /*!< Total number of invalid SPI frames received */
    uint32_t iRxBuffAllocErrCount;           /*!< Total number of Rx memory buffer allocation errors */
    uint32_t iRunningRxErrorCount;           /*!< running count of Rx errors - this count will be reset when a valid frame is received */
    uint32_t iRunningLoggedOutFrameCount;    /*!< running count of how many times the port was logged out - this count will be reset when a valid frame is received */
    uint32_t iRunningValidFrameCount;        /*!< running count of how many valid idle+data frames received since last error frame */
    uint32_t iAckLostCount;                  /*!< Total number of acknowledgments that were unable to be queued */
    uint32_t iAckTxFailCount;                /*!< Total number of acknowledgments that were unable to be transmitted */
} adi_wil_port_stats_t;

/**
 * @brief   adi_wil_mgr_id_t
 */
typedef enum
{
    ADI_WIL_MGR_UNKNOWN,
    ADI_WIL_MGR_PRIMARY,
    ADI_WIL_MGR_SECONDARY
} adi_wil_mgr_id_t;

/**
 * @brief   adi_wil_query_device_state_t
 */
typedef struct
{
    uint32_t iTimeStamp;                                  /*!< Timestamp of request */
    uint8_t iRetries;                                     /*!< Retry count variable */
    bool bInProgress;                                     /*!< QueryDevice in progress flag */
    bool bDeviceExists;                                   /*!< Device exists in device list already flag */
    bool bPackLockAcquired;                               /*!< Pack lock acquired flag */
} adi_wil_query_device_state_t;

/**
 * @brief   adi_wil_ack_queue_t
 */
typedef struct
{
    uint16_t iValue [ADI_WIL_ACK_QUEUE_COUNT];      /*!< Value indicating that a notification ID respond with */
    uint8_t iCommandId [ADI_WIL_ACK_QUEUE_COUNT];   /*!< Value indicating that a notification type to respond with */
    uint8_t iHead;                                  /*!< Rolling count of number of elements added to queue */
    uint8_t iTail;                                  /*!< Rolling count of number of elements retrieved from queue */

} adi_wil_ack_queue_t;

/**
 * @brief   adi_wil_rx_frame_buffer_t
 */
typedef struct
{
    uint8_t iData [ADI_WIL_SPI_TRANSACTION_SIZE];       /*!< Buffer for SPI data */
    volatile bool bInUse;                               /*!< Boolean used to indicate if an RX frame is in use */
    volatile bool bReadyForProcessing;                  /*!< Booleans used to indicate if an Rx frame is ready for processing */
} adi_wil_rx_buffer_t;

/**
 * @brief Port State structure
 * For each port, the following information is stored:
 *  - pointers to Tx, Rx and job queues
 *  - pointers to Tx and Rx buffers used for last SPI transaction
 *  - session information
 */
typedef struct
{
    adi_wil_ack_queue_t          AckQueue;                                                 /*!< Queue for pending acknowledgment */
    adi_wil_port_stats_t         PortStatistics;                                           /*!< Port statistics */
    adi_wil_query_device_state_t QueryDeviceState;                                         /*!< QueryDevice state */
    adi_wil_rx_buffer_t          RxBuffer [ADI_WIL_PORT_RX_FRAME_COUNT];                   /*!< Array of buffers for receiving SPI data */
    adi_wil_pack_internals_t *   pPackInternals;                                           /*!< Parent link */
    uint8_t *                    pTx;                                                      /*!< Tx pointer for the last SPI transfer */
    uint8_t *                    pRx;                                                      /*!< Rx pointer for the last SPI transfer */
    adi_wil_mgr_id_t             Role;                                                     /*!< Id of Manager 1 or Manager 2 */
    uint8_t                      UserRequestFrame [ADI_WIL_SPI_TRANSACTION_SIZE];          /*!< Buffer for storing user context request frame */
    uint8_t                      ProcessTaskRequestFrame [ADI_WIL_SPI_TRANSACTION_SIZE];   /*!< Buffer for storing process task context request frame */
    uint8_t                      IdleFrame [ADI_WIL_SPI_TRANSACTION_SIZE];                 /*!< Idle frame buffer */
    uint8_t                      iSessionId;                                               /*!< Key used for identifying the SPI session */
    uint8_t                      iProtocolVersion;                                         /*!< Protocol version */
    volatile bool                bUserRequestFramePending;                                 /*!< Flag to indicate user context frame is ready for tx */
    volatile bool                bProcessTaskRequestFramePending;                          /*!< Flag to indicate process task context frame is ready for tx */
    volatile bool                bConnectionRequestPending;                                /*!< Flag to indicate connect request should be generated from process task context */
    volatile bool                bInUse;                                                   /*!< Flag to indicate if we have submitted a SPI transaction but are yet to receive a callback */
    bool                         bConnected;                                               /*!< Current state of manager connection */
    bool                         bPreviousTxWasUserFrame;                                  /*!< Flag to indicate the previous outgoing frame was a user frame for ping-pong */
    bool                         bInitialized;                                             /*!< Flag to indicate that a SPI transaction has been performed on this port */
    bool                         bLinkAvailable;                                           /*!< Flag to indicate that the manager on this SPI link is available */
    bool                         bTransmitted;                                             /*!< Flag to indicate that data was transmitted on this port */
} adi_wil_port_internal_t;

/**
 * @brief   Port structure
 */
struct adi_wil_port_t
{
    adi_wil_port_internal_t Internals;                              /*!< Port internals */
    uint8_t                 iSPIDevice;                             /*!< Physical SPI Dev of the port */
    uint8_t                 iChipSelect;                            /*!< Physical CS of the port */
};

#endif //ADI_WIL_PORT__H


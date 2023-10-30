/******************************************************************************
 * @file     adi_wil_pack_internals.h
 *
 * @brief    WBMS non-safety pack internals structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_PACK_INTERNALS__H
#define ADI_WIL_PACK_INTERNALS__H

#include "adi_wil_types.h"
#include "adi_wil_script_change.h"
#include "adi_wil_sensor_data_buffer.h"
#include "adi_wil_network_data_buffer.h"
#include "adi_wil_contextual_data.h"
#include "adi_wil_acl.h"
#include "adi_wil_xms_parameters.h"
#include "adi_wil_file_crc.h"
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Enumerations
 *****************************************************************************/
/**
* @brief   LoadFile states
*/
typedef enum
{
    ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER,                /*!< No transfer file state */
    ADI_WIL_LOAD_FILE_STATE_DOWNLOAD,                   /*!< File transfer state */
    ADI_WIL_LOAD_FILE_STATE_RETRANSMIT,                 /*!< File retransmit state */
    ADI_WIL_LOAD_FILE_STATE_HANDSHAKING,                /*!< Handshaking has not finished yet. */
    ADI_WIL_LOAD_FILE_STATE_TRANSFER,                   /*!< Handshaking has finished - proceed to data TX phase. */
} adi_wil_transfer_state_t;

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/
/**
 * @brief State of the node connection
 */
typedef struct
{
    adi_wil_device_t iConnectState;                     /*!< Node connection information */
    uint8_t iCount;                                     /*!< Number of valid nodes in system */
} adi_wil_node_state_t;

/**
 * @brief State of connect/disconnect command state machine
 */
typedef struct
{
    uint32_t iManager0ConfigurationHash;                /*!< Configuration hash returned by manager 1 */
    uint32_t iManager1ConfigurationHash;                /*!< Configuration hash returned by manager 2 */
    uint32_t iTimeStamp;                                /*!< Timestamp used for tracking individual request timeouts */
    uint16_t iToken;                                    /*!< Token to use for connection requests */
    uint8_t iRetries;                                   /*!< Retry counter for current connection attempt */
    bool bDMHActive;                                    /*!< Boolean flag indicating if DMH is active or not */
    bool bInProgress;                                   /*!< Boolean flag indicating if a connection is in progress */
    bool bReconnection;                                 /*!< Boolean flag indicating if this is a user request or background request */
} adi_wil_connect_state_t;

/**
 * @brief State of SetMode command state machine
 */
typedef struct
{
    uint64_t iNodeModeMask;                             /*!< Bitmask indicating the SetMode status of the nodes */
    adi_wil_mode_t eMode;                               /*!< value of system mode to use in set mode command */
    bool bExpectManager0Response;                       /*!< Flag to indicate we are expecting a response from manager 0 */
    bool bExpectManager1Response;                       /*!< Flag to indicate we are expecting a response from manager 1 */
    bool bPendingManager0Response;                      /*!< Flag to indicate we have not received a response from manager 0 when expecting */
    bool bPendingManager1Response;                      /*!< Flag to indicate we have not received a response from manager 1 when expecting */
} adi_wil_setmode_state_t;

/**
 * @brief struct for storing state of PMS measurement
 *
 */
typedef struct
{
    adi_wil_api_process_func_t pfRequestFunc;                           /*!< Request function pointer */
    uint64_t iDeviceId;                                                 /*!< Target device ID for the request */
    uint64_t iPendingResponses;                                         /*!< Bitmask of pending responses for this request */
    adi_wil_target_t eTarget;                                           /*!< Target for the request */
    uint32_t iStartTime;                                                /*!< Time when the request was posted */
    uint32_t iTimeout;                                                  /*!< Variable to store a custom timeout value */
    uint16_t iToken;                                                    /*!< Token number to identify the request */
    uint8_t iRetries;                                                   /*!< Number of retries already performed */
    uint8_t iRequestCount;                                              /*!< Used for tracking total requests in APIs that require multiple blocks or commands to be issued */
    bool bValid;                                                        /*!< Boolean to mark request as active or not */
    bool bPartialSuccess;                                               /*!< At least one device completed the operation */
} adi_wil_cmd_state_t;

/**
 * @brief State information for load file state machine
 */
typedef struct
{
    uint64_t iMissingBlockMask1;                                        /*!< Bitmask representing missing block indices relative to base block */
    uint64_t iMissingBlockMask2;                                        /*!< Bitmask representing missing block indices relative to base block */
    adi_wil_device_t iDeviceActiveBitmask;                              /*!< A bitmask which holds the active nodes and managers */
    adi_wil_device_t iDeviceBitMaskInternalNodeCopy;                    /*!< A bitmask which holds an internal copy of the devices that need to be handshaked in unicast */
    adi_wil_device_t iDeviceBitMaskSentUnicastHS;                       /*!< A bitmask which indicates which nodes have already been sent an unicast handshake. */
    adi_wil_device_t iDeviceBitMaskReceivedUnicastHS;                   /*!< A bitmask which indicates which nodes have received handshake responses. */
    uint64_t iUpdateMap;                                                /*!< Mask for keep tracking of nodes */
    adi_wil_transfer_state_t eState;
    uint32_t iFileSize;                                                 /*!< Size of file passed in by the application*/
    uint8_t const * pData;                                              /*!< Pointer to current chunk of file data */
    uint16_t iTotalBlocks;                                              /*!< Total number of blocks in the file */
    uint16_t iBaseBlockIdx;                                             /*!< Block number of starting block in the current chunk */
    uint16_t iPreviouslyRequestedBlockNumber;                           /*!< Block number to keep track so that the same block is not requested forever */
    uint16_t iNumberOfBlkNumberRetry;                                   /*!< Block number to keep track so that the same block is not requested forever */
    uint8_t iFileType;                                                  /*!< Integer FileType representation sent during handshake */
    uint8_t iBlockIdx;                                                  /*!< Block number relative to base block number */
    uint8_t iBlocksInSector;                                            /*!< Number of blocks to transmit for this current sector */
    uint8_t iRetransmissionCount;                                       /*!< Counter to track how many retransmission attempts have occurred for a given sector */
    bool bWaitReceived;                                                 /*!< Flag to indicate that at least one device requested a re-request */
    bool bDeviceRemoved;                                                /*!< Flag to indicate at least one device was removed from the operation */
    bool bUnicastHandshaking;                                           /*!< Flag to indicate that the bits set in the 64bit bit-mask that need to be handshaked in unicast */
    bool bBroadcastHandshaking;                                         /*!< Flag to indicate that all the nodes need to be sent a broadcast handshake. */
} adi_wil_loadfile_state_t;

/**
 * @brief State variables GPIO related information
 */
typedef struct {
    adi_wil_gpio_id_t GpioId;                   /*!< GPIO ID */
    adi_wil_gpio_value_t GpioValue;             /*!< GPIO value */
} adi_wil_gpio_state_t;

/**
 * @brief  State variables for set script
 */
typedef struct {
    uint8_t iScriptId;                          /*!< Script ID */
    uint8_t iSensorId;                          /*!< Sensor ID */
} adi_wil_selectscript_state_t;

/**
 * @brief  State variables for modify script
 */
typedef struct {
    adi_wil_script_change_t ScriptChange;       /*!< Script change data */
    uint8_t iSensorId;                          /*!< Sensor ID */
} adi_wil_modifyscript_state_t;

/**
 * @brief  State variables Contextual Data
 */
typedef struct {
    adi_wil_contextual_data_t ContextualData;   /*!< Contextual data */
    uint8_t iContextualId;                      /*!< Contextual data ID */
} adi_wil_contextual_state_t;

/**
 * @brief State variables for rotate key
 */
typedef struct {
    uint8_t iDeviceId;                          /*!< Device ID */
} adi_wil_rotate_key_state_t;

/**
 * @brief State variables for set/get state of health
 */
typedef struct {
    uint8_t iPercentage;                        /*!< State of charge in percentage */
} adi_wil_state_of_health_state_t;

/**
 * @brief State variables for get file state machine
 */
typedef struct {
    uint64_t iDeviceId;                         /*!< Current requested device ID of the transfer */
    uint16_t iOffset;                           /*!< Current block offset */
    uint8_t iFileType;                          /*!< Current file type */
    bool bInProgress;                           /*!< Boolean indicating if the operation is in progress */
} adi_wil_get_file_state_t;

/**
 * @brief State variables for erase container state machine
 */
typedef struct {
    uint8_t iFileType;                          /*!< Current file type */
} adi_wil_erase_file_state_t;

/**
 *@brief State variables for entering and exiting inventory state
 */
typedef struct {
    uint64_t iCurrentTime;                      /*!< Current UNIX time */
    uint8_t iExitInventoryStateFlag;            /*!< 0 = Enter inventory state; 1 = exit inventory state */
} adi_wil_inventory_transition_state_t;

/**
 * @brief State variables for get file CRC
 */
typedef struct {
    adi_wil_file_crc_list_t FileCRCList;        /*!< File CRC list to return to user */
    uint8_t iFileType;                          /*!< File Type to retrieve the CRC for */
} adi_wil_get_file_crc_state_t;

/**
 * @brief State variables for reset device
 */
typedef struct {
    uint16_t iToken;                 /*!< Job Token */
} adi_wil_reset_device_state_t;

/**
 * @brief State variables for SetCustomerIdentifier
 */
typedef struct {
    uint8_t Data[ADI_WIL_MAX_CUSTOMER_IDENTIFIER_DATA_LEN];     /*!< Identifier data */
    uint8_t iCustomerIdentifier;                                /*!< Customer identifier */
    uint8_t iLength;                                            /*!< Length of the identifier data */
} adi_wil_customer_identifier_state_t;

/**
 * @brief State variables for environmental monitoring
 */
typedef struct {
    bool bEnableFaultServicing;                         /*!< Enable/disable fault servicing on manager */
} adi_wil_env_mon_state_t;

/**
 * @brief State variables for update monitor parameters
 */
typedef struct {
    uint64_t iUpdateMap;                                /*!< Bitmap for keep tracking of nodes */
    uint8_t* pData;                                     /*!< Pointer to update data */
    uint16_t iOffset;                                   /*!< Current offset into data */
    uint16_t iLength;                                   /*!< Total length of data */
    uint8_t iPreviousRequestLength;                     /*!< Length of previous request sent */
} adi_wil_update_monitor_params_state_t;

/**
 * @brief State variables for update monitor parameters
 */
typedef struct {
    adi_wil_topology_t eTopology;                       /*!< Requested topology type in use by API */
} adi_wil_apply_network_topology_state_t;

/**
 * @brief   WIL XMS timestamp state structure
 */
typedef struct {
    uint32_t iTimestamp;                                /*!< Current timestamp extracted from a measurement */
    bool bInitialized;                                  /*!< Flag indicating if we've populated the iTimestamp field */
    bool bPendingChange;                                /*!< Flag indicating if the iTimestamp field must change due to a new interval */
} adi_wil_xms_timestamp_state_t;

/**
 * @brief   WIL XMS timestamp conversion state structure
 */
typedef struct {
    adi_wil_xms_timestamp_state_t TimestampState [2u];      /*!< Array of timestamp states. Index 1 for Manager 1 and index 0 for all other devices */
    uint8_t iMeasurementsReceived [ADI_WIL_MAX_DEVICES];    /*!< Array of the current count of measurements received in this interval per device */
    uint8_t iBaseSequenceNumber;                            /*!< Base sequence number for the interval that all devices should increment from */
    bool bFlushRequired;                                    /*!< Flag to indicate that the sequence number has been rolled forward so flush any packets currently buffered */
} adi_wil_xms_timestamp_conversion_state_t;

/**
 * @brief   XMS packet statistics
 */
typedef struct {
    uint32_t iManager0PktCount;                         /*!< Total number of XMS packets received from manager 1 */
    uint32_t iManager1PktCount;                         /*!< Total number of XMS packets received from manager 2 */
    uint32_t iRejectedPktCount;                         /*!< Total number of XMS packets rejected */
} adi_wil_xms_pkt_statistics_t;

/**
 * @brief   Network data packet capturing statistics
 */
typedef struct {
    uint32_t iPktReceivedCount;                         /*!< Number of network packets received */
    uint32_t iPktCapturedCount;                         /*!< Number of network packets captured */
    uint32_t iNwkDataRdyNotifCount;                     /*!< Number of notifications generated */
} adi_wil_nwk_pkt_statistics_t;

/**
 * @brief   Loadfile statistics
 */
typedef struct {
    uint16_t iHandshakeRequests;                        /*!< Number of handshake requests sent */
    uint16_t iStatusRequests;                           /*!< Number of status requests sent */
    uint16_t iCommitRequests;                           /*!< Number of commit requests sent */
    uint16_t iHandshakeMaxRetries;                      /*!< Handshake retry watermark */
    uint16_t iStatusMaxRetries;                         /*!< Status retry watermark */
    uint16_t iCommitMaxRetries;                         /*!< Commit retry watermark */
    uint16_t iDataBlocks;                               /*!< Number of data blocks sent */
    uint16_t iRetransmittedBlocks;                      /*!< Number of data blocks retransmitted */
    uint16_t iHandshakePhaseFails;                      /*!< Number of non-success responses received in handshake phase */
    uint16_t iDataPhaseFails;                           /*!< Number of non-success responses received in data phase */
    uint16_t iStatusPhaseFails;                         /*!< Number of non-success responses received in status phase */
    uint16_t iCommitPhaseFails;                         /*!< Number of non-success responses received in commit phase */
    uint16_t iCommitCrcFails;                           /*!< Number of CRC errors encountered in commit phase */
    uint16_t iCommitFileRejected;                       /*!< Number of file rejected errors encountered in commit phase */
} adi_wil_otap_statistics_t;

/**
 * @brief   Manager application and SPI driver status
 */
typedef struct {
    uint8_t iDeviceId;                   /*!< ID of manager device that produced the report */
    uint64_t iPacketGenerationTime;      /*!< Packet generation time in ASN */
    uint16_t iSWVersionMajor;            /*!< Major software version */
    uint16_t iSWVersionMinor;            /*!< Minor software version */
    uint32_t iSpiTxQueueOFCount;         /*!< SPIS Tx queue overflow counter */
    uint32_t iSpiRxCrcErrorCount;        /*!< SPIS Tx CRC error counter */
    uint32_t iSpiDevXferErrorCount;      /*!< Number of SPI transfers failed due to SPI peripheral error */
    uint32_t iSpiSWXferErrorCount;       /*!< Number of SPI transfers failed due to SPI driver error */
    uint32_t iSpiAbortCount;             /*!< SPIS transfer abort counter */
    uint32_t iSpiTxFrameAllocErrorCount; /*!< SPIS Tx frame memory allocation error counter */
    uint32_t iSpiTxMsgAllocErrorCount;   /*!< SPIS Tx message memory allocation error counter */
    uint32_t iSpiRxFrameAllocErrorCount; /*!< SPIS Rx frame memory allocation error counter */
    uint32_t iSpiRxMsgAllocErrorCount;   /*!< SPIS Rx message memory allocation error counter */
    uint32_t iIdleFramesSinceLastMsg;    /*!< Number of idles frames sent since last status message */
    uint16_t iFlash0OneBitEccErrCount;   /*!< Flash 0 Single bit ECC error count (SBE) */
    uint16_t iFlash0TwoBitEccErrCount;   /*!< Flash 0 Double bit ECC error count (DBE) */
    uint32_t iFlash0LastEccErrAddr;      /*!< Flash 0 ECC error address */
    uint16_t iFlash1OneBitEccErrCount;   /*!< Flash 1 Single bit ECC error count (SBE) */
    uint16_t iFlash1TwoBitEccErrCount;   /*!< Flash 1 Double bit ECC error count (DBE) */
    uint32_t iFlash1LastEccErrAddr;      /*!< Flash 1 ECC error address */
    uint16_t iFlashWriteErrCount;        /*!< Flash program error count */
    uint8_t  iLffsFreePercent;           /*!< LFFS free bytes in percentage */
    uint16_t iLffsDefragCount;           /*!< LFFS defragmentation count */
    uint32_t iLffsStatus;                /*!< LFFS error status */
} adi_wil_mgr_status_msg_t;

/**
 * @brief   Manager statistics
 */
typedef struct {
    adi_wil_mgr_status_msg_t Manager0StatusMsg; /*!< System status message from manager 0 */
    adi_wil_mgr_status_msg_t Manager1StatusMsg; /*!< System status message from manager 1 */
} adi_wil_mgr_statistics_t;

/**
 * @brief   statistics structure
 */
typedef struct {
    adi_wil_xms_pkt_statistics_t PmsPktStats;                           /*!< BMS data statistics */
    adi_wil_xms_pkt_statistics_t BmsPktStats;                           /*!< PMS data statistics */
    adi_wil_xms_pkt_statistics_t EmsPktStats;                           /*!< EMS data statistics */
    adi_wil_nwk_pkt_statistics_t NetworkPktStats;                       /*!< Network data capturing statistics */
    adi_wil_otap_statistics_t OTAPStats;                                /*!< HR data statistics */
    adi_wil_mgr_statistics_t MgrStats;                                  /*!< Manager statistics */
} adi_wil_statistics_t;

/**
 * @brief State variables containing current state of instance
 */
struct adi_wil_pack_internals_t
{
    adi_wil_cmd_state_t UserRequestState;                                                             /*!< Current user request state */
    adi_wil_node_state_t NodeState;                                                                   /*!< current ACL list with node states */
    adi_wil_connect_state_t ConnectState;                                                             /*!< connect state */
    adi_wil_setmode_state_t SetModeState;                                                             /*!< state of SetMode WIL API state machine */
    adi_wil_loadfile_state_t LoadFileState;                                                           /*!< state of sending file state machine */
    adi_wil_gpio_state_t GpioState;                                                                   /*!< GPIO state */
    adi_wil_selectscript_state_t SelectScriptState;                                                   /*!< Select script state */
    adi_wil_modifyscript_state_t ModifyScriptState;                                                   /*!< Modify script state */
    adi_wil_contextual_state_t ContextualState;                                                       /*!< Contextual data state */
    adi_wil_rotate_key_state_t RotateKeyState;                                                        /*!< Rotate key state */
    adi_wil_state_of_health_state_t StateOfHealthState;                                               /*!< SOH state */
    adi_wil_network_data_buffer_t NetDataBuffer;                                                      /*!< Network metadata buffer */
    adi_wil_get_file_state_t GetFileState;                                                            /*!< Get file state */
    adi_wil_erase_file_state_t EraseFileState;                                                        /*!< Erase file state */
    adi_wil_inventory_transition_state_t InventoryTransitionState;                                    /*!< Inventory transition state */
    adi_wil_get_file_crc_state_t GetFileCRCState;                                                     /*!< Get File CRC state */
    adi_wil_customer_identifier_state_t CustomerIdentifierState;                                      /*!< Set customer identifier State */
    adi_wil_reset_device_state_t ResetDeviceState;                                                    /*!< Reset device state */
    adi_wil_env_mon_state_t EnvMonState;                                                              /*!< Environmental monitoring related state */
    adi_wil_update_monitor_params_state_t UpdateMonParamsState;                                       /*!< Update monitor mode parameters state */
    adi_wil_apply_network_topology_state_t ApplyNetworkTopologyState;                                 /*!< Apply Network Topology State */
    adi_wil_acl_t ACLState;                                                                           /*!< Get/Set ACL State */
    adi_wil_xms_parameters_t XmsMeasurementParameters;                                                /*!< XMS measurement parameters */
    adi_wil_xms_timestamp_conversion_state_t BmsTimestampConversionState;                             /*!< BMS timestamp conversion state */
    adi_wil_xms_timestamp_conversion_state_t PmsTimestampConversionState;                             /*!< PMS timestamp conversion state */
    adi_wil_xms_timestamp_conversion_state_t EmsTimestampConversionState;                             /*!< EMS timestamp conversion state */
    adi_wil_statistics_t Stats;                                                                       /*!< Statistics */
    adi_wil_port_t * pManager0Port;                                                                   /*!< Manager 0 port pointer */
    adi_wil_port_t * pManager1Port;                                                                   /*!< Manager 1 port pointer */
    adi_wil_port_t * pCurrentPort;                                                                    /*!< Current port to call SendData with - stripe between managers in dual manager mode */
    adi_wil_pack_t const * pPack;                                                                     /*!< Reference only to parent pack - not de-referencable in non-safety code */
    adi_wil_mode_t eMode;                                                                             /*!< current system mode */
    uint8_t iMaxNodeCount;                                                                            /*!< Maximum number of nodes in the system. */
    bool bMaintainConnection;                                                                         /*!< flag to indicate whether to maintain connection or not */
    bool bReconnectPending;                                                                           /*!< flag to indicate whether to call Reconnect on next ProcessTask invocation */
    bool bInvalidState;                                                                               /*!< Boolean flag indicating if we logged in even though the system is in an invalid configured state */
};

#endif //ADI_WIL_PACK_INTERNALS__H

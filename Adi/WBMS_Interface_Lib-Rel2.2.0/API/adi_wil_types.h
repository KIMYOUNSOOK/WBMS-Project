/******************************************************************************
 * @file     adi_wil_types.h
 *
 * @brief    WBMS Interface Library data type header.
 *
 * @details  Contains API data type declarations for the WBMS Interface Library.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_TYPES__H
#define ADI_WIL_TYPES__H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * #defines
 *****************************************************************************/

/**
 * @brief   Maximum number of network managers allowed in each pack
 */
#define ADI_WIL_NUM_NW_MANAGERS   (2u)

/**
 * @brief   Maximum number of nodes
 */
#define ADI_WIL_MAX_NODES   (62u)

/**
 * @brief   Maximum number of devices
 */
#define ADI_WIL_MAX_DEVICES   (ADI_WIL_MAX_NODES + ADI_WIL_NUM_NW_MANAGERS)

/**
 * @brief Maximum size of change data length
 */
#define ADI_WIL_MAX_SCRIPT_CHANGE_DATA_LENGTH (64u)

/**
* @brief   SPI transaction size in bytes
*/
#define ADI_WIL_SPI_TRANSACTION_SIZE (256u)

/**
* @brief   Number of acknowledgments able to be buffered for each port
*/
#define ADI_WIL_ACK_QUEUE_COUNT  (32u)

/**
 * @brief Default timeout value for responses in ms
 */
#define ADI_WIL_RESPONSE_TIMEOUT_MS (1000u)

/**
 * @brief Maximum retries for an API request before returning timeout/failure
 */
#define ADI_WIL_RETRIES (3u)

 /**
* @brief Maximum time in ms that an API can be in progress for
*/
#define ADI_WIL_MAX_API_TIMEOUT (ADI_WIL_RESPONSE_TIMEOUT_MS * ADI_WIL_RETRIES)

/**
* @brief Length of the missing block bitmap in bytes for OTAP
*/
#define  ADI_WIL_DEVICE_MASK_LEN (8u)

/**
* @brief Maximum size of the OEM contextual data in bytes
*/
#define ADI_WIL_CONTEXTUAL_DATA_SIZE (75u)

/**
* @brief Maximum size of customer identifier data
*/
#define ADI_WIL_MAX_CUSTOMER_IDENTIFIER_DATA_LEN (31u)

/**
* @brief Maximum size of the sensor data payload
*/
#define ADI_WIL_SENSOR_DATA_SIZE (80u)

/**
* @brief Maximum size of the health report data
*/
#define ADI_WIL_HR_DATA_SIZE (80u)

/**
* @brief Size of a MAC address in bytes
*/
#define ADI_WIL_MAC_ADDR_SIZE (8u)

/**
 * @brief   The size of LoadFile data block size in bytes
 */
#define ADI_WIL_LOADFILE_DATA_SIZE (64u)

/**
 * @brief   The size of the plausibility fault channel bitmap
 */
#define ADI_WIL_FAULT_CHANNELS_SIZE (24u)

/**
 * @brief   Bitmask indicating a plausibility fault has occurred
 */
#define ADI_WIL_FAULT_MASK_PLAUSIBILITY (0x0001u)

/**
 * @brief   Bitmask indicating a sensor communication fault has occurred
 */
#define ADI_WIL_FAULT_MASK_SENSOR_COMMS (0x0002u)

/**
 * @brief   Bitmask indicating a control flow fault has occurred
 */
#define ADI_WIL_FAULT_MASK_CONTROL_FLOW (0x0004u)

/*
 * @brief   Bitmask indicating a manager-to-manager fault has occurred
 */
#define ADI_WIL_FAULT_MASK_M2M_COMMS (0x0100u)

/*
 * @brief   Bitmask indicating the fault servicing has been disabled
 */
#define ADI_WIL_FAULT_MASK_SERVICE_DISABLE (0x0200u)

/*
 * @brief   Bitmask indicating the event was generated due to a periodic assertion
 */
#define ADI_WIL_FAULT_MASK_PERIODIC_ASSERTION (0x0400u)

/**
 * @brief   Bitmask indicating a network communication fault has occurred
 */
#define ADI_WIL_FAULT_MASK_NETWORK_COMMS (0x0800u)

 /**
 * @brief  Length of the nonce used for SPI encryption in bytes
 */
#define ADI_WIL_NONCE_SIZE (13u)

/**
 * @brief  Byte offset of the start of data in a file retrieved by GetFile API
 */
#define ADI_WIL_GETFILE_DATA_OFFSET (36u)

/**
 * @brief  Maximum number of cells supported by the system
 */
#define ADI_WIL_MAX_CELLS (64u)

/**
 * @brief   Maximum length of time (in milliseconds) between submission of measurement ensembles.
 */
#define ADI_WIL_MEASUREMENT_TIMEOUT_MS (250u)


#define ADI_WIL_DEV_NODE_0         (0x0000000000000001u)     /*!< Node 0 */
#define ADI_WIL_DEV_NODE_1         (0x0000000000000002u)     /*!< Node 1 */
#define ADI_WIL_DEV_NODE_2         (0x0000000000000004u)     /*!< Node 2 */
#define ADI_WIL_DEV_NODE_3         (0x0000000000000008u)     /*!< Node 3 */
#define ADI_WIL_DEV_NODE_4         (0x0000000000000010u)     /*!< Node 4 */
#define ADI_WIL_DEV_NODE_5         (0x0000000000000020u)     /*!< Node 5 */
#define ADI_WIL_DEV_NODE_6         (0x0000000000000040u)     /*!< Node 6 */
#define ADI_WIL_DEV_NODE_7         (0x0000000000000080u)     /*!< Node 7 */
#define ADI_WIL_DEV_NODE_8         (0x0000000000000100u)     /*!< Node 8 */
#define ADI_WIL_DEV_NODE_9         (0x0000000000000200u)     /*!< Node 9 */
#define ADI_WIL_DEV_NODE_10        (0x0000000000000400u)     /*!< Node 10 */
#define ADI_WIL_DEV_NODE_11        (0x0000000000000800u)     /*!< Node 11 */
#define ADI_WIL_DEV_NODE_12        (0x0000000000001000u)     /*!< Node 12 */
#define ADI_WIL_DEV_NODE_13        (0x0000000000002000u)     /*!< Node 13 */
#define ADI_WIL_DEV_NODE_14        (0x0000000000004000u)     /*!< Node 14 */
#define ADI_WIL_DEV_NODE_15        (0x0000000000008000u)     /*!< Node 15 */
#define ADI_WIL_DEV_NODE_16        (0x0000000000010000u)     /*!< Node 16 */
#define ADI_WIL_DEV_NODE_17        (0x0000000000020000u)     /*!< Node 17 */
#define ADI_WIL_DEV_NODE_18        (0x0000000000040000u)     /*!< Node 18 */
#define ADI_WIL_DEV_NODE_19        (0x0000000000080000u)     /*!< Node 19 */
#define ADI_WIL_DEV_NODE_20        (0x0000000000100000u)     /*!< Node 20 */
#define ADI_WIL_DEV_NODE_21        (0x0000000000200000u)     /*!< Node 21 */
#define ADI_WIL_DEV_NODE_22        (0x0000000000400000u)     /*!< Node 22 */
#define ADI_WIL_DEV_NODE_23        (0x0000000000800000u)     /*!< Node 23 */
#define ADI_WIL_DEV_NODE_24        (0x0000000001000000u)     /*!< Node 24 */
#define ADI_WIL_DEV_NODE_25        (0x0000000002000000u)     /*!< Node 25 */
#define ADI_WIL_DEV_NODE_26        (0x0000000004000000u)     /*!< Node 26 */
#define ADI_WIL_DEV_NODE_27        (0x0000000008000000u)     /*!< Node 27 */
#define ADI_WIL_DEV_NODE_28        (0x0000000010000000u)     /*!< Node 28 */
#define ADI_WIL_DEV_NODE_29        (0x0000000020000000u)     /*!< Node 29 */
#define ADI_WIL_DEV_NODE_30        (0x0000000040000000u)     /*!< Node 30 */
#define ADI_WIL_DEV_NODE_31        (0x0000000080000000u)     /*!< Node 31 */
#define ADI_WIL_DEV_NODE_32        (0x0000000100000000u)     /*!< Node 32 */
#define ADI_WIL_DEV_NODE_33        (0x0000000200000000u)     /*!< Node 33 */
#define ADI_WIL_DEV_NODE_34        (0x0000000400000000u)     /*!< Node 34 */
#define ADI_WIL_DEV_NODE_35        (0x0000000800000000u)     /*!< Node 35 */
#define ADI_WIL_DEV_NODE_36        (0x0000001000000000u)     /*!< Node 36 */
#define ADI_WIL_DEV_NODE_37        (0x0000002000000000u)     /*!< Node 37 */
#define ADI_WIL_DEV_NODE_38        (0x0000004000000000u)     /*!< Node 38 */
#define ADI_WIL_DEV_NODE_39        (0x0000008000000000u)     /*!< Node 39 */
#define ADI_WIL_DEV_NODE_40        (0x0000010000000000u)     /*!< Node 40 */
#define ADI_WIL_DEV_NODE_41        (0x0000020000000000u)     /*!< Node 41 */
#define ADI_WIL_DEV_NODE_42        (0x0000040000000000u)     /*!< Node 42 */
#define ADI_WIL_DEV_NODE_43        (0x0000080000000000u)     /*!< Node 43 */
#define ADI_WIL_DEV_NODE_44        (0x0000100000000000u)     /*!< Node 44 */
#define ADI_WIL_DEV_NODE_45        (0x0000200000000000u)     /*!< Node 45 */
#define ADI_WIL_DEV_NODE_46        (0x0000400000000000u)     /*!< Node 46 */
#define ADI_WIL_DEV_NODE_47        (0x0000800000000000u)     /*!< Node 47 */
#define ADI_WIL_DEV_NODE_48        (0x0001000000000000u)     /*!< Node 48 */
#define ADI_WIL_DEV_NODE_49        (0x0002000000000000u)     /*!< Node 49 */
#define ADI_WIL_DEV_NODE_50        (0x0004000000000000u)     /*!< Node 50 */
#define ADI_WIL_DEV_NODE_51        (0x0008000000000000u)     /*!< Node 51 */
#define ADI_WIL_DEV_NODE_52        (0x0010000000000000u)     /*!< Node 52 */
#define ADI_WIL_DEV_NODE_53        (0x0020000000000000u)     /*!< Node 53 */
#define ADI_WIL_DEV_NODE_54        (0x0040000000000000u)     /*!< Node 54 */
#define ADI_WIL_DEV_NODE_55        (0x0080000000000000u)     /*!< Node 55 */
#define ADI_WIL_DEV_NODE_56        (0x0100000000000000u)     /*!< Node 56 */
#define ADI_WIL_DEV_NODE_57        (0x0200000000000000u)     /*!< Node 57 */
#define ADI_WIL_DEV_NODE_58        (0x0400000000000000u)     /*!< Node 58 */
#define ADI_WIL_DEV_NODE_59        (0x0800000000000000u)     /*!< Node 59 */
#define ADI_WIL_DEV_NODE_60        (0x1000000000000000u)     /*!< Node 60 */
#define ADI_WIL_DEV_NODE_61        (0x2000000000000000u)     /*!< Node 61 */
#define ADI_WIL_DEV_MANAGER_0      (0x4000000000000000u)     /*!< Manager 0 */
#define ADI_WIL_DEV_MANAGER_1      (0x8000000000000000u)     /*!< Manager 1 */
#define ADI_WIL_DEV_ALL_MANAGERS   (0xC000000000000000u)     /*!< All managers */
#define ADI_WIL_DEV_ALL_NODES      (0x3FFFFFFFFFFFFFFFu)     /*!< All nodes */


/**
 * @brief Maximum length of application payload that can be sent to a node
 */
#define ADI_WIL_MAX_APP_PAYLOAD_SIZE (80u)

/**
 * @brief Interval in ms of the heartbeat message sent to a Safety CPU
 */
#define ADI_WIL_SAFETY_HEARTBEAT_TIME (300u)

/**
 * @brief Maximum size of fault message received from a safety CPU
 */
#define ADI_WIL_MAX_SAFETY_CPU_FAULT_MESSAGE_SIZE     (71u)

/******************************************************************************
 * Enumerations
 *******************************************************************************/

/**
 * @brief   WBMS Interface Library API
 *
 * @details This enumeration is used to indicate which API is returning via the
 *          user supplied callback method.
 */
typedef enum
{
    ADI_WIL_API_INITIALIZE,
    ADI_WIL_API_TERMINATE,
    ADI_WIL_API_CONNECT,
    ADI_WIL_API_DISCONNECT,
    ADI_WIL_API_SET_MODE,
    ADI_WIL_API_GET_MODE,
    ADI_WIL_API_SET_ACL,
    ADI_WIL_API_GET_ACL,
    ADI_WIL_API_QUERY_DEVICE,
    ADI_WIL_API_GET_NETWORK_STATUS,
    ADI_WIL_API_LOAD_FILE,
    ADI_WIL_API_ERASE_FILE,
    ADI_WIL_API_GET_DEVICE_VERSION,
    ADI_WIL_API_GET_FILE_CRC,
    ADI_WIL_API_SET_GPIO,
    ADI_WIL_API_GET_GPIO,
    ADI_WIL_API_SELECT_SCRIPT,
    ADI_WIL_API_MODIFY_SCRIPT,
    ADI_WIL_API_ENTER_INVENTORY_STATE,
    ADI_WIL_API_GET_FILE,
    ADI_WIL_API_GET_WIL_SOFTWARE_VERSION,
    ADI_WIL_API_PROCESS_TASK,
    ADI_WIL_API_SET_CONTEXTUAL_DATA,
    ADI_WIL_API_GET_CONTEXTUAL_DATA,
    ADI_WIL_API_RESET_DEVICE,
    ADI_WIL_API_SET_STATE_OF_HEALTH,
    ADI_WIL_API_GET_STATE_OF_HEALTH,
    ADI_WIL_API_ENABLE_FAULT_SERVICING,
    ADI_WIL_API_ROTATE_KEY,
    ADI_WIL_API_SET_CUSTOMER_IDENTIFIER_DATA,
    ADI_WIL_API_ENABLE_NETWORK_DATA_CAPTURE,
    ADI_WIL_API_UPDATE_MONITOR_PARAMETERS,
    ADI_WIL_API_GET_MONITOR_PARAMETERS_CRC,
    ADI_WIL_API_ASSESS_NETWORK_TOPOLOGY,
    ADI_WIL_API_APPLY_NETWORK_TOPOLOGY,
    ADI_WIL_API_CONFIGURE_CELL_BALANCING,
    ADI_WIL_API_GET_CELL_BALANCING_STATUS
} adi_wil_api_t;

/**
 * @brief   WBMS error code
 *
 * @details These enumerations provide information on the outcome of a WIL operation.
 */
typedef enum
{
    ADI_WIL_ERR_SUCCESS,                                        /*!< Operation successful */
    ADI_WIL_ERR_FAIL,                                           /*!< The operation failed */
    ADI_WIL_ERR_API_IN_PROGRESS,                                /*!< The operation could not be completed because another operation is currently in progress */
    ADI_WIL_ERR_TIMEOUT,                                        /*!< The operation has timed out */
    ADI_WIL_ERR_NOT_CONNECTED,                                  /*!< The operation could not be completed because there is no active connection to the network managers */
    ADI_WIL_ERR_INVALID_PARAMETER,                              /*!< An invalid parameter has been specified */
    ADI_WIL_ERR_INVALID_STATE,                                  /*!< The system was in an invalid state when processing the request was made */
    ADI_WIL_ERR_NOT_SUPPORTED,                                  /*!< The requested feature is not supported */
    ADI_WIL_ERR_EXTERNAL,                                       /*!< An error has been returned from a HAL or OSAL component */
    ADI_WIL_ERR_INVALID_MODE,                                   /*!< The requested/current system mode is invalid */
    ADI_WIL_ERR_IN_PROGRESS,                                    /*!< Requesting a block of data from the user during the file loading process */
    ADI_WIL_ERR_CONFIGURATION_MISMATCH,                         /*!< The two managers have mismatched configurations */
    ADI_WIL_ERR_CRC,                                            /*!< The operation encountered a CRC error */
    ADI_WIL_ERR_FILE_REJECTED,                                  /*!< The file was rejected by the end device */
    ADI_WIL_ERR_PARTIAL_SUCCESS                                 /*!< Some devices failed complete the operation while others were successful */
} adi_wil_err_t;

/**
 * @brief   System modes
 *
 * @details Available system modes:
 *          - Standby mode: no data measurement is active
 *          - Commission mode: used for forming a network
 *          - Active mode: all data measurements are active
 *          - Monitoring mode: Environmental Monitoring is active
 *          - OTAP mode: used to load firmware updates to devices
 *          - Sleep Mode: Ultra low power mode for long periods of inactivity
 */
typedef enum
{
    ADI_WIL_MODE_STANDBY = 1,                                   /*!< Standby mode */
    ADI_WIL_MODE_COMMISSIONING,                                 /*!< Commissioning mode */
    ADI_WIL_MODE_ACTIVE,                                        /*!< Active mode */
    ADI_WIL_MODE_MONITORING,                                    /*!< Monitoring mode */
    ADI_WIL_MODE_OTAP,                                          /*!< OTAP mode */
    ADI_WIL_MODE_SLEEP,                                         /*!< Sleep mode */
} adi_wil_mode_t;

/**
 * @brief   Target Types
 *
 * @details These enumerations are used internally by the WIL to address different
 *          types of device targets.
 */
typedef enum
{
    ADI_WIL_TARGET_SINGLE_MANAGER,
    ADI_WIL_TARGET_SINGLE_NODE,
    ADI_WIL_TARGET_ALL_MANAGERS,
    ADI_WIL_TARGET_ALL_NODES,
} adi_wil_target_t;

/**
 * @brief   Notification IDs
 *
 * @details When an asynchronous event (not triggered by an API call) occurs, the
 *          event callback (of type {@link adi_wil_event_cb}) is called with one of
 *          the following events passed in via the argument Event code. Some events
 *          are triggered with data which are passed in via the pData argument. For
 *          those events that do not carry data, the pData argument is set to NULL.
 */
typedef enum
{
    ADI_WIL_EVENT_COMM_NODE_CONNECTED,                          /*!< A node joined the WBMS network, connected node device ID ({@link adi_wil_device_t} *) is returned */
    ADI_WIL_EVENT_COMM_NODE_DISCONNECTED,                       /*!< A node dropped from the WBMS network, disconnected node device ID ({@link adi_wil_device_t} *) is returned */
    ADI_WIL_EVENT_COMM_MGR_CONNECTED,                           /*!< Communication with a network manager was established, connected device ID ({@link adi_wil_device_t} *) is returned */
    ADI_WIL_EVENT_COMM_MGR_DISCONNECTED,                        /*!< Communication with a network manager was lost, disconnected device ID ({@link adi_wil_device_t} *) is returned */
    ADI_WIL_EVENT_COMM_MGR_TO_MGR_ERROR,                        /*!< Communication between the two network managers failed (dual manager mode only), no data is returned */
    ADI_WIL_EVENT_COMM_SAFETY_CPU_CONNECTED,                    /*!< A Connect message was received from a Safety CPU. Safety CPU Connect message ({@link adi_wil_connect_safety_cpu_t} *) is returned */
    ADI_WIL_EVENT_COMM_SAFETY_CPU_DISCONNECTED,                 /*!< A Safety CPU was disconnected. Device ID ({@link adi_wil_device_t} *) of the disconnected device is returned */
    ADI_WIL_EVENT_DATA_READY_BMS,                               /*!< BMS data is ready for extraction. Sensor data buffer (adi_wil_sensor_data_buffer_t *) is returned. The client must copy out the data during the event notification callback */
    ADI_WIL_EVENT_DATA_READY_PMS,                               /*!< PMS data is ready for extraction. Sensor data buffer (adi_wil_sensor_data_buffer_t *) is returned. The client must copy out the data during the event notification callback */
    ADI_WIL_EVENT_DATA_READY_EMS,                               /*!< Environmental monitoring data is ready for extraction. Sensor data buffer (adi_wil_sensor_data_buffer_t *) is returned. The client must copy out the data during the event notification callback */
    ADI_WIL_EVENT_DATA_READY_HEALTH_REPORT,                     /*!< Health report is ready for extraction. HR data (adi_wil_health_report_t *) is returned. The client must copy of the data during the event notification callback */
    ADI_WIL_EVENT_DATA_READY_NETWORK_DATA,                      /*!< Event indicating enough network metadata has been accumulated and is ready for extraction, network data buffer (adi_wil_network_data_buffer_t *) is returned */
    ADI_WIL_EVENT_FAULT_SOURCES,                                /*!< A fault in monitor mode has occurred and a system summary has been received, the associated system fault summary ({@link adi_wil_device_t} *) is returned */
    ADI_WIL_EVENT_FAULT_REPORT,                                 /*!< An Environmental monitoring fault report has been received, the associated device fault report ({@link adi_wil_fault_report_t} *) is returned */
    ADI_WIL_EVENT_FAULT_LOCK_RELEASE,                           /*!< A lock in the system has been held for longer than the maximum expected time. */
    ADI_WIL_EVENT_FAULT_SAFETY_CPU,                             /*!< Fault summary data received from a Safety CPU, Fault message ({@link adi_wil_fault_safety_cpu_t} *) is returned */
    ADI_WIL_EVENT_FAULT_SCL_TX_MESSAGE_EXPIRED,                 /*!< A message from the SCL was not retrieved before expiring. This can occur when the adi_wil_ProcessTask API is not invoked at the specified rate */
    ADI_WIL_EVENT_FAULT_SCL_VALIDATION,                         /*!< An issue was detected while validating a message received from a Safety CPU */
    ADI_WIL_EVENT_SEC_NODE_NOT_IN_ACL,                          /*!< Node that tried to join the WBMS is not in the ACL list, the MAC address of the unidentified node (uint8_t *) is returned */
    ADI_WIL_EVENT_SEC_CERTIFICATE_CALCULATION_ERROR,            /*!< A certificate calculation error occurred, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_JOIN_NO_KEY,                              /*!< A node does not have a valid join key, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_JOIN_DUPLICATE_JOIN_COUNTER,              /*!< A node has a duplicate join counter, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_JOIN_MIC_FAILED,                          /*!< MIC verification failed for a join packet, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_UNKNOWN_ERROR,                            /*!< Unknown security error, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_XFER_DEVICE_REMOVED,                          /*!< A device was removed from the transfer process. device ID ({@link adi_wil_device_removed_t} *) is returned */
    ADI_WIL_EVENT_INSUFFICIENT_BUFFER,                          /*!< The user has provided a adi_wil_sensor_data_t ({@link adi_wil_sensor_data_t} *) buffer that is insufficiently sized for this network */
    ADI_WIL_EVENT_SEC_SESSION_MIC_FAILED,                       /*!< Session MIC Failure for a session packet, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_M2M_JOIN_CNTR_ERROR,                      /*!< M2M Join Counter Security Error, the MAC address of the peer manager (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_M2M_SESSION_CNTR_ERROR,                   /*!< M2M Session Counter Security Error, the MAC address of the peer manager (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_SEC_CERTIFICATE_EXCHANGE_LOCK_ERROR,          /*!< A certificate exchange error occurred, the MAC address of the device (uint8_t *) that triggered the event is returned */
    ADI_WIL_EVENT_MGR_QUEUE_OVERFLOW,                           /*!< An overflow occurred in the Manager TX SPI queue. Manager device ID ({@link adi_wil_device_t} *) is returned. */
    ADI_WIL_EVENT_XMS_DUPLICATE,                                /*!< A duplicate measurement has been received. */
    ADI_WIL_EVENT_XMS_INSERTION,                                /*!< A measurement has been received from an invalid or disallowed entity. */
    ADI_WIL_EVENT_XMS_DELAY,                                    /*!< A measurement has been received from a previous measurement interval. */
    ADI_WIL_EVENT_XMS_DROPPED,                                  /*!< A measurement has been received but the device was not authenticated before the measurement interval began. */
    ADI_WIL_EVENT_TOPOLOGY_ASSESSMENT_COMPLETE,                 /*!< A topology assessment has completed. ({@link adi_wil_topology_assessment_t} *) is returned. */
    ADI_WIL_EVENT_TOPOLOGY_APPLICATION_COMPLETE,                /*!< A topology application has completed. ({@link adi_wil_err_t} *) is returned. */
    ADI_WIL_EVENT_NODE_MODE_MISMATCH,                           /*!< The node is in the incorrect operational/OTAP mode. Node device ID ({@link adi_wil_device_t} *) is returned  */
    ADI_WIL_EVENT_CELL_BALANCING_STATUS                         /*!< A cell balancing status message has been received. ({@link adi_wil_cell_balancing_status_t} *) is returned  */
} adi_wil_event_id_t;

/**
 * @brief   GPIO pin identifier
 *
 * @details GPIO pins that are available for use to the host application.
 *
 */
typedef enum
{
    ADI_WIL_GPIO_0,                                             /*!< GPIO Pin 0 */
    ADI_WIL_GPIO_2,                                             /*!< GPIO Pin 2 */
    ADI_WIL_GPIO_7,                                             /*!< GPIO Pin 7 */
    ADI_WIL_GPIO_8,                                             /*!< GPIO Pin 8 */
    ADI_WIL_GPIO_9,                                             /*!< GPIO Pin 9 */
    ADI_WIL_GPIO_10                                             /*!< GPIO Pin 10 */
} adi_wil_gpio_id_t;

/**
 * @brief   GPIO pin value
 *
 * @details Valid GPIO states (either high or low).
 *
 */
typedef enum
{
    ADI_WIL_GPIO_LOW,                                           /*!< GPIO low state */
    ADI_WIL_GPIO_HIGH,                                          /*!< GPIO high state */
} adi_wil_gpio_value_t;

/**
 * @brief   Device ID used to address a device or devices on the network
 *
 * @details Used to address a device or a collection of devices on the network. For
 *          node devices, the enumeration specifies the node in terms of its location
 *          in the ACL, i.e. ADI_WIL_DEV_NODE_0 is the first node in the ACL, ADI_WIL_DEV_NODE_3
 *          is the 4th node in the ACL etc..
 */
typedef uint64_t adi_wil_device_t;


/**
 * @brief   File Types
 */
typedef enum
{
    ADI_WIL_FILE_TYPE_FIRMWARE,                                 /*!< Manager/Node software file */
    ADI_WIL_FILE_TYPE_BMS_CONTAINER,                            /*!< BMS Container file */
    ADI_WIL_FILE_TYPE_CONFIGURATION,                            /*!< Configuration file */
    ADI_WIL_FILE_TYPE_BLACK_BOX_LOG,                            /*!< Black box log file */
    ADI_WIL_FILE_TYPE_INVENTORY_LOG,                            /*!< Inventory log file */
    ADI_WIL_FILE_TYPE_ENV_MON,                                  /*!< Environmental monitoring data file */
    ADI_WIL_FILE_TYPE_PMS_CONTAINER,                            /*!< PMS Container file */
    ADI_WIL_FILE_TYPE_EMS_CONTAINER                             /*!< EMS Container file */
} adi_wil_file_type_t;

/**
 * @brief   Sensor ID Type
 *
 * @details These enumerations are used to indicate a certain sensor on
 *          a node or a manager device.
 */
typedef enum
{
    ADI_WIL_SENSOR_ID_BMS,                                      /*!< BMS sensor ID */
    ADI_WIL_SENSOR_ID_PMS,                                      /*!< PMS sensor ID */
    ADI_WIL_SENSOR_ID_EMS                                       /*!< EMS sensor ID */
} adi_wil_sensor_id_t;

/**
 * @brief   Contextual Data ID
 *
 * @details These enumerations are used to address a certain contextual data.
 *          Note that ADI_WIL_CONTEXTUAL_ID_WRITE_ONCE is only for contextual
 *          data that is to be written only once, usually some permanent data
 *          that is written during manufacturing or commissioning.
 */
typedef enum
{
    ADI_WIL_CONTEXTUAL_ID_0,                                    /*!< Contextual data ID 0 */
    ADI_WIL_CONTEXTUAL_ID_1,                                    /*!< Contextual data ID 1 */
    ADI_WIL_CONTEXTUAL_ID_WRITE_ONCE                            /*!< Write once contextual data */
} adi_wil_contextual_id_t;

/**
 * @brief   Customer identifier types
 *
 * @details These enumerations are used to specify a custom advertising identifier
 *          to use on the network.
 */
typedef enum
{
    ADI_WIL_CUSTOMER_IDENTIFIER_0,                              /*!< Customer Identifier 0 */
    ADI_WIL_CUSTOMER_IDENTIFIER_1,                              /*!< Customer Identifier 1 */
    ADI_WIL_CUSTOMER_IDENTIFIER_2                               /*!< Customer Identifier 2 */
} adi_wil_customer_identifier_t;

/**
 * @brief   Key types
 *
 * @details Keys that are rotate-able via the Rotate Key API
 */
typedef enum
{
    ADI_WIL_KEY_JOIN,                                           /*!< Join Key */
    ADI_WIL_KEY_SESSION,                                        /*!< Session key */
    ADI_WIL_KEY_NETWORK                                         /*!< Network key */
} adi_wil_key_t;

/**
 * @brief   Topology types
 *
 * @details Keys that are rotate-able via the Rotate Key API
 */
typedef enum
{
    ADI_WIL_TOPOLOGY_SAVED,                                     /*!< Apply the existing saved network topology */
    ADI_WIL_TOPOLOGY_NEW,                                       /*!< Apply the most recently assessed network topology. This will save the topology */
    ADI_WIL_TOPOLOGY_STAR                                       /*!< Revert to a star network topology */
} adi_wil_topology_t;


/**
 * @brief   Discharge Duty Cycle
 *
 * @details Array of individual cell discharge duty cycle values
 */
typedef uint8_t adi_wil_ddc_t [ADI_WIL_MAX_CELLS];

/******************************************************************************
 * Structure Definitions
 *******************************************************************************/
/**
 * @brief   Device version information structure
 */
typedef struct adi_wil_dev_version_t adi_wil_dev_version_t;

/**
 * @brief   Software information structure
 */
typedef struct adi_wil_version_t adi_wil_version_t;

/**
 * @brief   Sensor data structure
 */
typedef struct adi_wil_sensor_data_t adi_wil_sensor_data_t;

/**
 * @brief   Sensor data buffer structure
 */
typedef struct adi_wil_sensor_data_buffer_t adi_wil_sensor_data_buffer_t;

/**
 * @brief   Script change data structure
 */
typedef struct adi_wil_script_change_t adi_wil_script_change_t;

/**
* @brief    Health Report data structure
*/
typedef struct adi_wil_health_report_t adi_wil_health_report_t;

/**
 * @brief   File information structure
 */
typedef struct adi_wil_file_t adi_wil_file_t;

/**
 * @brief   Network status structure
 */
typedef struct adi_wil_network_status_t adi_wil_network_status_t;

/**
 * @brief   Contextual data structure
 */
typedef struct adi_wil_contextual_data_t adi_wil_contextual_data_t;

/**
 * @brief   LoadFile status structure
 */
typedef struct adi_wil_loadfile_status_t adi_wil_loadfile_status_t;

/**
 * @brief   Get configuration structure
 */
typedef struct adi_wil_configuration_t adi_wil_configuration_t;

/**
 * @brief   Network metadata structure
 */
typedef struct adi_wil_network_data_t adi_wil_network_data_t;

/**
 * @brief   Network metadata buffer structure
 */
typedef struct adi_wil_network_data_buffer_t adi_wil_network_data_buffer_t;

/**
 * @brief   ACL structure
 */
typedef struct adi_wil_acl_t adi_wil_acl_t;

/**
 * @brief   Device removed structure
 */
typedef struct adi_wil_device_removed_t adi_wil_device_removed_t;

/**
 * @brief   Device fault report structure
 */
typedef struct adi_wil_fault_report_t adi_wil_fault_report_t;

/**
 * @brief   Topology assessment structure
 */
typedef struct adi_wil_topology_assessment_t adi_wil_topology_assessment_t;

/**
 * @brief   Port State structure
 */
typedef struct adi_wil_port_t adi_wil_port_t;

/**
 * @brief   Pack structure
 */
typedef struct adi_wil_pack_t adi_wil_pack_t;

/**
 * @brief   Pack internals structure
 */
typedef struct adi_wil_pack_internals_t adi_wil_pack_internals_t;

/**
 * @brief   Safety internals structure
 */
typedef struct adi_wil_safety_internals_t adi_wil_safety_internals_t;

/**
 * @brief   Safety CPU fault message declaration
 */
typedef struct adi_wil_fault_safety_cpu_t adi_wil_fault_safety_cpu_t;

/**
 * @brief   Safety CPU connect message declaration
 */
typedef struct adi_wil_connect_safety_cpu_t adi_wil_connect_safety_cpu_t;

/**
 * @brief   File CRC list definition
 */
typedef struct adi_wil_file_crc_list_t adi_wil_file_crc_list_t;

/**
 * @brief   Connection details structure
 */
typedef struct adi_wil_connection_details_t adi_wil_connection_details_t;

/**
 * @brief   Cell balancing status structure
 */
typedef struct adi_wil_cell_balancing_status_t adi_wil_cell_balancing_status_t;

/******************************************************************************
 * Callback types
 *******************************************************************************/
/**
 * @brief Function type for WIL API state machine function
 */
typedef void (*adi_wil_api_process_func_t)(adi_wil_pack_internals_t * const pInternals);

#ifdef __cplusplus
}
#endif
#endif  // ADI_WIL_TYPES__H

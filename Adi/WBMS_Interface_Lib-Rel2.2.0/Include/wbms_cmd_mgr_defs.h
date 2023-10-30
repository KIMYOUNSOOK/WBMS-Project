/******************************************************************************
 * @file     wbms_cmd_mgr_defs.h
 *
 * @brief    Operational command header.
 *
 * @details  Contains operational command definitions.
 *           This header is used by the WBMS Interface Library,
 *           and Network Manager Application.
 *
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WBMS_CMD_MGR_DEFS__H
#define WBMS_CMD_MGR_DEFS__H

#include "wbms_cmd_defs.h"

/******************************************************************************
 * #defines
 *****************************************************************************/
/**
* @brief  Protocol version in use by application
*/
#define WBMS_SPI_PROTOCOL_VERSION               (5u)

/**
* @brief  Defines the maximum number of nodes supported by this interface
*         definition
*/
#define WBMS_MAX_NODES                          (62u)

/**
* @brief  Maximum count of ACL entries to be transmitted in one request
*/
#define WBMS_MAX_ACL_ENTRIES_PER_REQ            (8u)

/**
* @brief  Length of array represeting all nodes as a single bit
*/
#define WBMS_NODE_BITMAP_SIZE                  (8u)

/**
* @brief  Indicates a manager is assigned as device 0 in the user application
*/
#define WBMS_MANAGER_0_DEVICE_ID               (240u)

/**
* @brief  Indicates a manager is assigned as device 1 in the user application
*/
#define WBMS_MANAGER_1_DEVICE_ID               (241u)

/**
* @brief  Length of the nonce used for SPI encryption in bytes
*/
#define WBMS_SPI_NONCE_SIZE                    (13u)

/**
* @brief  Indicates a manager is configured for use as single or dual manager
*/
#define WBMS_MANAGER_DUAL                       (0u)
#define WBMS_MANAGER_SINGLE                     (1u)

/**
 * @brief  Additional network manager Commands
 */
#define WBMS_CMD_CONNECT                        (120u)
#define WBMS_CMD_SEND_DATA                      (121u)
#define WBMS_CMD_SET_ACL                        (122u)
#define WBMS_CMD_GET_ACL                        (123u)
#define WBMS_CMD_CLEAR_ACL                      (124u)
#define WBMS_CMD_SET_MODE                       (125u)
#define WBMS_CMD_ROTATE_KEY                     (126u)
#define WBMS_CMD_QUERY_DEVICE                   (127u)
#define WBMS_CMD_SET_FAULT_SERVICE_MODE         (128u)
#define WBMS_CMD_DMH_ASSESS                     (129u)
#define WBMS_CMD_DMH_APPLY                      (130u)

#define WBMS_NOTIF_PMS_DATA                     (200u)
#define WBMS_NOTIF_EMS_DATA                     (201u)
#define WBMS_NOTIF_SECURITY_ERROR               (202u)
#define WBMS_NOTIF_PACKET_RECIEVED              (203u)
#define WBMS_NOTIF_NODE_STATE                   (204u)
#define WBMS_NOTIF_HEALTH_REPORT                (205u)
#define WBMS_NOTIF_MON_ALERT_SYSTEM             (206u)
#define WBMS_NOTIF_SYSTEM_STATUS                (207u)
#define WBMS_NOTIF_M2M_COMM_LOSS                (208u)
#define WBMS_NOTIF_DMH_ASSESS                   (209u)
#define WBMS_NOTIF_DMH_APPLY                    (210u)
#define WBMS_NOTIF_NODE_MODE_MISMATCH           (211u)

/**
 * @brief   Constant SPI transaction length in bytes - Must be in multiples of 4
 */
#define WBMS_SPI_TRANSACTION_SIZE               (256u)
#define WBMS_SPI_LOGGED_OUT_SESSION_ID          (0xFFu)

/**
 * @brief  SPI frame protocol offsets and sizes
 */
#define WBMS_FRAME_HDR_LEN                      (4u)
#define WBMS_FRAME_CRC_LEN                      (4u)
#define WBMS_FRAME_SESSION_ID_OFFSET            (1u)
#define WBMS_FRAME_CRC_OFFSET                   (WBMS_SPI_TRANSACTION_SIZE - WBMS_FRAME_CRC_LEN)
#define WBMS_PACKET_HDR_SIZE                    (2u)
#define WBMS_FRAME_PAYLOAD_MAX_SIZE             (WBMS_SPI_TRANSACTION_SIZE - WBMS_FRAME_HDR_LEN - WBMS_FRAME_CRC_LEN)
#define WBMS_MESSAGE_PAYLOAD_MAX_SIZE           (WBMS_FRAME_PAYLOAD_MAX_SIZE - WBMS_PACKET_HDR_SIZE)
#define WBMS_PACKET_CMD_ID_LEN                  (1u)

/**
 * @brief   Manager app request structure sizes
 */
#define WBMS_CMD_REQ_CONNECT_LEN                (3u)
#define WBMS_CMD_REQ_SET_MODE_LEN               (3u)
#define WBMS_CMD_REQ_SEND_DATA_LEN              (6u)
#define WBMS_CMD_REQ_ROTATE_KEY_LEN             (3u)
#define WBMS_CMD_REQ_GET_ACL_LEN                (3u)
#define WBMS_CMD_REQ_SET_ACL_LEN                (3u)
#define WBMS_CMD_REQ_FAULT_MODE_LEN             (3u)
#define WBMS_CMD_REQ_DMH_APPLY_LEN              (3u)

/**
 * @brief   Manager app response structure sizes
 */
#define WBMS_CMD_RESP_CONNECT_LEN               (25u)
#define WBMS_CMD_RESP_QUERY_DEVICE_LEN          (44u + WBMS_SPI_NONCE_SIZE)
#define WBMS_CMD_RESP_GET_ACL_LEN               (4u)
#define WBMS_CMD_RESP_SET_MODE_LEN              (4u + WBMS_NODE_BITMAP_SIZE)

#define WBMS_CMD_NOTIF_SENSOR_DATA_HDR_LEN      (3u)
#define WBMS_CMD_NOTIF_PACKET_RECEIVED_LEN      (21u)
#define WBMS_CMD_NOTIF_NODE_STATE_LEN           (4u)
#define WBMS_CMD_NOTIF_HR_HDR_LEN               (14u)
#define WBMS_CMD_NOTIF_SECURITY_ERROR_LEN       (11u)
#define WBMS_NOTIF_MON_ALERT_SYSTEM_LEN         (1u + WBMS_NODE_BITMAP_SIZE)
#define WBMS_NOTIF_SYSTEM_STATUS_LEN            (77u)
#define WBMS_CMD_NOTIF_ACK_LEN                  (2u)
#define WBMS_NOTIF_M2M_COMM_LOSS_LEN            (2u)
#define WBMS_NOTIF_DMH_ASSESS_LEN               (4u + WBMS_MAX_NODES)
#define WBMS_NOTIF_DMH_APPLY_LEN                (3u)
#define WBMS_NOTIF_NODE_MODE_MISMATCH_LEN       (3u)

/**
 * @brief  Security notification types
 */
#define WBMS_CYS_EVT_NODE_NOT_IN_ACL            (1u)
#define WBMS_CYS_EVT_CERTIFICATE_CALC_ERROR     (2u)
#define WBMS_CYS_EVT_JOIN_NO_KEY                (3u)
#define WBMS_CYS_EVT_JOIN_DUPLICATE_COUNTER     (4u)
#define WBMS_CYS_EVT_JOIN_MIC_FAILURE           (5u)
#define WBMS_CYS_EVT_SEC_ERR_MIC                (6u)
#define WBMS_CYS_EVT_SEC_ERR                    (7u)
#define WBMS_CYS_EVT_SECERR_M2M_JOIN_CNTR       (8u)
#define WBMS_CYS_EVT_SECERR_M2M_SESS_CNTR       (9u)
#define WBMS_CYS_EVT_SECERR_EXCHCERT_LOCK       (10u)

/**
* @brief   Manager specific monitor-mode fault types
*/
#define WBMS_FAULT_M2M_COMMS                    (0x0100u)
#define WBMS_FAULT_SERVICE_DISABLE              (0x0200u)
#define WBMS_FAULT_PERIODIC_ASSERTION           (0x0400u)
#define WBMS_FAULT_NETWORK_COMMS                (0x0800u)

/**
 * @brief  DMH topology types
 */
#define WBMS_DMH_APPLY_SAVED                    (1u)
#define WBMS_DMH_APPLY_NEW                      (2u)
#define WBMS_DMH_APPLY_STAR                     (3u)

/******************************************************************************
 * Request structures
 *****************************************************************************/

/**
 * @brief   wbms_cmd_req_connect_t
 */
typedef struct wbms_cmd_req_connect_t wbms_cmd_req_connect_t;

/**
 * @brief   wbms_cmd_req_set_mode_t
 */
typedef struct wbms_cmd_req_set_mode_t wbms_cmd_req_set_mode_t;

/**
 * @brief   wbms_cmd_req_send_data_t
 */
typedef struct wbms_cmd_req_send_data_t wbms_cmd_req_send_data_t;

/**
 * @brief   wbms_cmd_req_rotate_key_t
 */
typedef struct wbms_cmd_req_rotate_key_t wbms_cmd_req_rotate_key_t;

/**
 * @brief   wbms_cmd_req_set_acl_t
 */
typedef struct wbms_cmd_req_set_acl_t wbms_cmd_req_set_acl_t;

/**
 * @brief   wbms_cmd_req_set_acl_t
 */
typedef struct wbms_cmd_req_get_acl_t wbms_cmd_req_get_acl_t;

/**
 * @brief   wbms_cmd_req_fault_service_t
 */
typedef struct wbms_cmd_req_fault_service_t wbms_cmd_req_fault_service_t;

/**
 * @brief   wbms_cmd_req_dmh_apply_t
 */
typedef struct wbms_cmd_req_dmh_apply_t wbms_cmd_req_dmh_apply_t;

/******************************************************************************
 * Response structures
 *****************************************************************************/

/**
 * @brief   wbms_cmd_resp_connect_t
 */
typedef struct wbms_cmd_resp_connect_t wbms_cmd_resp_connect_t;

/**
 * @brief   wbms_cmd_resp_query_device_t
 */
typedef struct wbms_cmd_resp_query_device_t wbms_cmd_resp_query_device_t;

/**
 * @brief   wbms_cmd_resp_get_acl_t
 */
typedef struct wbms_cmd_resp_get_acl_t wbms_cmd_resp_get_acl_t;

/**
 * @brief   wbms_cmd_resp_set_mode_t
 */
typedef struct wbms_cmd_resp_set_mode_t wbms_cmd_resp_set_mode_t;

/******************************************************************************
 * Notification Structures
 *****************************************************************************/

/**
 * @brief   wbms_notif_packet_received_t
 */
typedef struct wbms_notif_packet_received_t wbms_notif_packet_received_t;

/**
 * @brief   wbms_notif_sensor_data_t
 */
typedef struct wbms_notif_sensor_data_t wbms_notif_sensor_data_t;

/**
 * @brief   wbms_notif_node_state_t
 */
typedef struct wbms_notif_node_state_t wbms_notif_node_state_t;

/**
 * @brief   wbms_notif_health_report_t
 */
typedef struct wbms_notif_health_report_t wbms_notif_health_report_t;

/**
 * @brief   wbms_notif_security_error_t
 */
typedef struct wbms_notif_security_error_t wbms_notif_security_error_t;

/**
 * @brief   wbms_notif_mon_alert_system_t
 */
typedef struct wbms_notif_mon_alert_system_t wbms_notif_mon_alert_system_t;

/**
 * @brief   wbms_notif_system_status_t
 */
typedef struct wbms_notif_system_status_t wbms_notif_system_status_t;

/**
 * @brief   wbms_notif_m2m_comm_loss_t
 */
typedef struct wbms_notif_m2m_comm_loss_t wbms_notif_m2m_comm_loss_t;

/**
 * @brief   wbms_notif_dmh_assess_t
 */
typedef struct wbms_notif_dmh_assess_t wbms_notif_dmh_assess_t;

/**
 * @brief   wbms_notif_dmh_apply_t
 */
typedef struct wbms_notif_dmh_apply_t wbms_notif_dmh_apply_t;

/**
 * @brief   wbms_notif_node_mode_mismatch_t
 */
typedef struct wbms_notif_node_mode_mismatch_t wbms_notif_node_mode_mismatch_t;

/******************************************************************************
 * Notification Acknowledgement Structures
 *****************************************************************************/

/**
 * @brief   wbms_notif_ack_t
 */
typedef struct wbms_notif_ack_t wbms_notif_ack_t;

#endif  // #ifndef WBMS_CMD_MGR_DEFS__H

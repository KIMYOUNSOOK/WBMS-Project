/******************************************************************************
 * @file     wbms_cmd_defs.h
 *
 * @brief    Operational command header.
 *
 * @details  Contains operational command declarations common to manager and
 *           node.
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WBMS_CMD_DEFS__H
#define WBMS_CMD_DEFS__H

#include <stdint.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

#define WBMS_OTAP_BLOCK_SIZE                        (64u)
#define WBMS_GET_FILE_BLOCK_SIZE                    (64u)
#define WBMS_MAX_CONTEXTUAL_DATA_LEN                (75u)
#define WBMS_MAX_MODIFY_SCRIPT_LEN                  (64u)
#define WBMS_OTAP_MISSING_BLOCK_MASK_LEN            (16u)
#define WB_WIL_OTAP_FILE_HEADER_LEN                 (36u)
#define WBMS_MAC_ADDR_LEN                           (8u)
#define WBMS_FAULT_CHANNELS_SIZE                    (24u)

/**
 * @brief   Application commands
 */
#define WBMS_CMD_RESET                              (1u)
#define WBMS_CMD_SET_GPIO                           (2u)
#define WBMS_CMD_GET_GPIO                           (3u)
#define WBMS_CMD_GET_FILE                           (4u)
#define WBMS_CMD_OTAP_HANDSHAKE                     (5u)
#define WBMS_CMD_OTAP_DATA                          (6u)
#define WBMS_CMD_OTAP_STATUS                        (7u)
#define WBMS_CMD_OTAP_COMMIT                        (8u)
#define WBMS_CMD_SELECT_SCRIPT                      (9u)
#define WBMS_CMD_MODIFY_SCRIPT                      (10u)
#define WBMS_CMD_SET_CONTEXTUAL_DATA                (11u)
#define WBMS_CMD_GET_CONTEXTUAL_DATA                (12u)
#define WBMS_CMD_GET_VERSION                        (13u)
#define WBMS_CMD_ERASE_FILE                         (14u)
#define WBMS_CMD_GET_FILE_CRC                       (15u)
#define WBMS_CMD_SET_CUSTOMER_IDENTIFIER            (16u)
#define WBMS_CMD_SET_MON_PARAMS_DATA                (17u)
#define WBMS_CMD_SET_MON_PARAMS_COMMIT              (18u)
#define WBMS_CMD_GET_MON_PARAMS_CRC                 (19u)
#define WBMS_NOTIF_MON_ALERT_DEVICE                 (20u)

/**
 * @brief   System modes
 */
#define WBMS_MODE_STANDBY                           (1u)
#define WBMS_MODE_ACTIVE                            (2u)
#define WBMS_MODE_MONITORING                        (3u)
#define WBMS_MODE_OTAP                              (4u)
#define WBMS_MODE_COMMISSIONING                     (5u)
#define WBMS_MODE_SLEEP                             (6u)

/**
 * @brief   Return codes
 */
#define WBMS_CMD_RC_SUCCESS                         (1u)
#define WBMS_CMD_RC_FAILED                          (2u)
#define WBMS_CMD_RC_INVALID_ARGUMENT                (3u)
#define WBMS_CMD_RC_WAIT                            (4u)
#define WBMS_CMD_RC_NOT_SUPPORTED                   (5u)
#define WBMS_CMD_RC_CRC_ERROR                       (6u)
#define WBMS_CMD_RC_FILE_REJECTED                   (7u)
#define WBMS_CMD_RC_ERR_STATE                       (8u)

/**
 * @brief   Sensor Ids
 */
#define WBMS_SENSOR_ID_PMS                          (1u)
#define WBMS_SENSOR_ID_BMS                          (2u)
#define WBMS_SENSOR_ID_EMS                          (3u)

/**
 * @brief   Contextual data Ids
 */
#define WBMS_CONTEXTUAL_ID_0                        (1u)
#define WBMS_CONTEXTUAL_ID_1                        (2u)
#define WBMS_CONTEXTUAL_ID_WRITE_ONCE               (3u)

/**
 * @brief   Customer Identifier Ids
 */
#define WBMS_CUSTOMER_IDENTIFIER_ID_0               (1u)
#define WBMS_CUSTOMER_IDENTIFIER_ID_1               (2u)
#define WBMS_CUSTOMER_IDENTIFIER_ID_2               (3u)

/**
 * @brief   File types
 */
#define WBMS_FILE_TYPE_FIRMWARE                     (1u)
#define WBMS_FILE_TYPE_BMS_CONTAINER                (2u)
#define WBMS_FILE_TYPE_CONFIGURATION                (3u)
#define WBMS_FILE_TYPE_BLACK_BOX_LOG                (4u)
#define WBMS_FILE_TYPE_INVENTORY_LOG                (5u)
#define WBMS_FILE_TYPE_ENV_MON                      (6u)
#define WBMS_FILE_TYPE_PMS_CONTAINER                (7u)
#define WBMS_FILE_TYPE_EMS_CONTAINER                (8u)


/**
 * @brief   Common monitor-mode fault types
 */
#define WBMS_FAULT_PLAUSABILITY                     (0x0001u)
#define WBMS_FAULT_SENSOR_COMMS                     (0x0002u)
#define WBMS_FAULT_CONTROL_FLOW                     (0x0004u)

/**
 * @brief   Packed type lengths
 */

#define WBMS_CMD_REQ_GENERIC_LEN                    (2u)
#define WBMS_CMD_REQ_SET_GPIO_LEN                   (4u)
#define WBMS_CMD_REQ_GET_GPIO_LEN                   (3u)
#define WBMS_CMD_REQ_OTAP_HANDSHAKE_LEN             (3u + WB_WIL_OTAP_FILE_HEADER_LEN)
#define WBMS_CMD_REQ_OTAP_DATA_LEN                  (4u)
#define WBMS_CMD_REQ_SELECT_SCRIPT_LEN              (4u)
#define WBMS_CMD_REQ_MODIFY_SCRIPT_LEN              (10u)
#define WBMS_CMD_REQ_MODIFY_SCRIPT_METADATA_LEN     (WBMS_CMD_REQ_MODIFY_SCRIPT_LEN - 4u) /* Metadata is all fields in header except token and CRC itself */
#define WBMS_CMD_REQ_SET_CONTEXTUAL_LEN             (4u)
#define WBMS_CMD_REQ_GET_CONTEXTUAL_LEN             (3u)
#define WBMS_CMD_REQ_SET_CUSTOMER_IDENTIFIER_LEN    (4u)
#define WBMS_CMD_REQ_ERASE_FILE_LEN                 (3u)
#define WBMS_CMD_REQ_GET_FILE_CRC_LEN               (3u)
#define WBMS_CMD_REQ_GET_FILE_LEN                   (5u)
#define WBMS_CMD_REQ_SET_MON_PARAMS_DATA_LEN        (7u)
#define WBMS_CMD_REQ_SET_MON_PARAMS_PAYLOAD_MAX_LEN (79u - WBMS_CMD_REQ_SET_MON_PARAMS_DATA_LEN)

#define WBMS_CMD_RESP_GENERIC_LEN                   (3u)
#define WBMS_CMD_RESP_GET_GPIO_LEN                  (4u)
#define WBMS_CMD_RESP_OTAP_HANDSHAKE_LEN            (7u)
#define WBMS_CMD_RESP_OTAP_STATUS_LEN               (5u + WBMS_OTAP_MISSING_BLOCK_MASK_LEN)
#define WBMS_CMD_RESP_GET_VERSION_LEN               (27u)
#define WBMS_CMD_RESP_GET_CONTEXTUAL_LEN            (4u)
#define WBMS_CMD_RESP_GET_FILE_CRC_LEN              (7u)
#define WBMS_CMD_RESP_GET_FILE_LEN                  (6u)
#define WBMS_CMD_RESP_GET_MON_PARAMS_CRC_LEN        (7u)

#define WBMS_NOTIF_MON_ALERT_DEVICE_LEN             (2u + WBMS_FAULT_CHANNELS_SIZE)

/******************************************************************************
 * Request structures
 *****************************************************************************/
 /**
  * @brief   wbms_cmd_req_generic_t
  */
typedef struct wbms_cmd_req_generic_t wbms_cmd_req_generic_t;

/**
 * @brief   wbms_cmd_req_set_gpio_t
 */
typedef struct wbms_cmd_req_set_gpio_t wbms_cmd_req_set_gpio_t;

/**
 * @brief   wbms_cmd_req_get_gpio_t
 */
typedef struct wbms_cmd_req_get_gpio_t wbms_cmd_req_get_gpio_t;

/**
 * @brief   wbms_cmd_req_otap_handshake_t
 */
typedef struct wbms_cmd_req_otap_handshake_t wbms_cmd_req_otap_handshake_t;

/**
 * @brief   wbms_cmd_req_otap_data_t
 */
typedef struct wbms_cmd_req_otap_data_t wbms_cmd_req_otap_data_t;

/**
 * @brief wbms_cmd_req_select_script_t
 */
typedef struct wbms_cmd_req_select_script_t wbms_cmd_req_select_script_t;

/**
 * @brief wbms_cmd_req_modify_script_t
 */
typedef struct wbms_cmd_req_modify_script_t wbms_cmd_req_modify_script_t;

/**
 * @brief   wbms_cmd_req_set_contextual_t
 */
typedef struct wbms_cmd_req_set_contextual_t wbms_cmd_req_set_contextual_t;

/**
 * @brief   wbms_cmd_req_get_contextual_t
 */
typedef struct wbms_cmd_req_get_contextual_t wbms_cmd_req_get_contextual_t;

/**
 * @brief   wbms_cmd_req_set_customer_identifier_t
 */
typedef struct wbms_cmd_req_set_customer_identifier_t wbms_cmd_req_set_customer_identifier_t;

/**
 * @brief wbms_cmd_req_get_file_t
 */
typedef struct wbms_cmd_req_get_file_t wbms_cmd_req_get_file_t;

/**
 * @brief wbms_cmd_req_erase_file_t
 */
typedef struct wbms_cmd_req_erase_file_t wbms_cmd_req_erase_file_t;

/**
 * @brief wbms_cmd_req_get_file_crc_t
 */
typedef struct wbms_cmd_req_get_file_crc_t wbms_cmd_req_get_file_crc_t;

/**
 * @brief wbms_cmd_req_set_mon_params_data_t
 */
typedef struct wbms_cmd_req_set_mon_params_data_t wbms_cmd_req_set_mon_params_data_t;

/******************************************************************************
 * Response structures
 *****************************************************************************/
/**
 * @brief   wbms_cmd_resp_generic_t
 */
typedef struct wbms_cmd_resp_generic_t wbms_cmd_resp_generic_t;

/**
 * @brief   wbms_cmd_resp_get_gpio_t
 */
typedef struct wbms_cmd_resp_get_gpio_t wbms_cmd_resp_get_gpio_t;

/**
 * @brief   wbms_cmd_resp_otap_handshake_t
 */
typedef struct wbms_cmd_resp_otap_handshake_t wbms_cmd_resp_otap_handshake_t;

/**
 * @brief   wbms_cmd_resp_otap_status_t
 */
typedef struct wbms_cmd_resp_otap_status_t wbms_cmd_resp_otap_status_t;

/**
 * @brief   wbms_cmd_resp_get_version_t
 */
typedef struct wbms_cmd_resp_get_version_t wbms_cmd_resp_get_version_t;

/**
 * @brief wbms_cmd_resp_get_contextual_t
 */
typedef struct wbms_cmd_resp_get_contextual_t wbms_cmd_resp_get_contextual_t;

/**
 * @brief wbms_cmd_resp_get_file_t
 */
typedef struct wbms_cmd_resp_get_file_t wbms_cmd_resp_get_file_t;

/**
 * @brief wbms_cmd_resp_get_file_crc_t
 */
typedef struct wbms_cmd_resp_get_file_crc_t wbms_cmd_resp_get_file_crc_t;

/**
 * @brief wbms_cmd_resp_get_mon_params_crc_t
 */
typedef struct wbms_cmd_resp_get_mon_params_crc_t wbms_cmd_resp_get_mon_params_crc_t;

/******************************************************************************
 * Notification structures
 *****************************************************************************/
/**
 * @brief   wbms_notif_mon_alert_device_t
 */
typedef struct wbms_notif_mon_alert_device_t wbms_notif_mon_alert_device_t;

#endif  // #ifndef WBMS_CMD_DEFS__H

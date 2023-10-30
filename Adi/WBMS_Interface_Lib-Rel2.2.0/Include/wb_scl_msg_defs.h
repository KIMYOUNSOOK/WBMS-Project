/******************************************************************************
* @file    wb_scl_msg_defs.h
*
* @brief   WIL FE Safety Communications Layer Message macro definitions
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_SCL_MSG_DEFS__H
#define WB_SCL_MSG_DEFS__H

/**
 * @brief Length of SCL message header
 */
#define WB_SCL_MSG_HDR_LEN   (5u)

/**
 * @brief Length of SCL message CRC
 */
#define WB_SCL_MSG_CRC_LEN   (4u)

/******************************************************************************
 * Message type definitions
 *****************************************************************************/

#define WB_SCL_MSG_TYPE_CONNECT                        (1u)
#define WB_SCL_MSG_TYPE_MEASUREMENT_DATA_START         (2u)
#define WB_SCL_MSG_TYPE_MEASUREMENT_DATA_SUPPLEMENTAL  (3u)
#define WB_SCL_MSG_TYPE_SENSOR_COMMAND                 (4u)
#define WB_SCL_MSG_TYPE_HEARTBEAT                      (5u)
#define WB_SCL_MSG_TYPE_FAULT                          (6u)

/******************************************************************************
 * Sensor command type definitions
 *****************************************************************************/

#define WB_SCL_SENSOR_CMD_CONFIG_CELL_BALANCING          (1u)
#define WB_SCL_SENSOR_CMD_GET_CELL_BALANCING_STATUS      (2u)

/******************************************************************************
 * Sensor type definitions
 *****************************************************************************/

#define WB_SCL_SENSOR_TYPE_BMS                           (1u)
#define WB_SCL_SENSOR_TYPE_PMS                           (2u)
#define WB_SCL_SENSOR_TYPE_EMS                           (3u)

/******************************************************************************
 * Message length definitions
 *****************************************************************************/

#define WB_SCL_FRAME_LEN_MAX                           (ADI_WIL_MAX_APP_PAYLOAD_SIZE)
#define WB_SCL_FRAME_LEN_MIN                           (WB_SCL_MSG_HDR_LEN + WB_SCL_MSG_CRC_LEN)
#define WB_SCL_PAYLOAD_LEN_MAX                         (WB_SCL_FRAME_LEN_MAX - WB_SCL_FRAME_LEN_MIN)
#define WB_SCL_CONNECT_RESPONSE_LEN                    (51u)
#define WB_SCL_SENSOR_CMD_RSP_LEN_MIN                  (1u)
#define WB_SCL_CELL_BALANCING_REQUEST_LEN              (71u)
#define WB_SCL_CELL_BALANCING_RESPONSE_LEN             (3u)
#define WB_SCL_GET_CELL_BALANCING_STATUS_REQUEST_LEN   (1u)
#define WB_SCL_GET_CELL_BALANCING_STATUS_RESPONSE_LEN  (71u)
#define WB_SCL_HEARTBEAT_REQUEST_LEN                   (12u)
#define WB_SCL_XMS_MESSAGE_LEN_MIN                     (1u)

/******************************************************************************
 * Message byte offset definitions
 *****************************************************************************/

#define WB_SCL_CONNECT_MSG_NODE_MAC_OFFSET                     (0u)
#define WB_SCL_CONNECT_MSG_SAFETY_SERIAL_ID_OFFSET             (8u)
#define WB_SCL_CONNECT_MSG_SOFTWARE_MAJOR_OFFSET               (16u)
#define WB_SCL_CONNECT_MSG_SOFTWARE_MINOR_OFFSET               (18u)
#define WB_SCL_CONNECT_MSG_SOFTWARE_PATCH_OFFSET               (20u)
#define WB_SCL_CONNECT_MSG_SOFTWARE_BUILD_OFFSET               (22u)
#define WB_SCL_CONNECT_MSG_CONFIG_CRC_OFFSET                   (24u)
#define WB_SCL_CONNECT_MSG_PMS_CONTAINER_CRC_OFFSET            (28u)
#define WB_SCL_CONNECT_MSG_BMS_CONTAINER_CRC_OFFSET            (32u)
#define WB_SCL_CONNECT_MSG_EMS_CONTAINER_CRC_OFFSET            (36u)
#define WB_SCL_CONNECT_MSG_PMS_INTERVAL_OFFSET                 (40u)
#define WB_SCL_CONNECT_MSG_BMS_INTERVAL_OFFSET                 (42u)
#define WB_SCL_CONNECT_MSG_EMS_INTERVAL_OFFSET                 (44u)
#define WB_SCL_CONNECT_MSG_MAX_PMS_PACKETS_OFFSET              (46u)
#define WB_SCL_CONNECT_MSG_MAX_BMS_PACKETS_OFFSET              (47u)
#define WB_SCL_CONNECT_MSG_MAX_EMS_PACKETS_OFFSET              (48u)
#define WB_SCL_CONNECT_MSG_OPERATING_MODE_OFFSET               (49u)
#define WB_SCL_CONNECT_MSG_RC_OFFSET                           (50u)
#define WB_SCL_HEARTBEAT_MSG_CONNECTED_MASK_OFFSET             (0u)
#define WB_SCL_HEARTBEAT_MSG_RESERVED_OFFSET                   (8u)
#define WB_SCL_HEARTBEAT_MSG_RESERVED_LEN                      (4u)
#define WB_SCL_SENSOR_COMMAND_ID_OFFSET                        (0u)
#define WB_SCL_CELL_BALANCING_REQ_DDC_OFFSET                   (1u)
#define WB_SCL_CELL_BALANCING_REQ_DURATION_OFFSET              (65u)
#define WB_SCL_CELL_BALANCING_REQ_UV_OFFSET                    (67u)
#define WB_SCL_SENSOR_CMD_RSP_CMDID_OFFSET                     (0u)
#define WB_SCL_CELL_BALANCING_RSP_SEQNUM_OFFSET                (1u)
#define WB_SCL_CELL_BALANCING_RSP_RC_OFFSET                    (2u)
#define WB_SCL_CELL_BALANCING_STATUS_RSP_SEQNUM_OFFSET         (1u)
#define WB_SCL_CELL_BALANCING_STATUS_RSP_RC_OFFSET             (2u)
#define WB_SCL_CELL_BALANCING_STATUS_RSP_ENABLED_CELLS_OFFSET  (4u)
#define WB_SCL_CELL_BALANCING_STATUS_RSP_DURATION_OFFSET       (12u)
#define WB_SCL_CELL_BALANCING_STATUS_RSP_THERMAL_OFFSET        (14u)
#define WB_SCL_XMS_MSG_SENSOR_TYPE_OFFSET                      (0u)
#define WB_SCL_XMS_MSG_SENSOR_TYPE_LEN                         (1u)
#define WB_SCL_XMS_MSG_MEASUREMENT_DATA_OFFSET                 (1u)

/******************************************************************************
 * Device IDs
 *****************************************************************************/

#define WB_SCL_ALL_NODES_DEVICE_ID                   (255u)
#define WB_SCL_MANAGER_0_DEVICE_ID                   (240u)
#define WB_SCL_MANAGER_1_DEVICE_ID                   (241u)

#endif  //WB_SCL_MSG_DEFS__H

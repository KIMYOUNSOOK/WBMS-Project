 /*******************************************************************************
 * @file     wbms_cmd_node_defs.h
 *
 * @brief    Operational command header.
 *
 * @details  Contains operational command definitions.
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_CMD_DEFS_NODE_H
#define WB_CMD_DEFS_NODE_H

#include "wbms_cmd_defs.h"
#include <stdint.h>

/*******************************************************************************
 * #defines
 *******************************************************************************/

/**
 * @brief   Node Commands
 */
#define WBMS_CMD_SET_STATE_OF_HEALTH            (100u)
#define WBMS_CMD_GET_STATE_OF_HEALTH            (101u)
#define WBMS_CMD_INVENTORY_STATE_TRANSITION     (102u)

/**
 * @brief   Node app request structure sizes
 */
#define WBMS_CMD_REQ_SET_SOH_LEN                (3u)
#define WBMS_CMD_REQ_INVENTORY_TRANS_LEN        (11u)

/**
 * @brief   Node app response structure sizes
 */
#define WBMS_CMD_RSP_GET_SOH_LEN                (4u)

/*******************************************************************************
 * Request structures
 *******************************************************************************/

/**
 * @brief   wbms_cmd_req_get_state_of_health_t
 */
typedef struct wbms_cmd_req_set_state_of_health_t wbms_cmd_req_set_state_of_health_t;

/**
 * @brief   wbms_cmd_req_inventory_transition_t
 */
typedef struct wbms_cmd_req_inventory_transition_t wbms_cmd_req_inventory_transition_t;

/*******************************************************************************
 * Response structures
 *******************************************************************************/

/**
 * @brief   wbms_cmd_req_get_state_of_health_t
 */
typedef struct wbms_cmd_resp_get_state_of_health_t wbms_cmd_resp_get_state_of_health_t;

#endif  // #ifndef WB_CMD_NODE_DEFS_H

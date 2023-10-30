/******************************************************************************
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_PACK_WBMS_CMD_NODE_H
#define WB_PACK_WBMS_CMD_NODE_H

#include "wb_pack_cmd.h"
#include "wbms_cmd_node_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Request structures
 *****************************************************************************/

void wb_pack_SetStateOfHealthReq(wb_pack_element_t * packet, wbms_cmd_req_set_state_of_health_t * obj);

void wb_pack_InventoryTransitionReq(wb_pack_element_t * packet, wbms_cmd_req_inventory_transition_t * obj);

/******************************************************************************
 * Response structures
 *****************************************************************************/

void wb_pack_GetStateOfHealthResp(wb_pack_element_t * packet, wbms_cmd_resp_get_state_of_health_t * obj);

#ifdef __cplusplus
}
#endif
#endif //WB_PACK_WBMS_CMD_NODE_H

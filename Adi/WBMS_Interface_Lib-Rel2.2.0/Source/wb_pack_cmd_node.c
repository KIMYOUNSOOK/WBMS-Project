/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include "wb_pack_cmd_node.h"
#include "wb_req_set_state_of_health.h"
#include "wb_rsp_get_state_of_health.h"
#include "wb_req_inventory_transition.h"
#include "wb_packer.h"

/******************************************************************************
 * Request structures
 ******************************************************************************/

void wb_pack_SetStateOfHealthReq(wb_pack_element_t * packet, wbms_cmd_req_set_state_of_health_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iPercentage);
}

void wb_pack_InventoryTransitionReq(wb_pack_element_t * packet, wbms_cmd_req_inventory_transition_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->bExitFlag);
    wb_packer_uint64(&packet->packer, &obj->iTimeInSeconds);
}

/******************************************************************************
 * Response structures
 *****************************************************************************/

void wb_pack_GetStateOfHealthResp(wb_pack_element_t * packet, wbms_cmd_resp_get_state_of_health_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iPercentage);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

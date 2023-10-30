/******************************************************************************
 * @file    wb_wil_node_state.h
 *
 * @brief   Handled the Node State change in WBMS Network.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_NODE_STATE_H
#define WB_WIL_NODE_STATE_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

/**
 * @brief Handle Node State Change
 *
 * @param pInternals - pointer to pack internals struct
 * @param pPort - NIL Manager Id
 * @param pNotif - pointer to State Notification
 * @return none
 *
 */
void wb_wil_HandleNodeState(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_notif_node_state_t const * const pNotif);

/**
 * @brief Handle Incorrect Node Mode Message
 *
 * @param pInternals - pointer to pack internals struct
 * @param pPort - NIL Manager Id
 * @param pNotif - pointer to node mode message Notification
 * @return none
 *
 */
void wb_wil_HandleNodeModeMismatchNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wbms_notif_node_mode_mismatch_t const * const pNotif);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_NODE_STATE_H

/******************************************************************************
 * @file    wb_wil_dmh.h
 *
 * @brief   Contains declarations for DMH related APIs and handlers
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_DMH_H
#define WB_WIL_DMH_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_AssessNetworkTopologyAPI (adi_wil_pack_internals_t * const pInternals);

adi_wil_err_t wb_wil_ApplyNetworkTopologyAPI (adi_wil_pack_internals_t * const pInternals,
                                              adi_wil_topology_t eTopology);

void wb_wil_HandleAssessNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals,
                                                 wbms_cmd_resp_generic_t const * const pResponse);

void wb_wil_HandleApplyNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals,
                                                wbms_cmd_resp_generic_t const * const pResponse);

void wb_wil_HandleAssessTopologyNotif (adi_wil_pack_internals_t const * const pInternals,
                                       adi_wil_port_t * const pPort,
                                       wbms_notif_dmh_assess_t const * const pNotif);

void wb_wil_HandleApplyTopologyNotif (adi_wil_pack_internals_t const * const pInternals,
                                      adi_wil_port_t * const pPort,
                                      wbms_notif_dmh_apply_t const * const pNotif);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_DMH_H

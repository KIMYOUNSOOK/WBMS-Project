/******************************************************************************
 * @file    wb_wil_request.h
 *
 * @brief   Pack node and manager requests and submit to manager API port
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_REQUEST_H
#define WB_WIL_REQUEST_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"
#include "wbms_cmd_node_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetupRequest (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   uint8_t iTargetCount,
                                   adi_wil_target_t const ValidTargets [],
                                   adi_wil_api_process_func_t pfRequestFunc);

adi_wil_err_t wb_wil_ClearPendingResponse (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId);

adi_wil_err_t wb_wil_GenericRequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_generic_t * const pRequest,
                                     uint8_t iMessageId,
                                     uint32_t iTimeout);

adi_wil_err_t wb_wil_GenericAcknowledgement (adi_wil_port_t * const pPort,
                                             wbms_notif_ack_t * const pRequest,
                                             uint8_t iMessageId);

adi_wil_err_t wb_wil_ConnectRequest (adi_wil_port_t * const pPort,
                                     wbms_cmd_req_connect_t * const pRequest);

adi_wil_err_t wb_wil_SetModeRequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_set_mode_t * const pRequest,
                                     uint32_t iTimeout);

adi_wil_err_t wb_wil_SetStateOfHealthRequest (adi_wil_pack_internals_t * const pInternals,
                                              wbms_cmd_req_set_state_of_health_t * const pRequest,
                                              uint32_t iTimeout);

adi_wil_err_t wb_wil_OTAPHandshakeRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_otap_handshake_t * const pRequest,
                                           uint32_t iTimeout);

adi_wil_err_t wb_wil_OTAPDataRequest (adi_wil_pack_internals_t * const pInternals,
                                      wbms_cmd_req_otap_data_t * const pRequest,
                                      uint8_t const * const pData,
                                      uint8_t iLength,
                                      uint32_t iTimeout);

adi_wil_err_t wb_wil_SetGPIORequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_set_gpio_t * const pRequest,
                                     uint32_t iTimeout);

adi_wil_err_t wb_wil_GetGPIORequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_get_gpio_t * const pRequest,
                                     uint32_t iTimeout);

adi_wil_err_t wb_wil_SelectScriptRequest (adi_wil_pack_internals_t * const pInternals,
                                          wbms_cmd_req_select_script_t * const pRequest,
                                          uint32_t iTimeout);

adi_wil_err_t wb_wil_ModifyScriptRequest (adi_wil_pack_internals_t * const pInternals,
                                          wbms_cmd_req_modify_script_t * const pRequest,
                                          uint8_t const * const pData,
                                          uint8_t iLength,
                                          uint32_t iTimeout);

adi_wil_err_t wb_wil_SetContextualRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_set_contextual_t * const pRequest,
                                           uint8_t const * const pData,
                                           uint8_t iLength,
                                           uint32_t iTimeout);

adi_wil_err_t wb_wil_SetCustomerIdentifierRequest (adi_wil_pack_internals_t * const pInternals,
                                                   wbms_cmd_req_set_customer_identifier_t * const pRequest,
                                                   uint8_t const * const pData,
                                                   uint8_t iLength,
                                                   uint32_t iTimeout);

adi_wil_err_t wb_wil_GetContextualRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_get_contextual_t * const pRequest,
                                           uint32_t iTimeout);

adi_wil_err_t wb_wil_GetFileRequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_get_file_t * const pRequest,
                                     uint32_t iTimeout);

adi_wil_err_t wb_wil_EraseFileRequest (adi_wil_pack_internals_t * const pInternals,
                                       wbms_cmd_req_erase_file_t * const pRequest,
                                       uint32_t iTimeout);

adi_wil_err_t wb_wil_InventoryTransitionRequest (adi_wil_pack_internals_t * const pInternals,
                                                 wbms_cmd_req_inventory_transition_t * const pRequest,
                                                 uint32_t iTimeout);

adi_wil_err_t wb_wil_GetFileCRCRequest (adi_wil_pack_internals_t * const pInternals,
                                        wbms_cmd_req_get_file_crc_t * const pRequest,
                                        uint32_t iTimeout);

adi_wil_err_t wb_wil_RotateKeyRequest (adi_wil_pack_internals_t * const pInternals,
                                       wbms_cmd_req_rotate_key_t * const pRequest,
                                       uint32_t iTimeout);

adi_wil_err_t wb_wil_QueryDeviceRequest (adi_wil_port_t * pPort,
                                         wbms_cmd_req_generic_t * const pRequest);

adi_wil_err_t wb_wil_SetACLRequest (adi_wil_pack_internals_t * const pInternals,
                                    wbms_cmd_req_set_acl_t * const pRequest,
                                    uint8_t const * const pData,
                                    uint8_t iLength,
                                    uint32_t iTimeout);

adi_wil_err_t wb_wil_GetACLRequest (adi_wil_pack_internals_t * const pInternals,
                                    wbms_cmd_req_get_acl_t * const pRequest,
                                    uint32_t iTimeout);

adi_wil_err_t wb_wil_SetFaultModeRequest (adi_wil_pack_internals_t * const pInternals,
                                            wbms_cmd_req_fault_service_t * const pRequest,
                                            uint32_t iTimeout);

adi_wil_err_t wb_wil_UpdateMonitorParametersDataRequest (adi_wil_pack_internals_t * const pInternals,
                                                         wbms_cmd_req_set_mon_params_data_t * const pRequest,
                                                         uint8_t const * const pData,
                                                         uint8_t iLength,
                                                         uint32_t iTimeout);

adi_wil_err_t wb_wil_ASSLSendDataRequest (adi_wil_pack_internals_t * const pInternals,
                                          uint8_t const * const pMessage,
                                          uint8_t iDeviceId);

adi_wil_err_t wb_wil_ApplyNetworkTopologyRequest (adi_wil_pack_internals_t * const pInternals,
                                                  wbms_cmd_req_dmh_apply_t * const pRequest,
                                                  uint32_t iTimeout);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_REQUEST_H


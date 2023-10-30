/******************************************************************************
 * @file     wbms_pack_cmd.h
 *
 * @brief
 *
 * @details
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_PACK_WBMS_CMD_H
#define WB_PACK_WBMS_CMD_H

#include "wb_packer.h"
#include "wbms_cmd_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Pack functions for request structures
 *****************************************************************************/

void wb_pack_GenericReq(wb_pack_element_t * packet, wbms_cmd_req_generic_t * obj);

void wb_pack_SetGpioReq(wb_pack_element_t * packet, wbms_cmd_req_set_gpio_t * obj);

void wb_pack_GetGpioReq(wb_pack_element_t * packet, wbms_cmd_req_get_gpio_t * obj);

void wb_pack_OTAPHandshakeReq(wb_pack_element_t * packet, wbms_cmd_req_otap_handshake_t * obj);

void wb_pack_OTAPDataReq(wb_pack_element_t * packet, wbms_cmd_req_otap_data_t * obj, uint8_t ** data);

void wb_pack_SelectScriptReq(wb_pack_element_t * packet, wbms_cmd_req_select_script_t * obj);

void wb_pack_ModifyScriptReq(wb_pack_element_t * packet, wbms_cmd_req_modify_script_t * obj, uint16_t * hdr_crc, uint8_t ** data);

void wb_pack_SetContextualReq(wb_pack_element_t * packet, wbms_cmd_req_set_contextual_t * obj, uint8_t ** data);

void wb_pack_SetCustomerIdentifierReq(wb_pack_element_t * packet, wbms_cmd_req_set_customer_identifier_t * obj, uint8_t ** data);

void wb_pack_GetContextualReq(wb_pack_element_t * packet, wbms_cmd_req_get_contextual_t * obj);

void wb_pack_GetFileReq(wb_pack_element_t * packet, wbms_cmd_req_get_file_t * obj);

void wb_pack_EraseFileReq(wb_pack_element_t * packet, wbms_cmd_req_erase_file_t * obj);

void wb_pack_GetFileCRCReq(wb_pack_element_t * packet, wbms_cmd_req_get_file_crc_t * obj);

void wb_pack_SetMonParamsDataReq(wb_pack_element_t * packet, wbms_cmd_req_set_mon_params_data_t * obj, uint8_t ** data);

/******************************************************************************
 * Pack functions for response structures
 *****************************************************************************/

void wb_pack_GenericResp(wb_pack_element_t * packet, wbms_cmd_resp_generic_t * obj);

void wb_pack_GetGpioResp(wb_pack_element_t * packet, wbms_cmd_resp_get_gpio_t * obj);

void wb_pack_OTAPHandshakeResp(wb_pack_element_t * packet, wbms_cmd_resp_otap_handshake_t * obj);

void wb_pack_OTAPStatusResp(wb_pack_element_t * packet, wbms_cmd_resp_otap_status_t * obj);

void wb_pack_GetVersionResp(wb_pack_element_t * packet, wbms_cmd_resp_get_version_t * obj);

void wb_pack_GetContextualResp(wb_pack_element_t * packet, wbms_cmd_resp_get_contextual_t * obj, uint8_t ** data);

void wb_pack_GetFileResp(wb_pack_element_t * packet, wbms_cmd_resp_get_file_t *obj, uint8_t ** data);

void wb_pack_GetFileCRCResp(wb_pack_element_t * packet, wbms_cmd_resp_get_file_crc_t *obj);

void wb_pack_GetMonParamsCRCResp(wb_pack_element_t * packet, wbms_cmd_resp_get_mon_params_crc_t * obj);

/******************************************************************************
 * Pack functions for notification structures
 *****************************************************************************/

void wb_pack_MonitorAlertDeviceNotif(wb_pack_element_t * packet, wbms_notif_mon_alert_device_t * obj);

#ifdef __cplusplus
}
#endif
#endif //WB_PACK_WBMS_CMD_H

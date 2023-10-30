/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_PACK_WBMS_CMD_MGR_H
#define WB_PACK_WBMS_CMD_MGR_H

#include "wbms_cmd_mgr_defs.h"
#include "wb_pack_protocol.h"       /* SPI protocol type declarations */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Pack functions for request structures
 *****************************************************************************/

void wb_pack_ConnectReq(wb_pack_element_t * packet, wbms_cmd_req_connect_t * obj);

void wb_pack_SetModeReq(wb_pack_element_t * packet, wbms_cmd_req_set_mode_t * obj);

void wb_pack_SendDataReq(wb_pack_element_t * packet, wbms_cmd_req_send_data_t * obj, uint8_t ** const data);

void wb_pack_RotateKeyReq(wb_pack_element_t * packet, wbms_cmd_req_rotate_key_t * obj);

void wb_pack_SetAclReq(wb_pack_element_t * packet, wbms_cmd_req_set_acl_t * obj, uint8_t ** data);

void wb_pack_GetAclReq(wb_pack_element_t * packet, wbms_cmd_req_get_acl_t * obj);

void wb_pack_DMHApplyReq(wb_pack_element_t * packet, wbms_cmd_req_dmh_apply_t * obj);

/******************************************************************************
 * Pack functions for response structures
 *****************************************************************************/

void wb_pack_ConnectResp(wb_pack_element_t * packet, wbms_cmd_resp_connect_t * obj);

void wb_pack_QueryDeviceResp(wb_pack_element_t * packet, wbms_cmd_resp_query_device_t * obj);

void wb_pack_GetAclResp(wb_pack_element_t * packet, wbms_cmd_resp_get_acl_t * obj, uint8_t ** data);

void wb_pack_SetFaultModeReq(wb_pack_element_t * packet, wbms_cmd_req_fault_service_t * obj);

void wb_pack_SetModeResp(wb_pack_element_t * packet, wbms_cmd_resp_set_mode_t * obj);

/******************************************************************************
 * Pack functions for notification structures
 *****************************************************************************/

void wb_pack_SensorDataNotif(wb_pack_element_t * packet, wbms_notif_sensor_data_t * obj, uint8_t ** const data);

void wb_pack_PktReceivedNotif(wb_pack_element_t * packet, wbms_notif_packet_received_t * obj, uint8_t ** const data);

void wb_pack_NodeStateNotif(wb_pack_element_t * packet, wbms_notif_node_state_t * obj);

void wb_pack_HealthReportNotif(wb_pack_element_t * packet, wbms_notif_health_report_t * obj, uint8_t ** const data);

void wb_pack_NotifAck(wb_pack_element_t * packet, wbms_notif_ack_t * obj);

void wb_pack_SecurityErrorNotif(wb_pack_element_t * packet, wbms_notif_security_error_t * obj);

void wb_pack_MonitorAlertSystemNotif(wb_pack_element_t * packet, wbms_notif_mon_alert_system_t * obj);

void wb_pack_SystemStatusNotif(wb_pack_element_t * packet, wbms_notif_system_status_t * obj);

void wb_pack_M2MCommLossNotif(wb_pack_element_t * packet, wbms_notif_m2m_comm_loss_t * obj);

void wb_pack_DMHAssessNotif (wb_pack_element_t * packet, wbms_notif_dmh_assess_t * obj);

void wb_pack_DMHApplyNotif (wb_pack_element_t * packet, wbms_notif_dmh_apply_t * obj);

void wb_pack_NodeModeMismatchNotif (wb_pack_element_t * packet, wbms_notif_node_mode_mismatch_t * obj);

#ifdef __cplusplus
}
#endif
#endif //WB_PACK_WBMS_CMD_MGR_H

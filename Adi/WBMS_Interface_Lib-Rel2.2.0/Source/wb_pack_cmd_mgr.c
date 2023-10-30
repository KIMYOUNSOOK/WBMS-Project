/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include "wb_pack_cmd_mgr.h"
#include "wb_req_connect.h"
#include "wb_req_set_mode.h"
#include "wb_rsp_connect.h"
#include "wb_req_send_data.h"
#include "wb_req_rotate_key.h"
#include "wb_rsp_query_device.h"
#include "wb_ntf_packet_received.h"
#include "wb_ntf_sensor_data.h"
#include "wb_ntf_node_state.h"
#include "wb_ntf_security_error.h"
#include "wb_ntf_mon_alert_system.h"
#include "wb_ntf_system_status.h"
#include "wb_ntf_m2m_comm_loss.h"
#include "wb_ntf_dmh_assess.h"
#include "wb_ntf_dmh_apply.h"
#include "wb_ntf_ack.h"
#include "wb_protocol_sph.h"
#include "wb_ntf_health_report.h"
#include "wb_req_set_acl.h"
#include "wb_req_get_acl.h"
#include "wb_rsp_get_acl.h"
#include "wb_req_fault_service.h"
#include "wb_req_dmh_apply.h"
#include "wb_rsp_set_mode.h"
#include "wb_ntf_node_mode_mismatch.h"

/******************************************************************************
 * Request structures
 ******************************************************************************/

void wb_pack_ConnectReq(wb_pack_element_t * packet, wbms_cmd_req_connect_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iDeviceId);
}

void wb_pack_SetModeReq(wb_pack_element_t * packet, wbms_cmd_req_set_mode_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iMode);
}

void wb_pack_SendDataReq(wb_pack_element_t * packet, wbms_cmd_req_send_data_t * obj, uint8_t ** const data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iDeviceId);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    wb_packer_uint8(&packet->packer, &obj->iHighPriority);
    wb_packer_uint8(&packet->packer, &obj->iPortId);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_RotateKeyReq(wb_pack_element_t * packet, wbms_cmd_req_rotate_key_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iDeviceId);
}

void wb_pack_SetAclReq(wb_pack_element_t * packet, wbms_cmd_req_set_acl_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iCount);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_GetAclReq(wb_pack_element_t * packet, wbms_cmd_req_get_acl_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iIndex);
}

void wb_pack_SetFaultModeReq(wb_pack_element_t * packet, wbms_cmd_req_fault_service_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iEnable);
}

void wb_pack_DMHApplyReq(wb_pack_element_t * packet, wbms_cmd_req_dmh_apply_t * obj)
{
    wb_packer_uint16 (&packet->packer, &obj->iToken);
    wb_packer_uint8 (&packet->packer, &obj->iType);
}

/******************************************************************************
 * Response structures
 *****************************************************************************/

void wb_pack_ConnectResp(wb_pack_element_t * packet, wbms_cmd_resp_connect_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iSessionId);
    wb_packer_uint8(&packet->packer, &obj->iProtocolVersion);
    wb_packer_uint8(&packet->packer, &obj->iManagerNumber);
    wb_packer_uint8(&packet->packer, &obj->iMode);
    wb_packer_uint8(&packet->packer, &obj->iNodeCount);
    wb_packer_block(&packet->packer, &obj->iNodeStatusMask[0], 8u);
    wb_packer_uint8(&packet->packer, &obj->iMaxNodeCount);
    wb_packer_uint8(&packet->packer, &obj->iMaxBMSPacketsPerNode);
    wb_packer_uint8(&packet->packer, &obj->iMaxPMSPackets);
    wb_packer_uint8(&packet->packer, &obj->iPMSEnabledManagers);
    wb_packer_uint8(&packet->packer, &obj->iMaxEnvironmentalPackets);
    wb_packer_uint32(&packet->packer, &obj->iConfigurationHash);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_QueryDeviceResp(wb_pack_element_t * packet, wbms_cmd_resp_query_device_t * obj)
{
    wb_packer_block(&packet->packer, &(obj->MAC[0]), WBMS_MAC_ADDR_LEN);
    wb_packer_block(&packet->packer, &(obj->PeerMAC[0]), WBMS_MAC_ADDR_LEN);
    wb_packer_uint8(&packet->packer, &obj->isStandalone);
    wb_packer_uint8(&packet->packer, &obj->iMaxNodeCount);
    wb_packer_uint8(&packet->packer, &obj->iMaxBMSPacketsPerNode);
    wb_packer_uint8(&packet->packer, &obj->iMaxPMSPackets);
    wb_packer_uint8(&packet->packer, &obj->iPMSEnabledManagers);
    wb_packer_uint8(&packet->packer, &obj->iMaxEnvironmentalPackets);
    wb_packer_uint32(&packet->packer, &obj->iConfigurationHash);
    wb_packer_uint8(&packet->packer, &obj->iEncryptionEnabledFlag);
    wb_packer_block(&packet->packer, &(obj->Nonce[0u]), WBMS_SPI_NONCE_SIZE);
    wb_packer_uint16(&packet->packer, &obj->iVersionMajor);
    wb_packer_uint16(&packet->packer, &obj->iVersionMinor);
    wb_packer_uint16(&packet->packer, &obj->iVersionPatch);
    wb_packer_uint16(&packet->packer, &obj->iVersionBuild);
    wb_packer_uint32(&packet->packer, &obj->iReserved0);
    wb_packer_uint32(&packet->packer, &obj->iReserved1);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_GetAclResp(wb_pack_element_t * packet, wbms_cmd_resp_get_acl_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iCount);
    wb_packer_uint8(&packet->packer, &obj->rc);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_SetModeResp(wb_pack_element_t * packet, wbms_cmd_resp_set_mode_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iBitMapSize);
    wb_packer_block(&packet->packer, &(obj->nodeBitMap[0u]), WBMS_NODE_BITMAP_SIZE);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

/******************************************************************************
 * Notification structures
 *****************************************************************************/

void wb_pack_SensorDataNotif(wb_pack_element_t * packet, wbms_notif_sensor_data_t * obj, uint8_t ** const data)
{
    wb_packer_uint8(&packet->packer, &obj->iTotalPackets);
    wb_packer_uint8(&packet->packer, &obj->iPacketIndex);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_PktReceivedNotif(wb_pack_element_t * packet, wbms_notif_packet_received_t * obj, uint8_t ** const data)
{
    wb_packer_uint8(&packet->packer, &(obj->iDeviceId));
    wb_packer_uint64(&packet->packer, &(obj->iASN));
    wb_packer_uint32(&packet->packer, &(obj->iSequenceNum));
    wb_packer_uint16(&packet->packer, &(obj->iLength));
    wb_packer_uint16(&packet->packer, &(obj->iLatency));
    wb_packer_uint8(&packet->packer, &(obj->iPort));
    wb_packer_uint8(&packet->packer, &(obj->iTwoHopFlag));
    wb_packer_uint8(&packet->packer, (uint8_t*)&(obj->iRSSI));
    wb_packer_uint8(&packet->packer, &(obj->iChannel));
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_NodeStateNotif(wb_pack_element_t * packet, wbms_notif_node_state_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iNotifId);
    wb_packer_uint8(&packet->packer, &obj->iDeviceID);
    wb_packer_uint8(&packet->packer, &obj->iState);
}

void wb_pack_HealthReportNotif(wb_pack_element_t * packet, wbms_notif_health_report_t * obj, uint8_t ** const data)
{
    wb_packer_uint16(&packet->packer, &obj->iNotifId);
    wb_packer_uint8(&packet->packer, &obj->iDeviceId);
    wb_packer_uint16(&packet->packer, &(obj->iSequenceNumber));
    wb_packer_uint64(&packet->packer, &(obj->iASN));
    wb_packer_uint8(&packet->packer, &(obj->iLength));
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_SecurityErrorNotif(wb_pack_element_t * packet, wbms_notif_security_error_t * obj)
{
    wb_packer_uint16(&packet->packer, &(obj->iNotifId));
    wb_packer_uint8(&packet->packer, &(obj->iNotificationType));
    wb_packer_block(&packet->packer, &(obj->MAC[0]), WBMS_MAC_ADDR_LEN);
}

void wb_pack_MonitorAlertSystemNotif(wb_pack_element_t * packet, wbms_notif_mon_alert_system_t * obj)
{
    wb_packer_uint8(&packet->packer, &obj->iEventedManagers);
    wb_packer_block(&packet->packer, &(obj->iEventedNodes[0]), WBMS_NODE_BITMAP_SIZE);
}

void wb_pack_SystemStatusNotif(wb_pack_element_t * packet, wbms_notif_system_status_t * obj)
{
    wb_packer_uint64(&packet->packer, &(obj->iASN));
    wb_packer_uint16(&packet->packer, &(obj->iSWVersionMajor));
    wb_packer_uint16(&packet->packer, &(obj->iSWVersionMinor));
    wb_packer_uint32(&packet->packer, &(obj->iSpiTxQueueOFCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiRxCrcErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiDevXferErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiSWXferErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiAbortCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiTxFrameAllocErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiTxMsgAllocErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiRxFrameAllocErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iSpiRxMsgAllocErrorCount));
    wb_packer_uint32(&packet->packer, &(obj->iIdleFramesSinceLastMsg));
    wb_packer_uint16(&packet->packer, &(obj->iFlash0OneBitEccErrCount));
    wb_packer_uint16(&packet->packer, &(obj->iFlash0TwoBitEccErrCount));
    wb_packer_uint32(&packet->packer, &(obj->iFlash0LastEccErrAddr));
    wb_packer_uint16(&packet->packer, &(obj->iFlash1OneBitEccErrCount));
    wb_packer_uint16(&packet->packer, &(obj->iFlash1TwoBitEccErrCount));
    wb_packer_uint32(&packet->packer, &(obj->iFlash1LastEccErrAddr));
    wb_packer_uint16(&packet->packer, &(obj->iFlashWriteErrCount));
    wb_packer_uint8(&packet->packer, &(obj->iLffsFreePercent));
    wb_packer_uint16(&packet->packer, &(obj->iLffsDefragCount));
    wb_packer_uint32(&packet->packer, &(obj->iLffsStatus));
}

void wb_pack_M2MCommLossNotif(wb_pack_element_t * packet, wbms_notif_m2m_comm_loss_t * obj)
{
    wb_packer_uint16(&packet->packer, &(obj->iNotifId));
}

void wb_pack_DMHAssessNotif (wb_pack_element_t * packet, wbms_notif_dmh_assess_t * obj)
{
    wb_packer_uint16 (&packet->packer, &(obj->iNotifId));
    wb_packer_block (&packet->packer, (uint8_t *) &(obj->iRssiDeltas [0]), WBMS_MAX_NODES);
    wb_packer_uint8 (&packet->packer, (uint8_t *) &(obj->iSignalFloorImprovement));
    wb_packer_uint8 (&packet->packer, &(obj->rc));
}

void wb_pack_DMHApplyNotif (wb_pack_element_t * packet, wbms_notif_dmh_apply_t * obj)
{
    wb_packer_uint16 (&packet->packer, &(obj->iNotifId));
    wb_packer_uint8 (&packet->packer, &(obj->rc));
}

void wb_pack_NodeModeMismatchNotif (wb_pack_element_t * packet, wbms_notif_node_mode_mismatch_t * obj)
{
    wb_packer_uint16 (&packet->packer, &(obj->iNotifId));
    wb_packer_uint8 (&packet->packer, &(obj->iDeviceID));
}


/******************************************************************************
 * Notification Acknowledgement structures
 *******************************************************************************/

void wb_pack_NotifAck(wb_pack_element_t * packet, wbms_notif_ack_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iNotifId);
}

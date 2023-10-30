/******************************************************************************
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_pack_cmd.h"
#include "wb_req_generic.h"
#include "wb_rsp_generic.h"
#include "wb_req_set_gpio.h"
#include "wb_req_get_gpio.h"
#include "wb_rsp_get_gpio.h"
#include "wb_req_otap_hs.h"
#include "wb_rsp_otap_hs.h"
#include "wb_req_otap_data.h"
#include "wb_rsp_otap_status.h"
#include "wb_req_select_script.h"
#include "wb_req_modify_script.h"
#include "wb_rsp_get_version.h"
#include "wb_req_set_contextual.h"
#include "wb_req_get_contextual.h"
#include "wb_rsp_get_contextual.h"
#include "wb_req_set_customer_identifier.h"
#include "wb_req_get_file.h"
#include "wb_rsp_get_file.h"
#include "wb_req_erase_file.h"
#include "wb_req_get_file_crc.h"
#include "wb_rsp_get_file_crc.h"
#include "wb_ntf_node_state.h"
#include "wb_ntf_health_report.h"
#include "wb_ntf_ack.h"
#include "wb_ntf_mon_alert_device.h"
#include "wb_ntf_m2m_comm_loss.h"
#include "wb_req_set_mon_params_data.h"
#include "wb_rsp_get_mon_params_crc.h"
#include "wb_pack_protocol.h"
#include "wb_crc_16.h"
#include "wb_crc_config.h"

/******************************************************************************
 * Request structures
 *****************************************************************************/

void wb_pack_GenericReq(wb_pack_element_t * packet, wbms_cmd_req_generic_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
}

void wb_pack_SetGpioReq(wb_pack_element_t * packet, wbms_cmd_req_set_gpio_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iGPIOId);
    wb_packer_uint8(&packet->packer, &obj->iValue);
}

void wb_pack_GetGpioReq(wb_pack_element_t * packet, wbms_cmd_req_get_gpio_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iGPIOId);
}

void wb_pack_OTAPHandshakeReq(wb_pack_element_t * packet, wbms_cmd_req_otap_handshake_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iFileType);
    wb_packer_block(&packet->packer, &obj->iHeaderData[0], WB_WIL_OTAP_FILE_HEADER_LEN);
}

void wb_pack_OTAPDataReq(wb_pack_element_t * packet, wbms_cmd_req_otap_data_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &(obj->iToken));
    wb_packer_uint16(&packet->packer, &(obj->iBlockNumber));
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_SelectScriptReq(wb_pack_element_t * packet, wbms_cmd_req_select_script_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iSensorId);
    wb_packer_uint8(&packet->packer, &obj->iScriptId);
}

void wb_pack_ModifyScriptReq(wb_pack_element_t * packet, wbms_cmd_req_modify_script_t * obj, uint16_t * hdr_crc, uint8_t ** data)
{
    uint16_t iMetadataStartOffset;
    uint16_t const iMetadataLength = WBMS_CMD_REQ_MODIFY_SCRIPT_METADATA_LEN;

    wb_packer_uint16(&packet->packer, &obj->iToken);

    iMetadataStartOffset = packet->packer.index; /* Beginning of header metadata to calculate CRC over */

    wb_packer_uint8(&packet->packer, &obj->iSensorId);
    wb_packer_uint16(&packet->packer, &obj->iActivationTime);
    wb_packer_uint16(&packet->packer, &obj->iOffset);
    wb_packer_uint8(&packet->packer, &obj->iLength);

    /* Directly calculate CRC-16 over metadata */
    *hdr_crc = wb_crc_ComputeCRC16 (&packet->origin [iMetadataStartOffset], iMetadataLength, WB_CRC_SEED);

    /* Provide parent function a pointer to location of script change payload */
    *data = &packet->packer.buf[packet->packer.index + sizeof(uint16_t)];
}

void wb_pack_SetContextualReq(wb_pack_element_t *packet, wbms_cmd_req_set_contextual_t *obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iContextualId);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_GetContextualReq(wb_pack_element_t * packet, wbms_cmd_req_get_contextual_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iContextualId);
}

void wb_pack_SetCustomerIdentifierReq(wb_pack_element_t * packet, wbms_cmd_req_set_customer_identifier_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iCustomerIdentifierId);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_GetFileReq(wb_pack_element_t *packet, wbms_cmd_req_get_file_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint16(&packet->packer, &obj->iOffset);
    wb_packer_uint8(&packet->packer, &obj->iFileType);
}

void wb_pack_GetFileCRCReq(wb_pack_element_t * packet, wbms_cmd_req_get_file_crc_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iFileType);
}

void wb_pack_EraseFileReq(wb_pack_element_t * packet, wbms_cmd_req_erase_file_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iFileType);
}

void wb_pack_SetMonParamsDataReq(wb_pack_element_t * packet, wbms_cmd_req_set_mon_params_data_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint16(&packet->packer, &obj->iOffset);
    wb_packer_uint16(&packet->packer, &obj->iCRC);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    *data = &packet->packer.buf[packet->packer.index];
}

/******************************************************************************
 * Response structures
 *****************************************************************************/

void wb_pack_GenericResp(wb_pack_element_t * packet, wbms_cmd_resp_generic_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_GetGpioResp(wb_pack_element_t * packet, wbms_cmd_resp_get_gpio_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iValue);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_OTAPHandshakeResp(wb_pack_element_t * packet, wbms_cmd_resp_otap_handshake_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint32(&packet->packer, &obj->iFileSize);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_OTAPStatusResp(wb_pack_element_t * packet, wbms_cmd_resp_otap_status_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint16(&packet->packer, &obj->iIndex);
    wb_packer_block(&packet->packer, &obj->MissingBlocks[0], WBMS_OTAP_MISSING_BLOCK_MASK_LEN);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_GetVersionResp(wb_pack_element_t * packet, wbms_cmd_resp_get_version_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint16(&packet->packer, &obj->iVersionMajor);
    wb_packer_uint16(&packet->packer, &obj->iVersionMinor);
    wb_packer_uint16(&packet->packer, &obj->iVersionPatch);
    wb_packer_uint16(&packet->packer, &obj->iVersionBuild);
    wb_packer_uint16(&packet->packer, &obj->iSiliconVersion);
    wb_packer_uint16(&packet->packer, &obj->iCPVersionMajor);
    wb_packer_uint16(&packet->packer, &obj->iCPVersionMinor);
    wb_packer_uint16(&packet->packer, &obj->iCPVersionPatch);
    wb_packer_uint16(&packet->packer, &obj->iCPVersionBuild);
    wb_packer_uint16(&packet->packer, &obj->iCPSiliconVersion);
    wb_packer_uint32(&packet->packer, &obj->iLifeCycleInfo);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_GetContextualResp(wb_pack_element_t * packet, wbms_cmd_resp_get_contextual_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    wb_packer_uint8(&packet->packer, &obj->rc);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_GetFileResp(wb_pack_element_t * packet, wbms_cmd_resp_get_file_t * obj, uint8_t ** data)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint16(&packet->packer, &obj->iRemainingBytes);
    wb_packer_uint8(&packet->packer, &obj->iLength);
    wb_packer_uint8(&packet->packer, &obj->rc);
    *data = &packet->packer.buf[packet->packer.index];
}

void wb_pack_GetFileCRCResp(wb_pack_element_t *packet, wbms_cmd_resp_get_file_crc_t *obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint32(&packet->packer, &obj->iCRC);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

void wb_pack_GetMonParamsCRCResp(wb_pack_element_t * packet, wbms_cmd_resp_get_mon_params_crc_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iToken);
    wb_packer_uint32(&packet->packer, &obj->iCRC);
    wb_packer_uint8(&packet->packer, &obj->rc);
}

/******************************************************************************
 * Notification structures
 *****************************************************************************/

void wb_pack_MonitorAlertDeviceNotif(wb_pack_element_t * packet, wbms_notif_mon_alert_device_t * obj)
{
    wb_packer_uint16(&packet->packer, &obj->iAlertTypes);
    wb_packer_block(&packet->packer, &(obj->iChannels[0]), WBMS_FAULT_CHANNELS_SIZE);
}

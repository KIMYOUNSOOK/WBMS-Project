/******************************************************************************
 * @file     wb_nil_response.c
 *
 * @brief    Contains function definitions for individual command responses
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#include "wb_nil_packet.h"
#include "wb_pack_cmd.h"
#include "wb_pack_cmd_mgr.h"
#include "wb_pack_cmd_node.h"
#include "wb_rsp_connect.h"
#include "wb_ntf_packet_received.h"
#include "wb_ntf_sensor_data.h"
#include "wb_ntf_node_state.h"
#include "wb_ntf_health_report.h"
#include "wb_ntf_security_error.h"
#include "wb_ntf_system_status.h"
#include "wb_ntf_m2m_comm_loss.h"
#include "wb_rsp_generic.h"
#include "wb_rsp_get_state_of_health.h"
#include "wb_rsp_otap_hs.h"
#include "wb_rsp_otap_status.h"
#include "wb_rsp_get_version.h"
#include "wb_rsp_get_gpio.h"
#include "wb_rsp_get_contextual.h"
#include "wb_rsp_get_file_crc.h"
#include "wb_rsp_get_file.h"
#include "wb_rsp_set_mode.h"
#include "wb_rsp_query_device.h"
#include "wb_rsp_get_acl.h"
#include "wb_wil_connect.h"
#include "wb_wil_setmode.h"
#include "wb_wil_reset.h"
#include "wb_wil_capture_network_data.h"
#include "wb_wil_get_file.h"
#include "wb_wil_load_file.h"
#include "wb_wil_node_state.h"
#include "wb_wil_hr.h"
#include "wb_wil_system_status.h"
#include "wb_wil_setgpio.h"
#include "wb_wil_getgpio.h"
#include "wb_wil_modifyscript.h"
#include "wb_wil_selectscript.h"
#include "wb_wil_set_contextual.h"
#include "wb_wil_get_contextual.h"
#include "wb_wil_set_customer_identifier.h"
#include "wb_wil_set_state_of_health.h"
#include "wb_wil_get_state_of_health.h"
#include "wbms_port_config.h"
#include "wbms_cmd_node_defs.h"
#include "adi_wil_port.h"
#include "wb_wil_get_file.h"
#include "wb_wil_erase_file.h"
#include "wb_wil_inventory_transition.h"
#include "wb_wil_get_file_crc.h"
#include "wb_wil_rotate_key.h"
#include "wb_wil_fault_service.h"
#include "wb_wil_query_device.h"
#include "wb_wil_get_version.h"
#include "wb_wil_security_error.h"
#include "wb_wil_m2m_comm_loss.h"
#include "wb_wil_set_acl.h"
#include "wb_wil_get_acl.h"
#include "wb_wil_utils.h"
#include "wb_wil_fault_notif.h"
#include "wb_ntf_mon_alert_device.h"
#include "wb_ntf_mon_alert_system.h"
#include "wb_ntf_dmh_assess.h"
#include "wb_ntf_dmh_apply.h"
#include "wb_ntf_node_mode_mismatch.h"
#include "wb_wil_get_monitor_params_crc.h"
#include "wb_wil_update_monitor_params.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_msg_header.h"
#include "wb_xms.h"
#include "wb_wil_xms_sequence_generation.h"
#include "wb_scl.h"
#include "wb_wil_dmh.h"
#include "wb_assl.h"
#include <string.h>

/******************************************************************************
 *  Static functions
 *****************************************************************************/

static void wb_nil_packet_ProcessCommonResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement, uint8_t iMessageId);
static void wb_nil_packet_ProcessNodeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement);
static void wb_nil_HandlePacketReceivedNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement);

/* Manager commands */
static void wb_nil_HandleConnectResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetModeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleClearACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleGetACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleSendDataResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleRotateKeyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleFaultServiceResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleQueryDeviceResponse (adi_wil_port_t * const pPort, wb_pack_element_t * pElement);
static void wb_nil_HandleAssessNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleApplyNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);

static void wb_nil_HandleSecurityErrorNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleEMSNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandlePmsDataNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleNodeStateNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleHealthReportNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSystemStatusNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleFaultSummaryNotif (adi_wil_pack_internals_t const * const pInternals, wb_pack_element_t * pElement);
static void wb_nil_HandleM2MCommLossNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement);
static void wb_nil_HandleDMHAssessNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t * pElement);
static void wb_nil_HandleDMHApplyNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t * pElement);
static void wb_nil_HandleNodeModeMismatchNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t * pElement);

/* Node Commands */
static void wb_nil_HandleSetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleInventoryTransitionResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);

/* Common Commands */
static void wb_nil_HandleResetResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleOTAPHandshakeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleOTAPDataResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleOTAPStatusResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleOTAPCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetGPIOResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetGPIOResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetVersionResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSelectScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleModifyScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetContextualResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetContextualResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleEraseFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetFileCRCResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetCustomerIdentifierResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleFaultReportNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetMonParamsDataResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleSetMonParamsCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement);
static void wb_nil_HandleGetMonParamsCRCResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement);

/******************************************************************************
 *  Function definitions
 *****************************************************************************/

adi_wil_err_t wb_nil_packet_Process (adi_wil_port_t * const pPort, wb_pack_element_t * const pElement, uint8_t iMessageId)
{
    uint64_t iDeviceId;
    adi_wil_err_t rc;

    /* Validate input parameters - port and pack element */
    if (((void *) 0 == pPort) || ((void *) 0 == pElement))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
		
    /* Validate input parameter - - port's internal variables */
    else if ((void *) 0 == pPort->Internals.pPackInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Once input parameters are valid, the handler for the message 
         * ID would be called. So, it is safe to assign return code as 
         * SUCCESS.*/
        rc = ADI_WIL_ERR_SUCCESS;
        iDeviceId = (pPort == pPort->Internals.pPackInternals->pManager0Port) ? ADI_WIL_DEV_MANAGER_0 : ADI_WIL_DEV_MANAGER_1;

        switch (iMessageId)
        {
            /*  Handle packet received notification */
            case WBMS_NOTIF_PACKET_RECIEVED:
                wb_nil_HandlePacketReceivedNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for the command, set mode */
            case WBMS_CMD_SET_MODE:
                wb_nil_HandleSetModeResponse (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for the command, send data */
            case WBMS_CMD_SEND_DATA:
                wb_nil_HandleSendDataResponse (pPort->Internals.pPackInternals, pElement);
                break;
            /*  Handle ACL cases: setting, clearing and getting ACL */
            case WBMS_CMD_CLEAR_ACL:
                wb_nil_HandleClearACLResponse (pPort->Internals.pPackInternals, pElement);
                break;
            case WBMS_CMD_SET_ACL:
                wb_nil_HandleSetACLResponse (pPort->Internals.pPackInternals, pElement);
                break;
            case WBMS_CMD_GET_ACL:
                wb_nil_HandleGetACLResponse (pPort->Internals.pPackInternals, pElement);
                break;
            /*  Handle for notification of PMS data */
            case WBMS_NOTIF_PMS_DATA:
                wb_nil_HandlePmsDataNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for notification of state of node */
            case WBMS_NOTIF_NODE_STATE:
                wb_nil_HandleNodeStateNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for notification of health reports */
            case WBMS_NOTIF_HEALTH_REPORT:
                wb_nil_HandleHealthReportNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for notification of system status */
            case WBMS_NOTIF_SYSTEM_STATUS:
                wb_nil_HandleSystemStatusNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for notification of security error */
            case WBMS_NOTIF_SECURITY_ERROR:
                wb_nil_HandleSecurityErrorNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for notification of EMS data */
            case WBMS_NOTIF_EMS_DATA:
                wb_nil_HandleEMSNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for the command : set to fault service mode */
            case WBMS_CMD_SET_FAULT_SERVICE_MODE:
                wb_nil_HandleFaultServiceResponse (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for the command : DMH access */
            case WBMS_CMD_DMH_ASSESS:
                wb_nil_HandleAssessNetworkTopologyResponse (pPort->Internals.pPackInternals, pElement);
                break;
            /*  Handle for the command : DMH apply */
            case WBMS_CMD_DMH_APPLY:
                wb_nil_HandleApplyNetworkTopologyResponse (pPort->Internals.pPackInternals, pElement);
                break;
            /*  Handle for the command : query device */
            case WBMS_CMD_QUERY_DEVICE:
                wb_nil_HandleQueryDeviceResponse (pPort, pElement);
                break;
            /*  Handle for the notification of fault summary */
            case WBMS_NOTIF_MON_ALERT_SYSTEM:
                wb_nil_HandleFaultSummaryNotif (pPort->Internals.pPackInternals, pElement);
                break;
            /*  Handle for the notification of M2M communication loss */
            case WBMS_NOTIF_M2M_COMM_LOSS:
                wb_nil_HandleM2MCommLossNotif (pPort->Internals.pPackInternals, iDeviceId, pElement);
                break;
            /*  Handle for the notification of DMH assess */
            case WBMS_NOTIF_DMH_ASSESS:
                wb_nil_HandleDMHAssessNotif (pPort->Internals.pPackInternals, pPort, pElement);
                break;
            /*  Handle for the notification of DMH apply */
            case WBMS_NOTIF_DMH_APPLY:
                wb_nil_HandleDMHApplyNotif (pPort->Internals.pPackInternals, pPort, pElement);
                break;
            /*  Handle for the notification of node mode mismatch */
            case WBMS_NOTIF_NODE_MODE_MISMATCH:
                wb_nil_HandleNodeModeMismatchNotif (pPort->Internals.pPackInternals, pPort, pElement);
                break;
            default:
                /* Call a common response if none of the above 
                * message IDs match */
                wb_nil_packet_ProcessCommonResponse (pPort->Internals.pPackInternals, iDeviceId, pElement, iMessageId);
                break;
        }
    }

    return rc;
}

static void wb_nil_HandlePacketReceivedNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement)
{
    wbms_notif_packet_received_t obj = { 0 };
    uint8_t * pData;
    wb_msg_header_t MsgHeader;
    wb_xms_metadata_t XmsMetadata;

    /* Only proceed if size is within limits */
    if (pElement->size >= WBMS_CMD_NOTIF_PACKET_RECEIVED_LEN)
    {
        wb_pack_PktReceivedNotif (pElement, &obj, &pData);

        /* Input device ID shall be within max. limits */
        if (obj.iDeviceId >= ADI_WIL_MAX_NODES)
        {
            /* Do nothing : invalid device id */
        }
        else
        {
            switch (obj.iPort)
            {
                case WB_OPCMD_PORT_ID:
                case WB_DEVMON_PORT_ID:
                    wb_nil_packet_ProcessNodeResponse (pInternals, (1ULL << obj.iDeviceId), pElement);
                    break;

                case WB_BMS_PORT_ID:
                    MsgHeader.iPayloadLength = (uint8_t) (obj.iLength & (uint8_t)0xFF);
                    MsgHeader.iSourceDeviceId = obj.iDeviceId;
                    XmsMetadata.eType = ADI_WIL_XMS_BMS;
                    wb_wil_HandleXmsMeasurement (pInternals, &MsgHeader, &XmsMetadata, pData, iDeviceId);
                    break;

                case WB_SCL_PORT_ID:
                    if (obj.iLength <= (uint16_t)UINT8_MAX)
                    {
                        wb_scl_HandleSCLFrame (pInternals->pPack, obj.iDeviceId, (uint8_t) obj.iLength, pData);
                    }
                    break;

                default:
                    ; /* TODO : Log error */
                    break;
            }

            /* Generate notification to application that network   */
            /* data (RSSI, latency, etc) are available to be read */
            wb_wil_NetworkDataNotify (pInternals, iDeviceId, &obj);
        }
    }
}

static void wb_nil_packet_ProcessCommonResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement, uint8_t iMessageId)
{
    /* Call the required response handler that is needed */
    /* for the corresponding message ID */
    switch (iMessageId)
    {
        case WBMS_CMD_RESET:
            /* Call reset handler response when command is reset */
            wb_nil_HandleResetResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_FILE:
            /* Call getFile handler response when command is get file */
            wb_nil_HandleGetFileResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_ERASE_FILE:
            /* Call erase file handler response when command is erase file */
            wb_nil_HandleEraseFileResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_FILE_CRC:
            /* Call get file CRC handler response when command is get file CRC */
            wb_nil_HandleGetFileCRCResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_OTAP_HANDSHAKE:
            /* Call OTAP handshake handler response when command is OTAP handshake */
            wb_nil_HandleOTAPHandshakeResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_OTAP_DATA:
            /* Call OTAP data handler response when command is OTAP data */
            wb_nil_HandleOTAPDataResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_OTAP_STATUS:
            /* Call OTAP status handler response when command is OTAP status */
            wb_nil_HandleOTAPStatusResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_OTAP_COMMIT:
            /* Call OTAP commit handler response when command is OTAP commit */
            wb_nil_HandleOTAPCommitResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_VERSION:
            /* Call get version handler response when command is get version */
            wb_nil_HandleGetVersionResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_SET_GPIO:
            /* Call set GPIO handler response when command is set GPIO */
            wb_nil_HandleSetGPIOResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_GPIO:
            /* Call get GPIO handler response when command is get GPIO */
            wb_nil_HandleGetGPIOResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_SELECT_SCRIPT:
            /* Call select script handler response when command is select script */
            wb_nil_HandleSelectScriptResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_MODIFY_SCRIPT:
            /* Call modify script handler response when command is modify script */
            wb_nil_HandleModifyScriptResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_SET_CONTEXTUAL_DATA:
            /* Call set contextual data handler response when command is set contextual data */
            wb_nil_HandleSetContextualResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_CONTEXTUAL_DATA:
            /* Call get contextual data handler response when command is get contextual data */
            wb_nil_HandleGetContextualResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_ROTATE_KEY:
            /* Call rotate key handler response when command is rotate key */
            wb_nil_HandleRotateKeyResponse (pInternals, pElement);
            break;
        case WBMS_CMD_SET_CUSTOMER_IDENTIFIER:
            /* Call set customer ID handler response when command is set customer ID */
            wb_nil_HandleSetCustomerIdentifierResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_NOTIF_MON_ALERT_DEVICE:
            /* Call fault report handler response when command is fault report */
            wb_nil_HandleFaultReportNotif (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_SET_MON_PARAMS_DATA:
            /* Call set monitor parameters data handler response when command is set monitor parameters data */
            wb_nil_HandleSetMonParamsDataResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_SET_MON_PARAMS_COMMIT:
            /* Call set monitor parameters commit handler response when command is set monitor parameters commit */
            wb_nil_HandleSetMonParamsCommitResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_MON_PARAMS_CRC:
            /* Call get monitor parameters CRC handler response when command is get monitor parameters CRC */
            wb_nil_HandleGetMonParamsCRCResponse (pInternals, pElement);
            break;
        default:
            ; /* Added for MISRA rule 16.4 */
            break;
    }
}

static void wb_nil_packet_ProcessNodeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement)
{
    uint8_t iCmdId = 0u;

    /* Pack command ID in chunks of 8 bits into */
    /*the packet to be sent out */
    wb_packer_uint8 (&pElement->packer, &iCmdId);

    switch (iCmdId)
    {
        case WBMS_CMD_SET_STATE_OF_HEALTH:
            wb_nil_HandleSetStateOfHealthResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_GET_STATE_OF_HEALTH:
            wb_nil_HandleGetStateOfHealthResponse (pInternals, iDeviceId, pElement);
            break;
        case WBMS_CMD_INVENTORY_STATE_TRANSITION:
            wb_nil_HandleInventoryTransitionResponse (pInternals, iDeviceId, pElement);
            break;
        default:
            /* If command ID does not match with any of the above,*/
            /* call the common response routine */
            wb_nil_packet_ProcessCommonResponse (pInternals, iDeviceId, pElement, iCmdId);
            break;
    }
}

adi_wil_err_t wb_nil_packet_ProcessLoggedOutPacket (adi_wil_port_t * const pPort, wb_pack_element_t * const pElement, uint8_t iMessageId)
{
    adi_wil_err_t rc;
    uint64_t iDeviceId;

    /* Initialize return code with SUCCESS, to start with */
    rc = ADI_WIL_ERR_SUCCESS;

    /* Validate the input port */
    if (pPort != (void *) 0)
    {
        if (iMessageId == WBMS_CMD_QUERY_DEVICE)
        {
            wb_nil_HandleQueryDeviceResponse (pPort, pElement);
        }
        else if ((iMessageId == WBMS_CMD_CONNECT) && (pPort->Internals.pPackInternals != (void *) 0))
        {
            /* Find Device ID depending upon the active port */
            iDeviceId = (pPort == pPort->Internals.pPackInternals->pManager0Port) ? ADI_WIL_DEV_MANAGER_0 : ADI_WIL_DEV_MANAGER_1;
            wb_nil_HandleConnectResponse (pPort->Internals.pPackInternals, iDeviceId, pElement);
        }
        else
        {
            /* Message ID other than 'query device' and 'connect' is not supported */
            rc = ADI_WIL_ERR_FAIL;
        }
    }
    else
    {
        /* Return FAIL when input port is invalid */
        rc = ADI_WIL_ERR_FAIL;
    }

    return rc;
}

static void wb_nil_HandleConnectResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    /* Initialize the structure that is to be */
    /* passed to the wrapper function */
    wbms_cmd_resp_connect_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_CONNECT_LEN)
    {
        wb_pack_ConnectResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleConnectResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetModeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_set_mode_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_SET_MODE_LEN)
    {
        wb_pack_SetModeResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetModeResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleResetResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleResetResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleOTAPHandshakeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_otap_handshake_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_OTAP_HANDSHAKE_LEN)
    {
        wb_pack_OTAPHandshakeResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleHandshakeResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleOTAPDataResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    (void) iDeviceId;

    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleDataResponse (pInternals, &obj);
    }
}

static void wb_nil_HandleOTAPStatusResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_otap_status_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_OTAP_STATUS_LEN)
    {
        wb_pack_OTAPStatusResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleStatusResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleOTAPCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleCommitResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetGPIOResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetGPIOResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleGetGPIOResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_gpio_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_GPIO_LEN)
    {
        wb_pack_GetGpioResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetGPIOResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleGetVersionResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    /* Initialize the structure that is to be passed to the wrapper function */
    wbms_cmd_resp_get_version_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_VERSION_LEN)
    {
        wb_pack_GetVersionResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetVersionResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSelectScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSelectScriptResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleModifyScriptResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleModifyScriptResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetContextualResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetContextualResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleGetContextualResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_contextual_t obj = { 0 };
    uint8_t * pData;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_CONTEXTUAL_LEN)
    {
        wb_pack_GetContextualResp (pElement, &obj, &pData);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetContextualResponse (pInternals, iDeviceId, &obj, pData);
    }
}

static void wb_nil_HandleSendDataResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleDataResponse (pInternals, &obj);
    }
}

static void wb_nil_HandleGetFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_file_t obj = { 0 };
    uint8_t * pData;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_FILE_LEN)
    {
        wb_pack_GetFileResp (pElement, &obj, &pData);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetFileResponse (pInternals, iDeviceId, &obj, pData);
    }
}

static void wb_nil_HandleEraseFileResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleEraseFileResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleGetFileCRCResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_file_crc_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_FILE_CRC_LEN)
    {
        wb_pack_GetFileCRCResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetFileCRCResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleQueryDeviceResponse (adi_wil_port_t * const pPort, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_query_device_t obj;

    (void) memset (&obj, 0, sizeof (obj));

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_QUERY_DEVICE_LEN)
    {
        wb_pack_QueryDeviceResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleQueryDeviceResponse (pPort, &obj);
    }
}

static void wb_nil_HandleSecurityErrorNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_security_error_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_NOTIF_SECURITY_ERROR_LEN)
    {
        wb_pack_SecurityErrorNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSecurityError (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleEMSNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_sensor_data_t obj = { 0 };
    uint8_t * pData;
    wb_msg_header_t MsgHeader;
    wb_xms_metadata_t XmsMetadata;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_NOTIF_SENSOR_DATA_HDR_LEN)
    {
        /* Pack the sensor data information into the packet that is to be sent out */
        wb_pack_SensorDataNotif (pElement, &obj, &pData);

        MsgHeader.iPayloadLength = obj.iLength;
        MsgHeader.iSourceDeviceId = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? WBMS_MANAGER_0_DEVICE_ID : WBMS_MANAGER_1_DEVICE_ID;
        XmsMetadata.eType = ADI_WIL_XMS_EMS;

        /* If we incorrectly assumed the sender of EMS data at connect time,
         * modify the XMS parameters and update so that this device is now the
         * enabled. */
        if (pInternals->XmsMeasurementParameters.iEMSDevices != iDeviceId)
        {
            pInternals->XmsMeasurementParameters.iEMSDevices = iDeviceId;

            wb_assl_SetMeasurementParameters (pInternals->pPack,
                                              &pInternals->XmsMeasurementParameters);
        }

        /* Perform non-fusa measurement */
        wb_wil_HandleXmsMeasurement (pInternals, &MsgHeader, &XmsMetadata, pData, iDeviceId);
    }
}

static void wb_nil_HandlePmsDataNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_sensor_data_t obj = { 0 };
    uint8_t * pData;
    wb_msg_header_t MsgHeader;
    wb_xms_metadata_t XmsMetadata;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_NOTIF_SENSOR_DATA_HDR_LEN)
    {
        /* Pack the sensor data information into the packet that is to be sent out */
        wb_pack_SensorDataNotif (pElement, &obj, &pData);

        MsgHeader.iPayloadLength = obj.iLength;
        MsgHeader.iSourceDeviceId = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? WBMS_MANAGER_0_DEVICE_ID : WBMS_MANAGER_1_DEVICE_ID;
        XmsMetadata.eType = ADI_WIL_XMS_PMS;

        /* Perform non-fusa measurement */
        wb_wil_HandleXmsMeasurement (pInternals, &MsgHeader, &XmsMetadata, pData, iDeviceId);
    }
}

static void wb_nil_HandleNodeStateNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_node_state_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_NOTIF_NODE_STATE_LEN)
    {
        wb_pack_NodeStateNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleNodeState (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleHealthReportNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_health_report_t obj = { 0 };
    uint8_t * pData;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_NOTIF_HR_HDR_LEN)
    {
        wb_pack_HealthReportNotif (pElement, &obj, &pData);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleHealthReport (pInternals, iDeviceId, &obj, pData);
    }
}

static void wb_nil_HandleSystemStatusNotif (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    /* Initialize the structure that is to be */
    /* passed to the wrapper function */
    wbms_notif_system_status_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_SYSTEM_STATUS_LEN)
    {
        /* Pack system status information to be sent to the core function */
        wb_pack_SystemStatusNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSystemStatus (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t setStateOfHealthResp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &setStateOfHealthResp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetStateOfHealthResponse (pInternals, iDeviceId, &setStateOfHealthResp);
    }
}

static void wb_nil_HandleGetStateOfHealthResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_state_of_health_t getStateOfHealthResp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RSP_GET_SOH_LEN)
    {
        wb_pack_GetStateOfHealthResp (pElement, &getStateOfHealthResp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetStateOfHealthResponse (pInternals, iDeviceId, &getStateOfHealthResp);
    }
}

static void wb_nil_HandleInventoryTransitionResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t inventoryTransResp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &inventoryTransResp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleInventoryTransitionResponse (pInternals, iDeviceId, &inventoryTransResp);
    }
}

static void wb_nil_HandleRotateKeyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleRotateKeyResponse (pInternals, &obj);
    }
}

static void wb_nil_HandleFaultServiceResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleFaultServicingResponse (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetCustomerIdentifierResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t Response = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &Response);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetCustomerIdentifierResponse (pInternals, iDeviceId, &Response);
    }
}

static void wb_nil_HandleClearACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleClearACLResponse (pInternals, &Resp);
    }
}

static void wb_nil_HandleSetACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleSetACLResponse (pInternals, &Resp);
    }
}

static void wb_nil_HandleGetACLResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_acl_t Resp = { 0 };
    uint8_t * pData;

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_ACL_LEN)
    {
        wb_pack_GetAclResp (pElement, &Resp, &pData);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetACLResponse (pInternals, &Resp, pData);
    }
}

static void wb_nil_HandleAssessNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleAssessNetworkTopologyResponse (pInternals, &obj);
    }
}

static void wb_nil_HandleApplyNetworkTopologyResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleApplyNetworkTopologyResponse (pInternals, &obj);
    }
}

static void wb_nil_HandleFaultSummaryNotif (adi_wil_pack_internals_t const * const pInternals, wb_pack_element_t * pElement)
{
    wbms_notif_mon_alert_system_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_MON_ALERT_SYSTEM_LEN)
    {
        wb_pack_MonitorAlertSystemNotif (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleFaultSummaryNotif (pInternals, &Resp);
    }
}

static void wb_nil_HandleFaultReportNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_notif_mon_alert_device_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_MON_ALERT_DEVICE_LEN)
    {
        wb_pack_MonitorAlertDeviceNotif (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleFaultReportNotif (pInternals, iDeviceId, &Resp);
    }
}

static void wb_nil_HandleM2MCommLossNotif (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wb_pack_element_t * const pElement)
{
    wbms_notif_m2m_comm_loss_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_M2M_COMM_LOSS_LEN)
    {
        wb_pack_M2MCommLossNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleM2MCommLossNotif (pInternals, iDeviceId, &obj);
    }
}

static void wb_nil_HandleSetMonParamsDataResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleUpdateMonParamDataResponse (pInternals, iDeviceId, &Resp);
    }
}

static void wb_nil_HandleSetMonParamsCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_generic_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GENERIC_LEN)
    {
        wb_pack_GenericResp (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleUpdateMonParamCommitResponse (pInternals, iDeviceId, &Resp);
    }
}

static void wb_nil_HandleGetMonParamsCRCResponse (adi_wil_pack_internals_t * const pInternals, wb_pack_element_t * pElement)
{
    wbms_cmd_resp_get_mon_params_crc_t Resp = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_CMD_RESP_GET_MON_PARAMS_CRC_LEN)
    {
        wb_pack_GetMonParamsCRCResp (pElement, &Resp);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleGetMonitorParametersCRCResponse (pInternals, &Resp);
    }
}

static void wb_nil_HandleDMHAssessNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t *  pElement)
{
    wbms_notif_dmh_assess_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_DMH_ASSESS_LEN)
    {
        wb_pack_DMHAssessNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleAssessTopologyNotif (pInternals, pPort, &obj);
    }
}

static void wb_nil_HandleDMHApplyNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t * pElement)
{
    wbms_notif_dmh_apply_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_DMH_APPLY_LEN)
    {
        wb_pack_DMHApplyNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleApplyTopologyNotif (pInternals, pPort, &obj);
    }
}

static void wb_nil_HandleNodeModeMismatchNotif (adi_wil_pack_internals_t const * const pInternals, adi_wil_port_t * const pPort, wb_pack_element_t * pElement)
{
    wbms_notif_node_mode_mismatch_t obj = { 0 };

    /* Validate size before proceeding */
    if (pElement->size >= WBMS_NOTIF_NODE_MODE_MISMATCH_LEN)
    {
        wb_pack_NodeModeMismatchNotif (pElement, &obj);
        /* Call core function from */
        /* this wrapper function */
        wb_wil_HandleNodeModeMismatchNotif (pInternals, pPort, &obj);
    }
}

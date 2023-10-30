/******************************************************************************
* @file    wb_wil_security_error.c
*
* @brief   Handle Security Error Notification.
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*****************************************************************************/

#include "wb_wil_security_error.h"
#include "wbms_cmd_mgr_defs.h"
#include "adi_wil_pack_internals.h"
#include "wb_ntf_security_error.h"
#include "wb_wil_ack.h"
#include "wb_wil_utils.h"
#include "wb_wil_ui.h"

/******************************************************************************
 * Static Function Declarations
 *****************************************************************************/

static void wb_wil_GetSecurityErrorFromUint (uint8_t iSecurityError, adi_wil_event_id_t * const pSecurityError);

/******************************************************************************
 * Static Function Definitions
 *****************************************************************************/

static void wb_wil_GetSecurityErrorFromUint (uint8_t iSecurityError, adi_wil_event_id_t * const pSecurityError)
{
    /* Convert security error code from uint8_t to adi_wil_event_id_t* */
    switch (iSecurityError)
    {
        case WBMS_CYS_EVT_NODE_NOT_IN_ACL:
            /* Node not present in ACL case */
            *pSecurityError = ADI_WIL_EVENT_SEC_NODE_NOT_IN_ACL;
            break;
        case WBMS_CYS_EVT_CERTIFICATE_CALC_ERROR:
            /* Certificate calculation error case */
            *pSecurityError = ADI_WIL_EVENT_SEC_CERTIFICATE_CALCULATION_ERROR;
            break;
        case WBMS_CYS_EVT_JOIN_NO_KEY:
            /* No join key case */
            *pSecurityError = ADI_WIL_EVENT_SEC_JOIN_NO_KEY;
            break;
        case WBMS_CYS_EVT_JOIN_DUPLICATE_COUNTER:
            /* Join duplicate counter case */
            *pSecurityError = ADI_WIL_EVENT_SEC_JOIN_DUPLICATE_JOIN_COUNTER;
            break;
        case WBMS_CYS_EVT_JOIN_MIC_FAILURE:
            /* Join MIC failure case */
            *pSecurityError = ADI_WIL_EVENT_SEC_JOIN_MIC_FAILED;
            break;
        case WBMS_CYS_EVT_SEC_ERR_MIC:
            /* Session MIC failure case */
            *pSecurityError = ADI_WIL_EVENT_SEC_SESSION_MIC_FAILED;
            break;
        case WBMS_CYS_EVT_SEC_ERR:
            /* Unknown error case */
            *pSecurityError = ADI_WIL_EVENT_SEC_UNKNOWN_ERROR;
            break;
        case WBMS_CYS_EVT_SECERR_M2M_JOIN_CNTR:
            /* M2M join counter error case */
            *pSecurityError = ADI_WIL_EVENT_SEC_M2M_JOIN_CNTR_ERROR;
            break;
        case WBMS_CYS_EVT_SECERR_M2M_SESS_CNTR:
            /* M2M session counter error case */
            *pSecurityError = ADI_WIL_EVENT_SEC_M2M_SESSION_CNTR_ERROR;
            break;
        case WBMS_CYS_EVT_SECERR_EXCHCERT_LOCK:
            /* Certificate exchange lock error case */
            *pSecurityError = ADI_WIL_EVENT_SEC_CERTIFICATE_EXCHANGE_LOCK_ERROR;
            break;
        default:
            /* If there is no match with none of the above errors,
             * return with an error code of UNKNOWN ERROR */
            *pSecurityError = ADI_WIL_EVENT_SEC_UNKNOWN_ERROR;
            break;
    }
}

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

void wb_wil_HandleSecurityError (adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wbms_notif_security_error_t const * const pNotif)
{
    adi_wil_port_t * pPort;
    adi_wil_event_id_t eSecurityError;

    wb_wil_GetSecurityErrorFromUint (pNotif->iNotificationType, &eSecurityError);
    /* Generate event with the MAC address that has created the security
     * notification error */
    wb_wil_ui_GenerateEvent (pInternals->pPack, eSecurityError, &pNotif->MAC [0]);

    pPort = (iDeviceId == ADI_WIL_DEV_MANAGER_0) ? pInternals->pManager0Port : pInternals->pManager1Port;

    if (((void *) 0 != pPort) && (pNotif->iNotifId != 0u))
    {
        if (ADI_WIL_ERR_SUCCESS != wb_wil_ack_Put (&pPort->Internals.AckQueue, pNotif->iNotifId, WBMS_NOTIF_SECURITY_ERROR))
        {
            /* Keep track of the lost ACK count */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iAckLostCount);
        }
    }
}

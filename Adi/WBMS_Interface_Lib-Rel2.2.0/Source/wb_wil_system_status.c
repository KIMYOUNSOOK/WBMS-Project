/******************************************************************************
 * @file    wb_wil_system_status.c
 *
 * @brief   Handle System Status Notification.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_system_status.h"
#include "wbms_cmd_mgr_defs.h"
#include "adi_wil_pack_internals.h"
#include "wb_ntf_system_status.h"
#include "wb_wil_utils.h"
#include "wb_wil_ui.h"
#include <string.h>

void wb_wil_HandleSystemStatus (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_notif_system_status_t const * const pNotif)
{
    adi_wil_mgr_status_msg_t * status;
    bool bMgrSPIOverflowOccurred;

    if (iDeviceId == ADI_WIL_DEV_MANAGER_0)
    {
        /* Populate manager 0's status message structure */
        status = &pInternals->Stats.MgrStats.Manager0StatusMsg;
        status->iDeviceId = WBMS_MANAGER_0_DEVICE_ID;
    }
    else
    {
        /* Populate manager 1's status message structure */
        status = &pInternals->Stats.MgrStats.Manager1StatusMsg;
        status->iDeviceId = WBMS_MANAGER_1_DEVICE_ID;
    }

    /* Update the manager statistics. If the manager SPI queue
     * overflow counter has increased, then generate an overflow event */

    if ((pNotif->iSpiTxQueueOFCount) > (status->iSpiTxQueueOFCount))
    {
        bMgrSPIOverflowOccurred = true;
    }
    else
    {
        bMgrSPIOverflowOccurred = false;
    }

    /* Populate the internal status structure */
    status->iPacketGenerationTime = pNotif->iASN;
    /* Populate software version major version's value */
    status->iSWVersionMajor = pNotif->iSWVersionMajor;
    /* Populate software version minor version's value */
    status->iSWVersionMinor = pNotif->iSWVersionMinor;
    /* Populate transmit queue count value */
    status->iSpiTxQueueOFCount = pNotif->iSpiTxQueueOFCount;
    /* Populate SPI receive CRC error value */
    status->iSpiRxCrcErrorCount = pNotif->iSpiRxCrcErrorCount;
    /* Populate SPI device transfer error value */
    status->iSpiDevXferErrorCount = pNotif->iSpiDevXferErrorCount;
    /* Populate SPI SW transfer error value */
    status->iSpiSWXferErrorCount = pNotif->iSpiSWXferErrorCount;
    /* Populate number of SPI abort value */
    status->iSpiAbortCount = pNotif->iSpiAbortCount;
    /* Populate SPI transmit frame allocation error value */
    status->iSpiTxFrameAllocErrorCount = pNotif->iSpiTxFrameAllocErrorCount;
    /* Populate SPI transmit message allocation error value */
    status->iSpiTxMsgAllocErrorCount = pNotif->iSpiTxMsgAllocErrorCount;
    /* Populate SPI receive frame allocation error value */
    status->iSpiRxFrameAllocErrorCount = pNotif->iSpiRxFrameAllocErrorCount;
    /* Populate SPI receive message allocation error value */
    status->iSpiRxMsgAllocErrorCount = pNotif->iSpiRxMsgAllocErrorCount;
    /* Populate number of idle frames since last message value */
    status->iIdleFramesSinceLastMsg = pNotif->iIdleFramesSinceLastMsg;
    /* Populate Flash 0's One bit ECC error value */
    status->iFlash0OneBitEccErrCount = pNotif->iFlash0OneBitEccErrCount;
    /* Populate Flash 0's Two bit ECC error value */
    status->iFlash0TwoBitEccErrCount = pNotif->iFlash0TwoBitEccErrCount;
    /* Populate Flash 0's last ECC error value */
    status->iFlash0LastEccErrAddr = pNotif->iFlash0LastEccErrAddr;
    /* Populate Flash 1's One bit ECC error value */
    status->iFlash1OneBitEccErrCount = pNotif->iFlash1OneBitEccErrCount;
    /* Populate Flash 1's Two bit ECC error value */
    status->iFlash1TwoBitEccErrCount = pNotif->iFlash1TwoBitEccErrCount;
    /* Populate Flash 1's Last error of flash value */
    status->iFlash1LastEccErrAddr = pNotif->iFlash1LastEccErrAddr;
    /* Populate flash write error count value */
    status->iFlashWriteErrCount = pNotif->iFlashWriteErrCount;
    /* Populate percentage of free space value */
    status->iLffsFreePercent = pNotif->iLffsFreePercent;
    /* Populate de-fragmentation count value */
    status->iLffsDefragCount = pNotif->iLffsDefragCount;
    /* Populate status value */
    status->iLffsStatus = pNotif->iLffsStatus;

    if (bMgrSPIOverflowOccurred == true)
    {
        /* If a SPI queue overflow has happened, then generate an event
         * to notify the host application */
        wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_MGR_QUEUE_OVERFLOW, &iDeviceId);
    }
}

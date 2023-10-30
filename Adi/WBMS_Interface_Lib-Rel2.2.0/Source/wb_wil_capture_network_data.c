/******************************************************************************
 * @file    wb_wil_capture_network_data.c
 *
 * @brief   Capture network metadata from BMS/environmental measurement
 *          packets
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_capture_network_data.h"
#include "adi_wil_network_data.h"
#include "adi_wil_network_data_buffer.h"
#include "wb_wil_utils.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_api.h"
#include "wb_wil_ui.h"

#include <string.h>
#include <stdbool.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_EnableNetworkDataCaptureAPI (adi_wil_pack_internals_t * const pInternals,
                                                  adi_wil_network_data_t * pDataBuffer,
                                                  uint16_t iDataBufferCount,
                                                  bool bEnable)
{
    adi_wil_err_t rc;

    /* Validate input parameters */
    if ((pInternals == (void *) 0) || (iDataBufferCount == 0u) || (pDataBuffer == (void *) 0))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        rc = ADI_WIL_ERR_SUCCESS;
    }

    if (rc == ADI_WIL_ERR_SUCCESS)
    {
        pInternals->NetDataBuffer.iCount = 0u;

        /* Only make the capture data buffer valid if bEnable is set to true */
        if (bEnable)
        {
            pInternals->NetDataBuffer.pData = pDataBuffer;
            pInternals->NetDataBuffer.iDataBufferCount = iDataBufferCount;
            (void) memset (pDataBuffer, 0, (((uint32_t) iDataBufferCount) * sizeof (adi_wil_network_data_t)));
            pInternals->Stats.NetworkPktStats.iNwkDataRdyNotifCount = 0u;
            pInternals->Stats.NetworkPktStats.iPktReceivedCount = 0u;
            pInternals->Stats.NetworkPktStats.iPktCapturedCount = 0u;
        }
        else
        {
            /* Network data capture function is not enabled (via input to the
             * API) */
            pInternals->NetDataBuffer.pData = (void *) 0;
            pInternals->NetDataBuffer.iDataBufferCount = 0u;
        }
    }

    return rc;
}

void wb_wil_NetworkDataNotify (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_notif_packet_received_t const * const pElement)
{
    adi_wil_network_data_t * pNetData;

    /* Validate input parameters */
    if ((pInternals != (void *) 0) && (pElement != (void *) 0))
    {
        /* Update the count which keeps track of the number of network packets
         * received */
        wb_wil_IncrementWithRollover32 (&pInternals->Stats.NetworkPktStats.iPktReceivedCount);

        if ((pInternals->NetDataBuffer.iDataBufferCount != 0u) && (pInternals->NetDataBuffer.pData != (void *) 0))
        {
            pNetData = &pInternals->NetDataBuffer.pData [pInternals->NetDataBuffer.iCount];

            /* Update network internal structure with latest network data */
            pNetData->iPacketGenerationTime = pElement->iASN;
            pNetData->iLatency = pElement->iLatency;
            pNetData->iSequenceNumber = pElement->iSequenceNum;
            pNetData->iChannel = pElement->iChannel;
            pNetData->iRSSI = pElement->iRSSI;
            pNetData->iReserved = pElement->iTwoHopFlag;

            /* Proceed only if device ID is within max node limits */
            if (pElement->iDeviceId < ADI_WIL_MAX_NODES)
            {
                pNetData->eSrcDeviceId = wb_wil_GetExternalDeviceId (pElement->iDeviceId);
                pNetData->eSrcManagerId = iDeviceId;

                /* Update internal network statistics data */
                wb_wil_IncrementWithRollover16 (&pInternals->NetDataBuffer.iCount);
                wb_wil_IncrementWithRollover32 (&pInternals->Stats.NetworkPktStats.iPktCapturedCount);

                if (pInternals->NetDataBuffer.iCount >= pInternals->NetDataBuffer.iDataBufferCount)
                {
                    /* Notify the application that data is available to be
                     * read */
                    wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_DATA_READY_NETWORK_DATA, &pInternals->NetDataBuffer);

                    pInternals->NetDataBuffer.iCount = 0u;
                    wb_wil_IncrementWithRollover32 (&pInternals->Stats.NetworkPktStats.iNwkDataRdyNotifCount);
                }
            }
        }
    }
}

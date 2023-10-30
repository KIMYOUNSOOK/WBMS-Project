/******************************************************************************
 * @file    wb_wil_xms_sequence_generation.c
 *
 * @brief   Non-FuSa XMS Sequence Number Generation via Timestamp
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_xms_sequence_generation.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_msg_header.h"
#include "wb_xms.h"
#include "wb_wil_utils.h"
#include "wbms_cmd_mgr_defs.h"

#include <stdint.h>
#include <string.h>

/******************************************************************************
 *   #defines
 *****************************************************************************/

/* Time in milliseconds that the new timestamp cannot underflow the existing to
 * be considered a "new" interval instead of late */
#define WB_XMS_TIMESTAMP_ROLLOVER_DIFFERENCE (1000u)

/* Index in the arrays for Manager 0 */
#define WB_XMS_MANAGER_0_INDEX (62u)

/* Index in the arrays for Manager 1 */
#define WB_XMS_MANAGER_1_INDEX (63u)

/******************************************************************************
 *   Local function declarations
 *****************************************************************************/

static bool wb_wil_ValidateParameters (adi_wil_pack_internals_t const * const pInternals,
                                       wb_msg_header_t const * const pMsgHeader,
                                       wb_xms_metadata_t const * const pXmsMetadata,
                                       uint8_t const * const pData,
                                       uint64_t iDeviceId);

static bool wb_wil_GetDeviceIndex (uint8_t * pDeviceIndex,
                                   uint8_t iSourceDeviceId);

static bool wb_wil_ProcessXMSTimestamp (adi_wil_xms_timestamp_conversion_state_t * pState,
                                        uint8_t const * const pData,
                                        uint8_t iDeviceIndex,
                                        uint8_t iPacketsPerInterval);

static bool wb_wil_CheckXMSTimestampDistance (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                              adi_wil_xms_timestamp_state_t * const pThisTimestamp,
                                              adi_wil_xms_timestamp_state_t * const pOtherTimestamp,
                                              uint32_t iNewTimestamp,
                                              uint8_t iPacketsPerInterval);

static void wb_wil_RollIntervalForward (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                        uint8_t iPacketsPerInterval);

static void wb_wil_ClearCurrentIntervalState (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                              uint8_t iPacketsPerInterval);

/******************************************************************************
 *   Public functions
 *****************************************************************************/

void wb_wil_HandleXmsMeasurement (adi_wil_pack_internals_t * const pInternals,
                                  wb_msg_header_t * const pMsgHeader,
                                  wb_xms_metadata_t * const pXmsMetadata,
                                  uint8_t const * const pData,
                                  uint64_t iDeviceId)
{
    /* Pointer to XMS timestamp state */
    adi_wil_xms_timestamp_conversion_state_t * pState;
    
    /* Pointer to XMS statistics */
    adi_wil_xms_pkt_statistics_t * pStats;

    /* Storage for the number of packets per device in an interval */
    uint8_t iPacketsPerInterval;

    /* Storage for this device's index in the arrays (0-63) */
    uint8_t iDeviceIndex;

    /* Initialize local variables */
    iDeviceIndex = 0u;

    /* Validate input parameters before dereferencing */
    if (!wb_wil_ValidateParameters (pInternals,
                                    pMsgHeader,
                                    pXmsMetadata,
                                    pData,
                                    iDeviceId))
    {
        /* Do nothing - invalid input parameters */
    }
    else if (!wb_wil_GetDeviceIndex (&iDeviceIndex,
                                     pMsgHeader->iSourceDeviceId))
    {
        /* Do nothing - invalid source device ID */
    }
    else
    {
        /* Select statistics and XMS Timestamp state based on message type */
        if (pXmsMetadata->eType == ADI_WIL_XMS_BMS)
        {
            pStats = &pInternals->Stats.BmsPktStats;
            pState = &pInternals->BmsTimestampConversionState;
            iPacketsPerInterval = pInternals->XmsMeasurementParameters.iBMSPackets;
        }
        else if (pXmsMetadata->eType == ADI_WIL_XMS_PMS)
        {
            pStats = &pInternals->Stats.PmsPktStats;
            pState = &pInternals->PmsTimestampConversionState;
            iPacketsPerInterval = pInternals->XmsMeasurementParameters.iPMSPackets;
        }
        else
        {
            pStats = &pInternals->Stats.EmsPktStats;
            pState = &pInternals->EmsTimestampConversionState;
            iPacketsPerInterval = pInternals->XmsMeasurementParameters.iEMSPackets;
        }

        /* Increment the packet count for the relevant manager */
        if (iDeviceId == ADI_WIL_DEV_MANAGER_0)
        {
            wb_wil_IncrementWithRollover32 (&pStats->iManager0PktCount);
        }
        else
        {
            wb_wil_IncrementWithRollover32 (&pStats->iManager1PktCount);
        }

        /* Check the timestamp to determine if this is an old, current or new
         * interval */
        if (wb_wil_ProcessXMSTimestamp (pState,
                                        pData,
                                        iDeviceIndex,
                                        iPacketsPerInterval))
        {
            /* CERT-C Precondition check on parameters */
            if ((uint8_t) UINT8_MAX < (pState->iBaseSequenceNumber +
                                       pState->iMeasurementsReceived [iDeviceIndex]))
            {
                /* Do nothing - expect rollover */
            }

            /* Add the "base" sequence number to the number of measurements
             * received so far to generate the message sequence number */
            pMsgHeader->iSequenceNumber = pState->iBaseSequenceNumber +
                                          pState->iMeasurementsReceived [iDeviceIndex];

            /* If this is the first measurement in the interval for this device,
             * select as a START message */
            pXmsMetadata->eCmdId = (pState->iMeasurementsReceived [iDeviceIndex] == 0u) ? ADI_WIL_XMS_START_MEASUREMENT :
                                                                                          ADI_WIL_XMS_SUPPLEMENTAL_MEASUREMENT;

            /* If we've rolled the interval forward, we want to flush the
             * buffer. This prevents late packets being bundled with new
             * packets if the sequence number was not initialized yet */
            if (pState->bFlushRequired)
            {
                /* Invoke XMS flush function for this data type */
                wb_xms_Flush (pInternals->pPack, pXmsMetadata->eType);
                
                /* Reset flush flag */
                pState->bFlushRequired = false;
            }

            /* Check we haven't already submitted a full set of packets for this
             * device in the interval */
            if (pState->iMeasurementsReceived [iDeviceIndex] >= iPacketsPerInterval)
            {
                /* Message was a duplicate in a complete interval - reject  */
                wb_wil_IncrementWithRollover32 (&pStats->iRejectedPktCount);
            }
            /* Attempt to store the packet... */
            else if (ADI_WIL_ERR_SUCCESS != wb_xms_HandleMeasurement (pInternals->pPack,
                                                                      pMsgHeader,
                                                                      pXmsMetadata,
                                                                      pData))
            {
                /* Message was a duplicate in an incomplete interval - reject */
                wb_wil_IncrementWithRollover32 (&pStats->iRejectedPktCount);
            }
            /* Else, we were able to store it so... */
            else
            {
                /* Increment the count of measurements received for this device */
                pState->iMeasurementsReceived [iDeviceIndex]++;
            }
        }
    }
}

void wb_wil_XmsStandbyTransition (adi_wil_pack_internals_t * const pInternals)
{
    /* Validate input parameters before dereferencing */
    if ((void *) 0 != pInternals)
    {
        /* Clear any active BMS timestamp conversion state */
        wb_wil_ClearCurrentIntervalState (&pInternals->BmsTimestampConversionState,
                                          pInternals->XmsMeasurementParameters.iBMSPackets);

        /* Clear any active PMS timestamp conversion state */
        wb_wil_ClearCurrentIntervalState (&pInternals->PmsTimestampConversionState,
                                          pInternals->XmsMeasurementParameters.iPMSPackets);

        /* Clear any active EMS timestamp conversion state */
        wb_wil_ClearCurrentIntervalState (&pInternals->EmsTimestampConversionState,
                                          pInternals->XmsMeasurementParameters.iEMSPackets);

        /* Flush any non-FuSa packets in the buffer to the user */
        wb_xms_Flush (pInternals->pPack, ADI_WIL_XMS_BMS);
        wb_xms_Flush (pInternals->pPack, ADI_WIL_XMS_PMS);
        wb_xms_Flush (pInternals->pPack, ADI_WIL_XMS_EMS);
    }
}

/******************************************************************************
 *   Local function definitions
 *****************************************************************************/

static bool wb_wil_ValidateParameters (adi_wil_pack_internals_t const * const pInternals,
                                       wb_msg_header_t const * const pMsgHeader,
                                       wb_xms_metadata_t const * const pXmsMetadata,
                                       uint8_t const * const pData,
                                       uint64_t iDeviceId)
{
    /* Return true if:
     * 1. pInternals is not NULL
     * 2. pMsgHeader is not NULL
     * 3. pXmsMetadata is not NULL
     * 4. pData is not NULL
     * 5. iDeviceId is not 0u
     * 6. iDeviceId only has a single bit set */
    return ((pInternals != NULL) &&
            (pMsgHeader != NULL) &&
            (pXmsMetadata != NULL) &&
            (pData != NULL) &&
            (iDeviceId != 0ULL) &&
            ((iDeviceId & (iDeviceId - 1ULL)) == 0ULL));
}

static bool wb_wil_GetDeviceIndex (uint8_t * pDeviceIndex,
                                   uint8_t iSourceDeviceId)
{
    /* Return value of this function */
    bool bValid;

    /* Initialize return value to assume valid device ID */
    bValid = true;

    /* Get the device index in the bitmap */
    if (iSourceDeviceId < ADI_WIL_MAX_NODES)
    {
        *pDeviceIndex = iSourceDeviceId;
    }
    /* ... else if it's manager 0, use bit 62 */
    else if (iSourceDeviceId == WBMS_MANAGER_0_DEVICE_ID)
    {
        *pDeviceIndex = WB_XMS_MANAGER_0_INDEX;
    }
    /* ... else if it's manager 1, use bit 63 */
    else if (iSourceDeviceId == WBMS_MANAGER_1_DEVICE_ID)
    {
        *pDeviceIndex = WB_XMS_MANAGER_1_INDEX;
    }
    /* ...else, it's an invalid 8-bit device id */
    else
    {
        bValid = false;
    }

    /* Return validity to caller */
    return bValid;
}

static bool wb_wil_ProcessXMSTimestamp (adi_wil_xms_timestamp_conversion_state_t * pState,
                                        uint8_t const * const pData,
                                        uint8_t iDeviceIndex,
                                        uint8_t iPacketsPerInterval)
{
    /* Pointer to the relevant timestamp for this device */
    adi_wil_xms_timestamp_state_t * pThisTimestamp;

    /* Pointer to the other timestamp */
    adi_wil_xms_timestamp_state_t * pOtherTimestamp;

    /* Storage for the extracted timestamp */
    uint32_t iNewTimestamp;

    /* Return value of this function */
    bool bValid;

    /* Initialize return value to true indicating an accepted packet */
    bValid = true;

    /* Extract timestamp as a 32-bit value. As the value is only 24-bits, 
     * shift everything up 8-bits to simplify comparisons */
    iNewTimestamp = ((uint32_t) pData [1] << 24) |
                    ((uint32_t) pData [2] << 16) |
                    ((uint32_t) pData [3] << 8);

    /* Use the timestamp at index 0 for all devices except manager 1 */
    if (iDeviceIndex != WB_XMS_MANAGER_1_INDEX)
    {
        pThisTimestamp = &pState->TimestampState [0u];
        pOtherTimestamp = &pState->TimestampState [1u];
    }
    else
    {
        pThisTimestamp = &pState->TimestampState [1u];
        pOtherTimestamp = &pState->TimestampState [0u];
    }

    /* If we haven't already received a timestamp just use this one */
    if (!pThisTimestamp->bInitialized)
    {
        pThisTimestamp->iTimestamp = iNewTimestamp;
        pThisTimestamp->bInitialized = true;
        pThisTimestamp->bPendingChange = false;
    }
    /* Else, check the distance from the last timestamp received */
    else
    {
        bValid = wb_wil_CheckXMSTimestampDistance (pState,
                                                   pThisTimestamp,
                                                   pOtherTimestamp,
                                                   iNewTimestamp,
                                                   iPacketsPerInterval);
    }

    /* Return value to caller */
    return bValid;
}

static void wb_wil_RollIntervalForward (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                        uint8_t iPacketsPerInterval)
{
    /* Reset the individual device measurement counts for this interval */
    (void) memset (pState->iMeasurementsReceived, 0, sizeof (pState->iMeasurementsReceived));

    /* CERT-C Precondition check on parameters */
    if ((uint8_t) UINT8_MAX < (pState->iBaseSequenceNumber + iPacketsPerInterval))
    {
        /* Do nothing - expect rollover */
    }

    /* Roll forward the "base" sequence number */
    pState->iBaseSequenceNumber += iPacketsPerInterval;
}

static bool wb_wil_CheckXMSTimestampDistance (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                              adi_wil_xms_timestamp_state_t * const pThisTimestamp,
                                              adi_wil_xms_timestamp_state_t * const pOtherTimestamp,
                                              uint32_t iNewTimestamp,
                                              uint8_t iPacketsPerInterval)
{
    /* Storage for the "distance" from the current timestamp */
    uint32_t iDistance;

    /* Return value of this function */
    bool bValid;

    /* Initialize return value to true indicating an accepted packet */
    bValid = true;

    /* Calculate the distance between the timestamps */
    iDistance = iNewTimestamp - pThisTimestamp->iTimestamp;

    /* Check if timestamp matches what we currently have */
    if (iDistance == 0u)
    {
        /* If we were expecting a new timestamp but received the same timestamp
         * mark as invalid. Otherwise we're good and this is part of the
         * current interval */
        if (pThisTimestamp->bPendingChange)
        {
            bValid = false;
        }
    }
    /* Otherwise, the timestamps are different. Check if we've rolled forward
     * by a normal amount */
    else if ((iDistance < ((uint32_t) UINT32_MAX - (WB_XMS_TIMESTAMP_ROLLOVER_DIFFERENCE << 8u))))
    {
        /* Update to the new timestamp */
        pThisTimestamp->iTimestamp = iNewTimestamp;

        /* If this timestamp was pending a change, clear pending flag and return */
        if (pThisTimestamp->bPendingChange)
        {
            pThisTimestamp->bPendingChange = false;
        }
        /* Else, this is the first device to change so roll things forward */
        else
        {
            wb_wil_RollIntervalForward (pState,
                                        iPacketsPerInterval);

            /* Flag the other timestamp as now pending a change in timestamp */
            pOtherTimestamp->bPendingChange = true;

            /* Flag this state as now flushable as the timestamp has changed */
            pState->bFlushRequired = true;
        }
    }
    /* Else, we've rolled backwards. Mark as invalid */
    else
    {
        bValid = false;
    }

    return bValid;
}

static void wb_wil_ClearCurrentIntervalState (adi_wil_xms_timestamp_conversion_state_t * const pState,
                                              uint8_t iPacketsPerInterval)
{
    /* If both timestamps are pending changes, we've cycled to standby again
     * without receiving any new data. If we've a already prepared for a new
     * interval, ignore */
    if (pState->TimestampState [0u].bPendingChange &&
        pState->TimestampState [1u].bPendingChange)
    {
        /* Do nothing - already waiting for a new interval to begin */
    }
    else
    {
        /* Roll forward the "base" sequence number */
        wb_wil_RollIntervalForward (pState,
                                    iPacketsPerInterval);

        /* Mark both timestamps as now pending a change */
        pState->TimestampState [0].bPendingChange = true;
        pState->TimestampState [1].bPendingChange = true;
    }
}

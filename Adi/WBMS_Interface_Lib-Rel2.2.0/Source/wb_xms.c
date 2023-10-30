/******************************************************************************
 * @file    wb_xms.c
 *
 * @brief   XMS Buffering module
 *
 * Copyright (c) 2021-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_xms.h"
#include "wb_xms_fusa.h"
#include "wb_wil_xms_type.h"
#include "wb_wil_ui_fusa.h"
#include "wb_wil_msg_header.h"
#include "wb_scl_msg_defs.h"
#include "adi_wil_safety_internals.h"
#include "adi_wil_types.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_xms_internals.h"
#include "adi_wil_sensor_data.h"
#include "adi_wil_sensor_data_buffer.h"
#include "adi_wil_pack.h"
#include "wb_wil_device.h"

#include <string.h>

/******************************************************************************
 *   #defines
 *****************************************************************************/

/* Maximum number of packets allowed in a single interval */
#define ADI_WIL_XMS_MAX_PACKETS_PER_INT (64u)

/* Difference between a new and reference timestamp to be considered as from
 * the same dataset */
#define ADI_WIL_XMS_TIMESTAMP_TOLERANCE (2u)

/******************************************************************************
 *   Local functions
 *****************************************************************************/

static void wb_xms_InitializeStorageState (adi_wil_xms_storage_state_t * const pStorage,
                                           adi_wil_sensor_data_t * const pBuffer,
                                           uint16_t * const pBufferPosition,
                                           uint16_t iNumSlotsAllocated);

static void wb_xms_UpdateStorageStateAllocation (adi_wil_xms_storage_state_t * const pStorage,
                                                 uint64_t iDeviceMap,
                                                 uint8_t iPacketCount,
                                                 bool bFuSaContext);

static adi_wil_err_t wb_xms_HandleMeasurementCommon (adi_wil_safety_internals_t * const pInternals,
                                                     wb_msg_header_t const * const pMsgHeader,
                                                     wb_xms_metadata_t const * const pXmsMetadata,
                                                     uint8_t const * const pData,
                                                     bool bFuSaContext);

static bool wb_xms_ActivateBuffer (adi_wil_safety_internals_t const * const pInternals,
                                   adi_wil_xms_storage_state_t * const pStorage,
                                   uint8_t const * const pData);

static bool wb_xms_WriteDeviceIdFields (adi_wil_xms_storage_state_t * const pStorage);

static bool wb_xms_ValidateTimestamp (adi_wil_safety_internals_t const * const pInternals,
                                      adi_wil_xms_storage_state_t * const pStorage,
                                      uint8_t const * const pData);

static bool wb_xms_ValidateState (adi_wil_safety_internals_t const * const pInternals,
                                  adi_wil_xms_storage_state_t const * const pStorage,
                                  uint64_t iDeviceId,
                                  bool bFusaContext);

static bool wb_xms_ValidateSequence (adi_wil_safety_internals_t const * const pInternals,
                                     adi_wil_xms_storage_state_t * const pStorage,
                                     wb_msg_header_t const * const pMsgHeader,
                                     wb_xms_metadata_t const * const pXmsMetadata,
                                     uint64_t iDeviceId);

static bool wb_xms_ValidateStartSequence (adi_wil_safety_internals_t const * const pInternals,
                                          adi_wil_xms_storage_state_t * const pStorage,
                                          uint8_t iDeviceIndex,
                                          uint8_t iSequenceNumber,
                                          uint8_t iDistance);

static bool wb_xms_ValidateSupplementalSequence (adi_wil_safety_internals_t const * const pInternals,
                                                 adi_wil_xms_storage_state_t * const pStorage,
                                                 uint8_t iDeviceIndex,
                                                 uint8_t iSequenceNumber,
                                                 uint8_t iDistance);

static bool wb_xms_StorePacket (adi_wil_safety_internals_t const * const pInternals,
                                adi_wil_xms_storage_state_t * const pStorage,
                                wb_msg_header_t const * const pMsgHeader,
                                uint8_t const * const pData,
                                uint64_t iDeviceId);

static void wb_xms_SubmitBuffer (adi_wil_safety_internals_t const * const pInternals,
                                 adi_wil_xms_storage_state_t * const pStorage);

static void wb_xms_RollSequenceForward (adi_wil_xms_storage_state_t * const pStorage);

static bool wb_xms_FuSaDuplicateCheck (adi_wil_safety_internals_t const * const pInternals,
                                       adi_wil_xms_storage_state_t * const pStorage,
                                       uint32_t iTargetSlot);

static bool wb_xms_NonFuSaDuplicateCheck (adi_wil_safety_internals_t const * const pInternals,
                                          adi_wil_xms_storage_state_t * const pStorage,
                                          uint8_t const * const pData,
                                          uint32_t iStartIndex);

static void wb_xms_IncrementWithRollover32 (uint32_t * pValue);

static adi_wil_safety_internals_t * wb_xms_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack);

static adi_wil_xms_storage_state_t * wb_xms_GetStorageState (adi_wil_safety_internals_t * const pInternals,
                                                             adi_wil_xms_type_t eType);

static bool wb_xms_GetDeviceIndex (uint8_t * pDeviceIndex,
                                   uint8_t iSourceDeviceId);

static uint32_t wb_xms_GetTargetSlot (uint64_t iCollectingMap,
                                      uint64_t iDeviceId,
                                      uint8_t iNumMeasPerInt,
                                      uint8_t iDistance);

static uint8_t wb_xms_GetDistanceFromStartSequence (uint8_t iStartSequenceNumber,
                                                    uint8_t iReceivedSequenceNumber);

static uint32_t wb_xms_GetTargetSlotFromPosition (uint32_t iTargetPosition,
                                                  uint8_t iDistance);

static bool wb_xms_CheckSameTimestamp (uint32_t iTimestamp,
                                       uint32_t iReference);

/******************************************************************************
 *   Public functions
 *****************************************************************************/

adi_wil_err_t wb_xms_Initialize (adi_wil_safety_internals_t * const pInternals,
                                 adi_wil_sensor_data_t * const pDataBuffer,
                                 uint16_t iDataBufferCount)
{
    /* Return value of the function */
    adi_wil_err_t rc;

    /* Validate input parameters */
    if ((NULL == pInternals)  ||
        (NULL == pDataBuffer) ||
        (0u == iDataBufferCount))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Clear any existing state */
        (void) memset (&pInternals->XMS, 0, sizeof (pInternals->XMS));

        /* Setup XMS events */
        pInternals->XMS.BmsStorageState.eEvent = ADI_WIL_EVENT_DATA_READY_BMS;
        pInternals->XMS.PmsStorageState.eEvent = ADI_WIL_EVENT_DATA_READY_PMS;
        pInternals->XMS.EmsStorageState.eEvent = ADI_WIL_EVENT_DATA_READY_EMS;

        /* Setup XMS buffer with user parameters */
        pInternals->XMS.iXmsDataBufferCount = iDataBufferCount;
        pInternals->XMS.pXmsBuffer = pDataBuffer;

        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

void wb_xms_InitializeAllocation (adi_wil_safety_internals_t * const pInternals,
                                  uint16_t iPMSPackets,
                                  uint16_t iEMSPackets)
{
    /* Stores the current position while allocating the buffer */
    uint16_t iBufferPosition;

    /* Initialize position to index 0 */
    iBufferPosition = 0u;

    /* Check we were handed valid parameters before dereferencing */
    if (NULL != pInternals)
    {
        /* Check we 
         * 1. Haven't already allocated the buffer and 
         * 2. Have received the data buffer address and,
         * 3. Can allocate this many packets within the buffer */
        if (!pInternals->XMS.bAllocationInitialized &&
            (pInternals->XMS.pXmsBuffer != NULL) &&
            ((iPMSPackets + iEMSPackets) <= pInternals->XMS.iXmsDataBufferCount))
        {
            /* Initialize the EMS storage state */
            wb_xms_InitializeStorageState (&pInternals->XMS.EmsStorageState,
                                           pInternals->XMS.pXmsBuffer,
                                           &iBufferPosition,
                                           iEMSPackets);

            /* Initialize the PMS storage state */
            wb_xms_InitializeStorageState (&pInternals->XMS.PmsStorageState,
                                           pInternals->XMS.pXmsBuffer,
                                           &iBufferPosition,
                                           iPMSPackets);

            /* Initialize the BMS storage state with the remaining buffer */
            wb_xms_InitializeStorageState (&pInternals->XMS.BmsStorageState,
                                           pInternals->XMS.pXmsBuffer,
                                           &iBufferPosition,
                                           pInternals->XMS.iXmsDataBufferCount - (iPMSPackets + iEMSPackets));

            /* Flag this as now initialized so this cannot be changed during execution */
            pInternals->XMS.bAllocationInitialized = true;
        }
    }
}

adi_wil_err_t wb_xms_HandleMeasurement (adi_wil_pack_t const * const pPack,
                                        wb_msg_header_t const * const pMsgHeader,
                                        wb_xms_metadata_t const * const pXmsMetadata,
                                        uint8_t const * const pData)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_xms_GetSafetyInternalsPointer (pPack);

    /* Invoke handle measurement function with bFuSaContext parameter
     * identifying we are NOT being invoked via the ASSL */
    return wb_xms_HandleMeasurementCommon (pInternals,
                                           pMsgHeader,
                                           pXmsMetadata,
                                           pData,
                                           false);
}

adi_wil_err_t wb_xms_HandleFuSaMeasurement (adi_wil_safety_internals_t * const pInternals,
                                            wb_msg_header_t const * const pMsgHeader,
                                            wb_xms_metadata_t const * const pXmsMetadata,
                                            uint8_t const * const pData)
{
    /* Invoke handle measurement function with bFuSaContext parameter
     * identifying we are being invoked via the ASSL */
    return wb_xms_HandleMeasurementCommon (pInternals,
                                           pMsgHeader,
                                           pXmsMetadata,
                                           pData,
                                           true);
}

adi_wil_err_t wb_xms_CheckTimeouts (adi_wil_safety_internals_t * const pInternals,
                                    uint32_t iCurrentTicks)
{
    /* Return value of the function */
    adi_wil_err_t rc;

    /* Variables used to calculate the distance between the current internal
     * tick count and its last update */
    uint32_t iElapsedTime;

    /* Store pointers to each XMS storage */
    adi_wil_xms_storage_state_t * const eStorageStates [] =
    {
        &pInternals->XMS.PmsStorageState,
        &pInternals->XMS.EmsStorageState,
        &pInternals->XMS.BmsStorageState
    };

    rc = ADI_WIL_ERR_SUCCESS;

    /* Ensure pointer is not NULL */
    if (NULL == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If a measurement interval is active, then check if the time since
         * the last received measurement exceeds the timeout threshold */
        for (uint8_t i = 0u; i < (sizeof (eStorageStates) / sizeof (eStorageStates [0])); ++i)
        {
            if (eStorageStates [i]->bCollecting)
            {
                /* Calculate time we've been in the collecting state */
                iElapsedTime = iCurrentTicks - eStorageStates [i]->iLastTick;

                if (iElapsedTime > ADI_WIL_MEASUREMENT_TIMEOUT_MS)
                {
                    /* Submit buffer and transition INACTIVE state */
                    wb_xms_SubmitBuffer (pInternals,
                                         eStorageStates [i]);
                }
            }
        }
    }

    return rc;
}

void wb_xms_UpdateXMSAllocation (adi_wil_safety_internals_t * const pInternals,
                                 bool bFuSaContext)
{
    /* Storage to select the target XMS parameters (FuSa or non-FuSa */
    adi_wil_xms_parameters_t const * pTargetXMSParameters;

    /* Validate input parameters before dereference */
    if (NULL != pInternals)
    {
        /* Select the FuSa/Non-Fusa parameters based on the context */
        pTargetXMSParameters = bFuSaContext ? &pInternals->FuSaXMSParameters :
                                              &pInternals->XMSParameters;

        /* Update the BMS packet allocation based on the context */
        wb_xms_UpdateStorageStateAllocation (&pInternals->XMS.BmsStorageState,
                                             pTargetXMSParameters->iBMSDevices,
                                             pTargetXMSParameters->iBMSPackets,
                                             bFuSaContext);

        /* Update the PMS packet allocation based on the context */
        wb_xms_UpdateStorageStateAllocation (&pInternals->XMS.PmsStorageState,
                                             pTargetXMSParameters->iPMSDevices,
                                             pTargetXMSParameters->iPMSPackets,
                                             bFuSaContext);

        /* Update the EMS packet allocation based on the context */
        wb_xms_UpdateStorageStateAllocation (&pInternals->XMS.EmsStorageState,
                                             pTargetXMSParameters->iEMSDevices,
                                             pTargetXMSParameters->iEMSPackets,
                                             bFuSaContext);
    }
}

void wb_xms_Flush (adi_wil_pack_t const * const pPack,
                   adi_wil_xms_type_t eType)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Static variable for selecting internals based on XMS type */
    adi_wil_xms_storage_state_t * pState;

    /* Retrieve safety internals pointer */
    pInternals = wb_xms_GetSafetyInternalsPointer (pPack);

    /* Validate before dereferencing */
    if (NULL != pInternals)
    {
        /* Get the storage state */
        pState = wb_xms_GetStorageState (pInternals,
                                         eType);

        /* Only allow this function to flush the buffer if it's allocated to a
         * non-fusa context */
        if (!pState->bFuSaBuffer)
        {
            /* Submit anything that's currently buffered */
            wb_xms_SubmitBuffer (pInternals,
                                 pState);
        }
    }
}

/******************************************************************************
*   Local functions
******************************************************************************/

static void wb_xms_InitializeStorageState (adi_wil_xms_storage_state_t * const pStorage,
                                           adi_wil_sensor_data_t * const pBuffer,
                                           uint16_t * const pBufferPosition,
                                           uint16_t iNumSlotsAllocated)
{
    /* Store a pointer to the start of this XMS type's buffer */
    pStorage->pData = &pBuffer [*pBufferPosition];

    /* Store the number of packets allocated to this XMS type */
    pStorage->iNumSlotsAllocated = iNumSlotsAllocated;

    /* Update the index by the number of slots allocated */
    if ((*pBufferPosition + iNumSlotsAllocated) < (uint16_t) UINT16_MAX)
    {
        *pBufferPosition += iNumSlotsAllocated;
    }
}

static void wb_xms_UpdateStorageStateAllocation (adi_wil_xms_storage_state_t * const pStorage,
                                                 uint64_t iDeviceMap,
                                                 uint8_t iPacketCount,
                                                 bool bFuSaContext)
{
    /* Check if the buffer is already established as a FuSa buffer */
    if (!bFuSaContext && pStorage->bFuSaBuffer)
    {
        /* Do nothing - non-FuSa context cannot override settings */
    }
    else
    {
        /* If we're from a FuSa context with non-zero settings use these.
         * Flag this storage state as being used in a FuSa context */
        if (bFuSaContext && !pStorage->bFuSaBuffer &&
            ((iDeviceMap != 0ULL) || (iPacketCount != 0u)))
        {
            /* If we're switching from non-FuSa to FuSa, clear the START initialized map */
            pStorage->iSequenceInitializedMap = 0ULL;
            pStorage->bFuSaBuffer = true;
        }

        /* If the context matches the buffer config, update the params */
        if (bFuSaContext == pStorage->bFuSaBuffer)
        {
            pStorage->iDeviceMap = iDeviceMap;
            pStorage->iNumMeasPerInt = iPacketCount;
            
            /* Clear the sequence number for any now disconnected devices */
            pStorage->iSequenceInitializedMap &= iDeviceMap;
        }
    }
}

static adi_wil_err_t wb_xms_HandleMeasurementCommon (adi_wil_safety_internals_t * const pInternals,
                                                     wb_msg_header_t const * const pMsgHeader,
                                                     wb_xms_metadata_t const * const pXmsMetadata,
                                                     uint8_t const * const pData,
                                                     bool bFuSaContext)
{
    /* Return value of the function */
    adi_wil_err_t rc;

    /* Static variable for selecting internals based on XMS type */
    adi_wil_xms_storage_state_t * pStorage;

    /* Storage for 64-bit version of device ID */
    uint64_t iDeviceId;

    /* Storage for index of device in arrays and 64-bit bit-field */
    uint8_t iDeviceIndex;

    /* Initialize device index var to 0 before passing to child function */
    iDeviceIndex = 0u;

    /* Validate the input parameters before dereferencing */
    if ((NULL == pInternals) ||
        (NULL == pMsgHeader) ||
        (NULL == pXmsMetadata) ||
        (NULL == pData))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Validate size of the message and the device ID */
    else if (!wb_xms_GetDeviceIndex (&iDeviceIndex, pMsgHeader->iSourceDeviceId) ||
             (pMsgHeader->iPayloadLength > ADI_WIL_MAX_APP_PAYLOAD_SIZE))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Generate 64-bit version of Device ID */
        iDeviceId = 1ULL << iDeviceIndex;

        /* Get the storage based on XMS type */
        pStorage = wb_xms_GetStorageState (pInternals,
                                           pXmsMetadata->eType);

        /* Initialize return code to assume failure */
        rc = ADI_WIL_ERR_FAIL;

        /* If the packet received is unable to be stored, return failure */
        if (!wb_xms_ValidateState (pInternals,
                                   pStorage,
                                   iDeviceId,
                                   bFuSaContext))
        {
            /* Return failure */
        }
        else if (!wb_xms_ValidateTimestamp (pInternals,
                                            pStorage,
                                            pData))
        {
            /* Return failure */
        }
        /* else if the packet received fails a sequence check return failure */
        else if (!wb_xms_ValidateSequence (pInternals,
                                           pStorage,
                                           pMsgHeader,
                                           pXmsMetadata,
                                           iDeviceId))
        {
            /* Return failure */
        }
        /* If we're not currently collecting activate the buffer now */
        else if (!wb_xms_ActivateBuffer (pInternals,
                                         pStorage,
                                         pData))
        {
            /* Return failure */
        }
        /* else if the packet received is unable to be stored return failure */
        else if (!wb_xms_StorePacket (pInternals,
                                      pStorage,
                                      pMsgHeader,
                                      pData,
                                      iDeviceId))
        {
            /* Return failure */
        }
        /* else, we were able to store the packet. Return success */
        else
        {
            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    return rc;
}

static bool wb_xms_ValidateState (adi_wil_safety_internals_t const * const pInternals,
                                  adi_wil_xms_storage_state_t const * const pStorage,
                                  uint64_t iDeviceId,
                                  bool bFusaContext)
{
    /* Return value of the function */
    bool bValid;

    /* Initialize validity to false */
    bValid = false;

    /* Only proceed if our allocation is initialized */
    if (!pInternals->XMS.bAllocationInitialized)
    {
        /*TODO: Work out if we want to generate an event here */
    }
    /* Validate that
     * 1. The context we are receiving a packet from matches the origin set in
     *    the buffer storage state
     * 2. The device we are receiving a packet exists in the map of devices
     *    we expect packets from */
    else if ((bFusaContext != pStorage->bFuSaBuffer) ||
             ((pStorage->iDeviceMap & iDeviceId) == 0ULL))
    {
        /* A fusa measurement was received when a non-fusa measurement
         * was expected. There is a mixture of safety and non-safety
         * devices transmitting measurements in the network */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_XMS_INSERTION,
                                     NULL);
    }
    /* Validate that
     * 1.  If we are collecting:
     * 2a. The packets per interval has not changed since we've started
     *     collecting otherwise the sequence number checks will be incorrect
     * 2b. The context expected by the buffer has not changed
     * 2c. The device we are receiving from is currently being collected by
     *     this buffer */
    else if (pStorage->bCollecting &&
             ((pStorage->iCollectingNumMeasPerInt != pStorage->iNumMeasPerInt) ||
              (pStorage->bCollectingFuSa != bFusaContext) ||
              ((pStorage->iCollectingMap & iDeviceId) == 0ULL)))
    {
        /* If they things have changed, drop the packets until the interval
         * is complete */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_XMS_DROPPED,
                                     NULL);
    }
    else
    {
        bValid = true;
    }

    return bValid;
}

static bool wb_xms_ValidateTimestamp (adi_wil_safety_internals_t const * const pInternals,
                                      adi_wil_xms_storage_state_t * const pStorage,
                                      uint8_t const * const pData)
{
    /* Storage for the extracted timestamp value */
    uint32_t iTimestamp;

    /* Return value of this function */
    bool bValid;

    /* Initialize validity to true */
    bValid = true;

    /* Extract timestamp as a 32-bit value. As the value is only 24-bits,
     * shift everything up 8-bits to simplify comparisons */
    iTimestamp = ((uint32_t) pData [1] << 24) |
                 ((uint32_t) pData [2] << 16) |
                 ((uint32_t) pData [3] << 8);

    /* Only perform timestamp checks from a FuSa context as non-FuSa packets
     * are timestamp checked and the buffer flushed before arrival at the XMS
     * module */
    if (!pStorage->bFuSaBuffer)
    {
        /* Do nothing - non-FuSa and already marked as valid */
    }
    /* Else, check if we've got a packet from the current interval */
    else if (pStorage->bCollecting &&
             wb_xms_CheckSameTimestamp (iTimestamp,
                                        pStorage->iCurrentTimestamp))
    {
        /* Do nothing - same interval and already marked as valid */
    }
    /* Else, we're either not collecting or this is a new interval. Check it's
     * not from a previous interval */
    else
    {
        /* Loop through the timestamp history */
        for (uint8_t i = 0u; i < pStorage->iHistoricalTimestampCount; i++)
        {
            /* If we've got a historical match mark as invalid */
            if (wb_xms_CheckSameTimestamp (iTimestamp,
                                           pStorage->iHistoricalTimestamps [i]))
            {
                /* Generate event indicating a late packet was received */
                wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                             ADI_WIL_EVENT_XMS_DELAY,
                                             NULL);

                /* Mark as invalid and break out of loop */
                bValid = false;
                break;
            }
        }

        /* If we're still valid, we did not find a historical match. This is
         * a new timestamp so submit whatever we have. This timestamp will be
         * stored as the current in wb_xms_ActivateBuffer */
        if (bValid)
        {
            /* Submit anything that's currently buffered, rolling forward the interval */
            wb_xms_SubmitBuffer (pInternals, pStorage);
        }
    }

    /* Return validity boolean to caller */
    return bValid;
}

static bool wb_xms_ValidateSequence (adi_wil_safety_internals_t const * const pInternals,
                                     adi_wil_xms_storage_state_t * const pStorage,
                                     wb_msg_header_t const * const pMsgHeader,
                                     wb_xms_metadata_t const * const pXmsMetadata,
                                     uint64_t iDeviceId)
{
    /* Distance of sequence number from the expected start message */
    uint8_t iDistance;

    /* Storage for index of device in arrays and 64-bit bit-field */
    uint8_t iDeviceIndex;

    /* Return value of the function */
    bool bValid;

    /* Initialize validity to false */
    bValid = false;

    /* Initialize device index var to 0 before passing to child function */
    iDeviceIndex = 0u;

    /* Already validated the device ID is good so cast return value to void */
    (void) wb_xms_GetDeviceIndex (&iDeviceIndex, pMsgHeader->iSourceDeviceId);

    /* If we haven't received a START measurement for this device yet, and we've
     * now received one, use this as the first expected sequence number */
    if ((pXmsMetadata->eCmdId == ADI_WIL_XMS_START_MEASUREMENT) &&
        ((pStorage->iSequenceInitializedMap & iDeviceId) == 0ULL))
    {
        /* Store the sequence number and mark as initialized */
        pStorage->iStartSequenceNumber [iDeviceIndex] = pMsgHeader->iSequenceNumber;
        pStorage->iSequenceInitializedMap |= iDeviceId;
    }

    /* Only proceed if the sequence number has been initialized */
    if ((pStorage->iSequenceInitializedMap & iDeviceId) != 0ULL)
    {
        /* Calculate the distance from the expected START sequence number */
        iDistance = wb_xms_GetDistanceFromStartSequence (pStorage->iStartSequenceNumber [iDeviceIndex],
                                                         pMsgHeader->iSequenceNumber);

        /* Check for invalid number of packets per interval */
        if (pStorage->iNumMeasPerInt > ADI_WIL_XMS_MAX_PACKETS_PER_INT)
        {
            /* Do nothing - invalid # of packets per interval */
        }
        /* Check for a late packet (within the last two intervals) */
        else if (iDistance > ((uint8_t) UINT8_MAX - (2u * pStorage->iNumMeasPerInt)))
        {
            /* Generate event indicating a late packet was received */
            wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                         ADI_WIL_EVENT_XMS_DELAY,
                                         NULL);

            /* Increment late packet statistic */
            wb_xms_IncrementWithRollover32 (&pStorage->Stats.iLatePktCount);
        }
        /* If it's a START message, check if meets the rules */
        else if (pXmsMetadata->eCmdId == ADI_WIL_XMS_START_MEASUREMENT)
        {
            bValid = wb_xms_ValidateStartSequence (pInternals,
                                                   pStorage,
                                                   iDeviceIndex,
                                                   pMsgHeader->iSequenceNumber,
                                                   iDistance);
        }
        /* ... else it's a SUPPLEMENTAL message, check if it meets the rules */
        else
        {
            bValid = wb_xms_ValidateSupplementalSequence (pInternals,
                                                          pStorage,
                                                          iDeviceIndex,
                                                          pMsgHeader->iSequenceNumber,
                                                          iDistance);
        }
    }

    return bValid;
}

static bool wb_xms_ValidateStartSequence (adi_wil_safety_internals_t const * const pInternals,
                                          adi_wil_xms_storage_state_t * const pStorage,
                                          uint8_t iDeviceIndex,
                                          uint8_t iSequenceNumber,
                                          uint8_t iDistance)
{
    /* Return value of the function */
    bool bValid;

    /* Initialize validity to true */
    bValid = true;

    /* Check if this is the START packet we expect */
    if (iDistance == 0u)
    {
        /* Do nothing - in current interval */
    }
    /* Check if this is the start of the next interval */
    else if (iDistance == pStorage->iNumMeasPerInt)
    {
        /* Submit anything that's currently buffered, rolling forward */
        wb_xms_SubmitBuffer (pInternals, pStorage);
    }
    /* Else, if it's an unexpected jump in sequence beyond two measurement,
     * intervals, accept and re-sync to this new sequence if: 
     * 1. We're not currently in an active measurement interval, or
     * 2. We are, but haven't received any packets for this device yet. */
    else if ((iDistance >= (2u * pStorage->iNumMeasPerInt)) &&
             ((iDeviceIndex < ADI_WIL_MAX_DEVICES) &&
             (!pStorage->bCollecting ||
              ((pStorage->iReceivedMap & (1ULL << iDeviceIndex)) == 0u))))
    {
        /* Store the new sequence number for this device */
        pStorage->iStartSequenceNumber [iDeviceIndex] = iSequenceNumber;
    }
    /* Else, we've got an out of sequence packet */
    else
    {
        /* Generate event indicating out of sequence packet was received */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_XMS_DROPPED,
                                     NULL);

        /* Increment late packet statistic */
        wb_xms_IncrementWithRollover32 (&pStorage->Stats.iLatePktCount);

        /* Mark the measurement as valid */
        bValid = false;
    }

    /* Return value to caller */
    return bValid;
}

static bool wb_xms_ValidateSupplementalSequence (adi_wil_safety_internals_t const * const pInternals,
                                                 adi_wil_xms_storage_state_t * const pStorage,
                                                 uint8_t iDeviceIndex,
                                                 uint8_t iSequenceNumber,
                                                 uint8_t iDistance)
{
    /* Return value of the function */
    bool bValid;

    /* Initialize validity to true */
    bValid = true;

    /* Check if this is a SUPPLEMENTAL from this interval */
    if ((iDistance != 0u) &&
        (iDistance < pStorage->iNumMeasPerInt))
    {
        /* Do nothing - in current interval */
    }
    /* Check if this is a SUPPLEMENTAL from the next interval */
    else if ((iDistance != 0u) &&
             (iDistance != pStorage->iNumMeasPerInt) &&
             (iDistance < (2u * pStorage->iNumMeasPerInt)))
    {
        /* Submit anything that's currently buffered, rolling forward */
        wb_xms_SubmitBuffer (pInternals, pStorage);
    }
    /* Else, if it's an unexpected jump in sequence beyond two measurement,
     * intervals, accept and re-sync to this new sequence if:
     * 1. It falls on a supplemental slot, and 
     * 2a. We're not currently in an active measurement interval, or
     * 2b. We are, but haven't received any packets for this device yet. */
    else if (((iDistance % pStorage->iNumMeasPerInt) != 0u) &&
             ((iDeviceIndex < ADI_WIL_MAX_DEVICES) &&
              (!pStorage->bCollecting ||
              ((pStorage->iReceivedMap & (1ULL << iDeviceIndex)) == 0u))))
    {
        /* Find and store the START sequence number that corresponds to this
         * SUPPLEMENTAL sequence number */
        pStorage->iStartSequenceNumber [iDeviceIndex] = wb_xms_GetDistanceFromStartSequence ((iDistance % pStorage->iNumMeasPerInt),
                                                                                             iSequenceNumber);
    }
    /* Else, we've got an out of sequence packet */
    else
    {
        /* Generate event indicating an out of sequence packet was
         * received and increment statistic */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_XMS_DROPPED,
                                     NULL);

        wb_xms_IncrementWithRollover32 (&pStorage->Stats.iLatePktCount);

        /* Flag as a rejected packet */
        bValid = false;
    }

    /* Return value to caller */
    return bValid;
}

static bool wb_xms_ActivateBuffer (adi_wil_safety_internals_t const * const pInternals,
                                   adi_wil_xms_storage_state_t * const pStorage,
                                   uint8_t const * const pData)
{
    if (!pStorage->bCollecting)
    {
        /* Clear the sensor data memory before use */
        (void) memset (pStorage->pData, 0, sizeof (adi_wil_sensor_data_t) * pStorage->iNumSlotsAllocated);

        /* Initialize the tick count prior to switching to COLLECTING */
        pStorage->iLastTick = adi_wil_hal_TickerGetTimestamp ();

        /* Reset number of slots collected so far */
        pStorage->iNumSlotsCollected = 0u;

        /* Zero the map of devices that we've received measurements for */
        pStorage->iReceivedMap = 0ULL;

        /* Store a map of currently collecting devices for this interval */
        pStorage->iCollectingMap = pStorage->iDeviceMap;

        /* Store the currently collecting number of measurements */
        pStorage->iCollectingNumMeasPerInt = pStorage->iNumMeasPerInt;

        /* Store the currently collecting context of the buffer */
        pStorage->bCollectingFuSa = pStorage->bFuSaBuffer;

        /* Write the device ID fields to all slots. Flag as collecting if
         * we had enough buffer to complete the task */
        pStorage->bCollecting = wb_xms_WriteDeviceIdFields (pStorage);

        /* Extract timestamp as a 32-bit value. As the value is only 24-bits,
         * shift everything up 8-bits to simplify comparisons */
        pStorage->iCurrentTimestamp = ((uint32_t) pData [1] << 24) |
                                      ((uint32_t) pData [2] << 16) |
                                      ((uint32_t) pData [3] << 8);

        /* Check we were able to fit all packets within the allocated buffer */
        if (!pStorage->bCollecting)
        {
            /* If we can't store that many packets, generate an insufficient
             * buffer event */
            wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                         ADI_WIL_EVENT_INSUFFICIENT_BUFFER,
                                         (void *) 0);
        }
    }

    /* Return a boolean indicating if we were able to transition to the 
     * collecting state or were already collecting */
    return pStorage->bCollecting;
}

static bool wb_xms_WriteDeviceIdFields (adi_wil_xms_storage_state_t * const pStorage)
{
    /* Temporary copy of the device map to find device index within */
    uint64_t iTempDeviceMap;

    /* Storage for device ID we are currently writing to the buffer */
    uint64_t iDeviceId;

    /* Return value of this function */
    bool bSuccess;

    /* Initialize return value to successful */
    bSuccess = true;

    /* Store a local copy of the device map */
    iTempDeviceMap = pStorage->iCollectingMap;

    /* Initialize the total packet count to 0 */
    pStorage->iNumSlotsCollecting = 0u;

    /* Loop through each set bit of the device map */
    while (iTempDeviceMap != 0ULL)
    {
        /* Generate the device ID from the LSB of map */
        iDeviceId = iTempDeviceMap & ~(iTempDeviceMap - 1ULL);

        /* Clear the bit in the device map */
        iTempDeviceMap &= ~iDeviceId;

        /* Check we have sufficient storage in the buffer... */
        if (pStorage->iNumSlotsAllocated >= (pStorage->iNumSlotsCollecting +
                                             pStorage->iCollectingNumMeasPerInt))
        {
            /* Write the device id for n = number packets per device */
            for (uint8_t i = 0u; i < pStorage->iCollectingNumMeasPerInt; i++)
            {
                pStorage->pData [pStorage->iNumSlotsCollecting + i].eDeviceId = iDeviceId;
            }

            /* Increment the number of slots we are collecting */
            pStorage->iNumSlotsCollecting += pStorage->iCollectingNumMeasPerInt;
        }
        /* else, we have insufficient storage. Return an error */
        else
        {
            bSuccess = false;
            break;
        }
    }

    return bSuccess;
}

static bool wb_xms_StorePacket (adi_wil_safety_internals_t const * const pInternals,
                                adi_wil_xms_storage_state_t * const pStorage,
                                wb_msg_header_t const * const pMsgHeader,
                                uint8_t const * const pData,
                                uint64_t iDeviceId)
{
    /* The packet's final position within the buffer */
    uint32_t iTargetSlot;

    /* Distance of sequence number from the expected start message */
    uint8_t iDistance;

    /* Storage for index of device in arrays */
    uint8_t iDeviceIndex;

    /* Return value of the function */
    bool bValid;

    /* Initialize validity to false */
    bValid = false;

    /* Initialize device index var to 0 before passing to child function */
    iDeviceIndex = 0u;

    /* Already validated the device ID is good so cast return value to void */
    (void) wb_xms_GetDeviceIndex (&iDeviceIndex, pMsgHeader->iSourceDeviceId);

    /* Get the distance from the start message */
    iDistance = wb_xms_GetDistanceFromStartSequence (pStorage->iStartSequenceNumber [iDeviceIndex],
                                                     pMsgHeader->iSequenceNumber);

    /* Find the device's START packet position within the buffer */
    iTargetSlot = wb_xms_GetTargetSlot (pStorage->iCollectingMap,
                                        iDeviceId,
                                        pStorage->iCollectingNumMeasPerInt,
                                        iDistance);

    /* Perform a sanity check on the slot position */
    if (iTargetSlot < iDistance)
    {
        /* Do nothing - invalid condition */
    }
    /* If we've got a FuSaBuffer - perform the FuSa duplicate check using the
     * sequence number */
    else if (pStorage->bFuSaBuffer && wb_xms_FuSaDuplicateCheck (pInternals,
                                                                 pStorage,
                                                                 iTargetSlot))
    {
        /* Do nothing - Event already generated. Discard packet */
    }
    /* Else, if we've got a FuSaBuffer - perform the FuSa duplicate check using
     * the first byte of the data */
    else if (!pStorage->bFuSaBuffer && wb_xms_NonFuSaDuplicateCheck (pInternals,
                                                                     pStorage,
                                                                     pData,
                                                                     (iTargetSlot - iDistance)))
    {
        /* Do nothing - Event already generated. Discard packet */
    }
    /* Else, we passed our duplicate check so store the packet */
    else
    {
        /* Set response code to valid */
        bValid = true;

        /* Store the length */
        pStorage->pData [iTargetSlot].iLength = pMsgHeader->iPayloadLength;

        /* Store the measurement packet */
        (void) memcpy (&pStorage->pData [iTargetSlot].Data [0], &pData [0],
                       pMsgHeader->iPayloadLength);

        /* Increment the valid packet statistic */
        wb_xms_IncrementWithRollover32 (&pStorage->Stats.iValidPktCount);

        /* Add the device we received on to the list of received devices */
        pStorage->iReceivedMap |= iDeviceId;

        /* If all packets have been received, submit the buffer */
        if (++pStorage->iNumSlotsCollected == pStorage->iNumSlotsCollecting)
        {
            wb_xms_SubmitBuffer (pInternals, pStorage);
        }
    }

    /* Return success/non-success to caller */
    return bValid;
}

static void wb_xms_SubmitBuffer (adi_wil_safety_internals_t const * const pInternals,
                                 adi_wil_xms_storage_state_t * const pStorage)
{
    adi_wil_sensor_data_buffer_t const  Buffer = { .pData = pStorage->pData,
                                                   .iCount = pStorage->iNumSlotsCollecting };

    /* Check if we're currently collecting measurements */
    if (pStorage->bCollecting)
    {
        /* Notify the application that data is available to be read */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     pStorage->eEvent,
                                     &Buffer);

        /* Increment the notification count */
        wb_xms_IncrementWithRollover32 (&pStorage->Stats.iDataNotifCount);

        /* Roll the sequence number forward for all the collecting nodes */
        wb_xms_RollSequenceForward (pStorage);

        /* Shift the timestamp history forward to make space for the recently
         * submitted interval as a historical timestamp */
        (void) memmove (&pStorage->iHistoricalTimestamps [1u],
                        &pStorage->iHistoricalTimestamps [0u],
                        (sizeof (pStorage->iHistoricalTimestamps [0u]) * (ADI_WIL_TIMESTAMP_HISTORY_COUNT_MAX - 1u)));

        /* Store this interval's timestamp in the history */
        pStorage->iHistoricalTimestamps [0] = pStorage->iCurrentTimestamp;

        /* Add to the count of historical timestamps if not already full */
        if (pStorage->iHistoricalTimestampCount < ADI_WIL_TIMESTAMP_HISTORY_COUNT_MAX)
        {
            pStorage->iHistoricalTimestampCount++;
        }

        /* Transition to INACTIVE */
        pStorage->bCollecting = false;
    }
}

static void wb_xms_RollSequenceForward (adi_wil_xms_storage_state_t * const pStorage)
{
    /* Bit-field of devices we want to roll forward */
    uint64_t iDeviceMap;

    /* Index of */
    uint8_t iIndex;

    /* Initialize index in map to 0 */
    iIndex = 0u;

    /* Generate a map of:
     * 1. Device's we were collecting for, and
     * 2. Device's that we've received a START message for */
    iDeviceMap = pStorage->iCollectingMap & pStorage->iSequenceInitializedMap;

    /* Loop while we still have bits in the map */
    while (iDeviceMap != 0ULL)
    {
        /* Check if bit in bit-field is set */
        if ((iDeviceMap & (1ULL << iIndex)) != 0ULL)
        {
            /* CERT-C Precondition check on parameters */
            if ((uint8_t) UINT8_MAX <= (pStorage->iStartSequenceNumber [iIndex] + 
                                        pStorage->iCollectingNumMeasPerInt))
            {
                /* Do nothing - expect rollover */
            }

            /* If it is, roll the sequence number forward by the number of
             * packets in the interval */
            pStorage->iStartSequenceNumber [iIndex] += pStorage->iCollectingNumMeasPerInt;

            /* Erase the bit */
            iDeviceMap &= ~(1ULL << iIndex);
        }

        /* Check the next bit in the field */
        iIndex++;
    }
}

static adi_wil_safety_internals_t * wb_xms_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack)
{
    /* Return value of this method */
    adi_wil_safety_internals_t * pInternals;

    /* Initialize local variable to NULL */
    pInternals = NULL;

    /* Check pack instance before dereferencing */
    if (NULL != pPack)
    {
        /* Assign safety internals pointer to local variable */
        pInternals = pPack->pSafetyInternals;
    }

    /* Return local variable pointer to safety internals */
    return pInternals;
}

static bool wb_xms_FuSaDuplicateCheck (adi_wil_safety_internals_t const * const pInternals,
                                       adi_wil_xms_storage_state_t * const pStorage,
                                       uint32_t iTargetSlot)
{
    /* Check if this slot has already been filled */
    if (pStorage->pData [iTargetSlot].iLength != 0u)
    {
        /* If the target slot was already occupied, the current packet must
         * be a duplicate */
        wb_xms_IncrementWithRollover32 (&pStorage->Stats.iDupePktCount);

        /* Generate event indicating a duplicate packet was received */
        wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                     ADI_WIL_EVENT_XMS_DUPLICATE,
                                     NULL);
    }

    /* Return a boolean indicating if this slot was already occupied */
    return (pStorage->pData [iTargetSlot].iLength != 0u);
}

static bool wb_xms_NonFuSaDuplicateCheck (adi_wil_safety_internals_t const * const pInternals,
                                          adi_wil_xms_storage_state_t * const pStorage,
                                          uint8_t const * const pData,
                                          uint32_t iStartIndex)
{
    /* Return value of the function */
    bool rc;

    /* Initialize return value to indicate we haven't found a match */
    rc = false;

    /* Loop through all packets in the measurement interval for this device */
    for (uint8_t i = 0u; i < pStorage->iCollectingNumMeasPerInt; i++)
    {
        /* If the following clauses are met:
         * 1. The slot is already filled, and
         * 2. The first byte (Packet ID) matches our new packet
         * ... then indicate a duplicate */
        if ((pStorage->pData [iStartIndex + i].iLength != 0u) &&
            (pStorage->pData [iStartIndex + i].Data [0] == pData [0]))
        {
            /* If the target slot was already occupied, the current packet must
             * be a duplicate */
            wb_xms_IncrementWithRollover32 (&pStorage->Stats.iDupePktCount);

            /* Generate event indicating a duplicate packet was received */
            wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                         ADI_WIL_EVENT_XMS_DUPLICATE,
                                         NULL);

            /* Set the return value to true indicating a duplicate was found
             * and exit loop */
            rc = true;
            break;
        }
    }

    /* Return a boolean indicating if this packet was already buffered */
    return rc;
}

static uint32_t wb_xms_GetTargetSlot (uint64_t iCollectingMap,
                                      uint64_t iDeviceId,
                                      uint8_t iNumMeasPerInt,
                                      uint8_t iDistance)
{
    /* Temporary copy of the collecting map to find device index within */
    uint64_t iTempCollectingMap;

    /* The set bit's position within the collecting map */
    uint8_t iPositionWithinMap;

    /* Return value of this function */
    uint32_t iTargetSlot;

    /* Init temporary collecting map to 0 */
    iTempCollectingMap = 0ULL;

    /* Init the position within the map to bit 0 */
    iPositionWithinMap = 0u;

    /* Store a copy of the collecting map for identifying the device
     * position within it. Mask off any bits higher than our device */
    if (iDeviceId > 0ULL)
    {
        iTempCollectingMap = iCollectingMap & (iDeviceId - 1ULL);
    }

    /* Count the number of set bits in the temp collecting map while
     * clearing each set bit to get the number of leading 1's before our device */
    while (iTempCollectingMap != 0ULL)
    {
        iTempCollectingMap = iTempCollectingMap & (iTempCollectingMap - 1u);
        iPositionWithinMap++;

        if (iPositionWithinMap >= ((uint8_t) ADI_WIL_MAX_DEVICES - 1u))
        {
            break;
        }
    }

    /* Return the device's position within the map multiplied by the number
     * of measurements per interval and offset it by the distance from the
     * START */
    iTargetSlot = (uint32_t) iPositionWithinMap * (uint32_t) iNumMeasPerInt;

    return wb_xms_GetTargetSlotFromPosition (iTargetSlot, iDistance);
}

static adi_wil_xms_storage_state_t * wb_xms_GetStorageState (adi_wil_safety_internals_t * const pInternals,
                                                             adi_wil_xms_type_t eType)
{
    /* Return value of this function */
    adi_wil_xms_storage_state_t * pStorage;

    /* First check for BMS type... */
    if (eType == ADI_WIL_XMS_BMS)
    {
        pStorage = &pInternals->XMS.BmsStorageState;
    }
    /* ... else, check for PMS type */
    else if (eType == ADI_WIL_XMS_PMS)
    {
        pStorage = &pInternals->XMS.PmsStorageState;
    }
    /* ... else, it must be an EMS packet */
    else
    {
        pStorage = &pInternals->XMS.EmsStorageState;
    }

    /* Return value to caller */
    return pStorage;
}

static bool wb_xms_GetDeviceIndex (uint8_t * pDeviceIndex,
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
    else if (iSourceDeviceId == WB_SCL_MANAGER_0_DEVICE_ID)
    {
        *pDeviceIndex = 62u;
    }
    /* ... else if it's manager 1, use bit 63 */
    else if (iSourceDeviceId == WB_SCL_MANAGER_1_DEVICE_ID)
    {
        *pDeviceIndex = 63u;
    }
    /* ...else, it's an invalid 8-bit device id */
    else
    {
        bValid = false;
    }

    /* Return validity to caller */
    return bValid;
}

static void wb_xms_IncrementWithRollover32 (uint32_t * pValue)
{
    /* CERT-C Precondition check on parameters */
    if (*pValue == (uint32_t) UINT32_MAX)
    {
        /* Do nothing - expect rollover */
    }

    /* Increment the value pointed to */
    (*pValue)++;
}

static uint8_t wb_xms_GetDistanceFromStartSequence (uint8_t iStartSequenceNumber,
                                                    uint8_t iReceivedSequenceNumber)
{
    /* Return value of this function */
    uint8_t iDistance;

    /* CERT-C Precondition check on parameters */
    if (iReceivedSequenceNumber >= iStartSequenceNumber)
    {
        /* Do nothing - expect rollover */
    }

    iDistance = (iReceivedSequenceNumber - iStartSequenceNumber);

    /* Return value to caller */
    return iDistance;
}

static uint32_t wb_xms_GetTargetSlotFromPosition (uint32_t iTargetPosition,
                                                  uint8_t iDistance)
{
    if ((UINT32_MAX - iTargetPosition) < iDistance)
    {
        /* CERT-C - Pre-condition check on addition */
    }

    return iTargetPosition + iDistance;
}

static bool wb_xms_CheckSameTimestamp (uint32_t iTimestamp,
                                       uint32_t iReference)
{
    /* Stores the distance between the two reference points */
    uint32_t iDistance;

    /* Calculate the absolute difference between the two values */
    iDistance = (iTimestamp >= iReference) ? (iTimestamp - iReference) :
                                             (iReference - iTimestamp);

    /* Check the timestamps are within two ticks of eachother.
     * Return true if distance is between -2 and +2 ticks shifted to work with
     * 24-bit value stored in upper 24-bits of uint32_t */
    return ((iDistance <= ((uint32_t) ADI_WIL_XMS_TIMESTAMP_TOLERANCE << 8u)) ||
            (iDistance > ((uint32_t) UINT32_MAX - ((uint32_t) ADI_WIL_XMS_TIMESTAMP_TOLERANCE << 8u))));
}

/******************************************************************************
 * @file     adi_wil_xms_internals.h
 *
 * @brief    WIL FE XMSStorage internals structure definitions
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_XMS_INTERNALS_H
#define ADI_WIL_XMS_INTERNALS_H

#include "adi_wil_types.h"
#include "adi_wil_sensor_data_buffer.h"
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

/** Number of previous timestamps to track for interval identification */
#define ADI_WIL_TIMESTAMP_HISTORY_COUNT_MAX (2u)

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

 /**
  * @brief   Data packet statistics
  */
typedef struct {
    uint32_t iValidPktCount;                            /*!< Total number of accepted packets */
    uint32_t iLatePktCount;                             /*!< Number of late packets received */
    uint32_t iDupePktCount;                             /*!< Number of duplicate packets received */
    uint32_t iDataNotifCount;                           /*!< Number of data notifications generated */
} adi_wil_fusa_pkt_statistics_t;

/**
 * @brief   XMSStorage state structure
 */
typedef struct
{
    uint64_t iDeviceMap;                                 /*!< List of devices to allocate for on each measurement interval */
    uint64_t iCollectingMap;                             /*!< List of devices allocated for the current measurement interval */
    uint64_t iReceivedMap;                               /*!< List of devices we have received at least one packet for in the current measurement interval */
    uint64_t iSequenceInitializedMap;                    /*!< Mask to detect a devices's first measurement */
    adi_wil_sensor_data_t * pData;                       /*!< EMS node data buffer pointer */
    adi_wil_fusa_pkt_statistics_t Stats;                 /*!< BMS data statistics */
    adi_wil_event_id_t eEvent;                           /*!< Buffer submission event associated with this XMS type */
    uint32_t iLastTick;                                  /*!< Tick corresponding to last VALID measurement */
    uint32_t iCurrentTimestamp;                          /*!< Current timestamp of the collection interval */
    uint32_t iHistoricalTimestamps [ADI_WIL_TIMESTAMP_HISTORY_COUNT_MAX];   /*!< Storage for historical timestamps for interval detection */
    uint16_t iNumSlotsCollecting;                        /*!< Number of slots we are currently collecting for */
    uint16_t iNumSlotsCollected;                         /*!< Number of slots we have collected so far */
    uint16_t iNumSlotsAllocated;                         /*!< Number of slots in buffer (measurements per device times number of devices). */
    uint8_t iStartSequenceNumber [ADI_WIL_MAX_DEVICES];  /*!< Array to track the sequence number corresponding to ADI_WIL_XMS_START_MEASUREMENT of each node/manager. */
    uint8_t iNumMeasPerInt;                              /*!< Number of measurements expected per measurement interval */
    uint8_t iCollectingNumMeasPerInt;                    /*!< Currently collecting number of measurements expected per measurement interval */
    uint8_t iHistoricalTimestampCount;                   /*!< Number of valid iHistoricalTimestamps elements */
    bool bFuSaBuffer;                                    /*!< Indicates if buffer is fusa (true) or non-fusa (false). */
    bool bCollecting;                                    /*!< Indicates if xms storage is in collecting (true) or inactive (false) state. */
    bool bCollectingFuSa;                                /*!< Stores the current collecting state to protect against FuSa/non-fusa combination in a buffer */
} adi_wil_xms_storage_state_t;

/**
 * @brief   XMSStorage internal variables
 */
typedef struct
{
    adi_wil_xms_storage_state_t BmsStorageState;                       /*!< BMS storage */
    adi_wil_xms_storage_state_t PmsStorageState;                       /*!< PMS storage */
    adi_wil_xms_storage_state_t EmsStorageState;                       /*!< EMS storage */
    adi_wil_sensor_data_t * pXmsBuffer;                                /*!< XMS combined data buffer */
    uint16_t iXmsDataBufferCount;                                      /*!< XMS combined data buffer count */
    bool bAllocationInitialized;                                       /*!< Boolean indicating if the storage states have been allocated buffer */
} adi_wil_xms_internals_t;

#endif //ADI_WIL_XMS_INTERNALS_H

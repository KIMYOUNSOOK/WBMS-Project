/******************************************************************************
 * @file     wb_ntf_system_status.h
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_SYSTEM_STATUS_H
#define WB_NOTIF_SYSTEM_STATUS_H

#include <stdint.h>

/**
 * @brief   wbms_notif_system_status_t
 */
struct wbms_notif_system_status_t {
    uint64_t iASN;                       /* Timestamp of packet generation. */
    /* Software version information */
    uint16_t iSWVersionMajor;            /* Major software version */
    uint16_t iSWVersionMinor;            /* Minor software version */
    /* SPIS related counters*/
    uint32_t iSpiTxQueueOFCount;         /* SPIS tx queue overflow counter */
    uint32_t iSpiRxCrcErrorCount;        /* SPIS rx CRC error counter */
    uint32_t iSpiDevXferErrorCount;      /* Number of SPI transfers failed due to SPI peripheral error */
    uint32_t iSpiSWXferErrorCount;       /* Number of SPI transfers failed due to SPI driver error */
    uint32_t iSpiAbortCount;             /* SPIS transfer abort counter */
    uint32_t iSpiTxFrameAllocErrorCount; /* SPIS Tx frame memory allocation error counter */
    uint32_t iSpiTxMsgAllocErrorCount;   /* SPIS Tx message memory allocation error counter */
    uint32_t iSpiRxFrameAllocErrorCount; /* SPIS Rx frame memory allocation error counter */
    uint32_t iSpiRxMsgAllocErrorCount;   /* SPIS Rx message memory allocation error counter */
    uint32_t iIdleFramesSinceLastMsg;    /* Number of idles frames sent since last status msg */
    /* FS related counters */
    uint16_t iFlash0OneBitEccErrCount;   /* Flash 0 Single bit ECC error count (SBE) */
    uint16_t iFlash0TwoBitEccErrCount;   /* Flash 0 Double bit ECC error count (DBE) */
    uint32_t iFlash0LastEccErrAddr;      /* Flash 0 ECC error address */
    uint16_t iFlash1OneBitEccErrCount;   /* Flash 1 Single bit ECC error count (SBE) */
    uint16_t iFlash1TwoBitEccErrCount;   /* Flash 1 Double bit ECC error count (DBE) */
    uint32_t iFlash1LastEccErrAddr;      /* Flash 1 ECC error address */
    uint16_t iFlashWriteErrCount;        /* Flash program error count */
    uint8_t  iLffsFreePercent;           /* LFFS free bytes in percentage */
    uint16_t iLffsDefragCount;           /* LFFS defragmentation count */
    uint32_t iLffsStatus;                /* LFFS error status */
};

#endif //WB_NOTIF_SYSTEM_STATUS_H

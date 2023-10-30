/******************************************************************************
 * @file     adi_wil_ui_internals.h
 *
 * @brief    WIL UI internals structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_UI_INTERNALS__H
#define ADI_WIL_UI_INTERNALS__H

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/
/**
 * @brief   UI lock state structure
 */
typedef struct {
    volatile uint32_t iTimestamp;      /*!< Ticker value when lock was acquired */
    volatile bool bAcquired;           /*!< Boolean value indicating if the lock has been acquired */
    volatile bool bFusaContext;        /*!< Tracks whether a lock was acquired from the FuSa or non-fusa context */
} adi_wil_ui_lock_state_t;

/**
 * @brief   UI Internals structure
 */
typedef struct {
    adi_wil_ui_lock_state_t LockState;      /*!< Pack lock state */
} adi_wil_ui_internals_t;

#endif //ADI_WIL_UI_INTERNALS__H

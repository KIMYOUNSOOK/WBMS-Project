/******************************************************************************
 * @file     adi_wil_hal_tmr.h
 *
 * @brief    WBMS Interface Library timer scheduler HAL.
 *
 * @details  Contains API for the ticker that is used for scheduling the SPI.
 *					  
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_HAL_TMR__H
#define ADI_WIL_HAL_TMR__H

#include "adi_wil_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Type defines
 *****************************************************************************/
/**
 * @brief   HAL callback function type
 */
typedef void (*adi_wb_hal_tmr_cb_t)(void);

/******************************************************************************
 * Function Declarations
 *****************************************************************************/
/**
 * @brief   Initialize the scheduler timer.
 *
 * @details This function initializes the underlying timer hardware that is to
 *          be used as the timer scheduler.
 *
 * @param void                  None.
 *
 * @return  adi_wil_hal_err_t   Error code of the initialization.
 */
adi_wil_hal_err_t adi_wil_hal_TmrInit(void);

/**
 * @brief   Start the timer with the alarm interval and the call back pointer.
 *
 * @details This function starts the timer. The timer should be in alarm
 *          mode with the alarm set to argument interval. The timer should
 *          be run in a continuous mode i.e. once the alarm has been triggered,
 *          the timer is to be reloaded with the same alarm interval and this
 *          process runs indefinitely. When the alarm is triggered, the call
 &          back function pointed to by pfCb parameter should be invoked.
 *
 * @param pfCb                  Call back function pointer.
 *
 * @return  adi_wil_hal_err_t   Error code of the start operation.
 */
adi_wil_hal_err_t adi_wil_hal_TmrStart(adi_wb_hal_tmr_cb_t pfCb);

/**
 * @brief   Stop the running timer.
 *
 * @details This function stops the currently running timer.
 *
 * @param void                  None.
 *
 * @return  adi_wil_hal_err_t   Error code of the stop operation.
 */
adi_wil_hal_err_t adi_wil_hal_TmrStop(void);

#ifdef __cplusplus
}
#endif
#endif // #ifndef ADI_WIL_HAL_TMR__H

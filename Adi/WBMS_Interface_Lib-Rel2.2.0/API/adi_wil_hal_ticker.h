/******************************************************************************
 * @file     adi_wil_hal_ticker.h
 *
 * @brief    WBMS Interface Library ticker HAL.
 *
 * @details  Contains API for the ticker that is used for keeping time outs.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_WIL_HAL_TICKER__H
#define ADI_WIL_HAL_TICKER__H

#include "adi_wil_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * Function Declarations
 *****************************************************************************/
/**
 * @brief   Initialize the ticker timer.
 *
 * @details This function initializes the underlying timer hardware. Perform any
 *          driver specific initialization here.
 *
 * @param void                  None.
 *
 * @return  adi_wil_hal_err_t   Error code of the initialization.
 */
adi_wil_hal_err_t adi_wil_hal_TickerInit(void);

/**
 * @brief   Start the free running timer.
 *
 * @details This function starts the timer to tick up.
 *
 * @param void                  None.
 *
 * @return  adi_wil_hal_err_t   Error code of the timer start.
 */
adi_wil_hal_err_t adi_wil_hal_TickerStart(void);

/**
 * @brief   Get the current timestamp.
 *
 * @details This function gets the current timestamp since the start running of
 *          the timer. The timestamp is to be specified in ms. The underlying
 *          function needs to convert the timer counter value to ms based on the
 *          timer input clock frequency. Note that this function does not need
 *          to handle the wrap around.
 *
 * @param void                  None.
 *
 * @return  uint32_t            Current timestamp in ms.
 */
uint32_t adi_wil_hal_TickerGetTimestamp(void);

/**
 * @brief   Stop the ticker timer.
 *
 * @details This function stops the currently running timer.
 *
 * @param void                  None.
 *
 * @return  adi_wil_hal_err_t   Error code of the timer stop operation.
 */
adi_wil_hal_err_t adi_wil_hal_TickerStop(void);

uint32_t	GetTick_1ms(void);


#ifdef __cplusplus
}
#endif
#endif // #ifndef ADI_WIL_HAL_TICKER__H

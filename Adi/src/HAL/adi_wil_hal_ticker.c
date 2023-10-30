/*******************************************************************************
 * @brief    HAL Ticker layer
 *
 * @details  Implement 32-bit free-running timer
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include "adi_wil_hal_ticker.h"
#include "adi_wil_hal.h"
#include "IfxStm.h"
#include "adi_wil_example_isr_priorities.h"
#include "adi_wil_example_debug_functions.h"

#define HAL_TICKER_TICK_MSEC   (1)            /* hal_ticker should increment every msec, see WIL documentation */
#define HAL_TICKER_STM         &MODULE_STM0   /* Which STM timer to use */

static uint32 HalTickerTicks;      /* HAL_TICKER_TICK_MSEC converted to STM timer ticks */


adi_wil_hal_err_t adi_wil_hal_TickerInit(void)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

    /* The STM timers run automatically out of reset, so no hardware configuration required at init. */

    /* For portability, use provided functions to convert 3msec to appropriate number of timer ticks and store */
    HalTickerTicks = (uint32)IfxStm_getTicksFromMilliseconds(HAL_TICKER_STM, HAL_TICKER_TICK_MSEC);

    return result;
}


adi_wil_hal_err_t adi_wil_hal_TickerStart(void)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

    /* The STM timers run automatically out of reset, so no hardware configuration required at start. */

    return result;
}


uint32_t adi_wil_hal_TickerGetTimestamp(void)
{
    /* read current value of STM (STM is 64-bit timer) and divide to convert to milliseconds */
    uint64 iNow = IfxStm_get(HAL_TICKER_STM) / (uint64)HalTickerTicks;
    return (uint32_t)iNow;
}


adi_wil_hal_err_t adi_wil_hal_TickerStop(void)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

    /* Since the STM timer may be used by other features, this timer should be left running. Nothing to do here. */

    return result;
}

/*******************************************************************************
 * @brief    HAL TMR layer
 *
 * @details  Implements 3msec period call of application callback
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include "adi_wil_hal_tmr.h"
#include "adi_wil_hal.h"
#include "IfxStm.h"
#include "adi_wil_example_isr_priorities.h"
#include "adi_wil_example_debug_functions.h"

#define HAL_TMR_PERIOD_MicroSEC 3000
#define HAL_TMR_STM           &MODULE_STM0   /* Which STM timer to use */
#define HAL_TMR_COMPARATOR    IfxStm_Comparator_1  /* which of the two available comparators to use to track elapsed time. see IfxStm_Comparator */

#if defined(TMR_DEBUG_ENABLE)
    #define TMR_DEBUG_PIN         &MODULE_P21,0    /* Define debug pin */
#endif

static uint32 HalTmrPeriodTicks;                  /* HAL_TMR_PERIOD_MSEC converted to STM timer ticks */
static adi_wb_hal_tmr_cb_t pfTmrCb = (void *)0;    /* function pointer, points back to application callback */
extern uint32_t NetworkStatusTmr;
extern bool G_SPI_0_DMA_break;
extern bool G_SPI_1_DMA_break;

IFX_INTERRUPT(HalTmrIsr, 0, ISR_PRIORITY_HAL_TMR);

/* ISR, services the hal_tmr interrupt */
void HalTmrIsr(void)
{
    /* increment the comparator value by the number of ticks to set next interrupt */
    IfxStm_increaseCompare(HAL_TMR_STM, HAL_TMR_COMPARATOR, HalTmrPeriodTicks);
    
    G_SPI_0_DMA_break = false;
    G_SPI_1_DMA_break = false;
    NetworkStatusTmr++;

#if defined(TMR_DEBUG_ENABLE)
    IfxPort_togglePin(TMR_DEBUG_PIN);
#endif

    /* call the application callback */
    if (pfTmrCb != (void *)0)
    {
        IfxCpu_enableInterrupts();
        pfTmrCb();
    }

#if defined(TMR_DEBUG_ENABLE)
    IfxPort_togglePin(TMR_DEBUG_PIN);
#endif
}


adi_wil_hal_err_t adi_wil_hal_TmrInit(void)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

#if defined(TMR_DEBUG_ENABLE)
    IfxPort_setPinModeOutput(TMR_DEBUG_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
#endif

    /* The STM timers run automatically out of reset, so no hardware configuration required at init. */

    /* For portability, use provided functions to convert 3msec to appropriate number of timer ticks and store */
    // HalTmrPeriodTicks = (uint32)IfxStm_getTicksFromMilliseconds(HAL_TMR_STM, HAL_TMR_PERIOD_MSEC);
    HalTmrPeriodTicks = (uint32)IfxStm_getTicksFromMicroseconds(HAL_TMR_STM, HAL_TMR_PERIOD_MicroSEC);

    return result;
}   


adi_wil_hal_err_t adi_wil_hal_TmrStart(adi_wb_hal_tmr_cb_t pfCb)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

    /* config and init 3msc period and interrupt */
    IfxStm_CompareConfig HalTmr;

    IfxStm_initCompareConfig(&HalTmr);           /* Initialize the configuration structure with default values   */

    HalTmr.comparator          = HAL_TMR_COMPARATOR; 
    HalTmr.comparatorInterrupt = IfxStm_ComparatorInterrupt_ir1; 
    HalTmr.compareOffset       = IfxStm_ComparatorOffset_0; 
    HalTmr.compareSize         = IfxStm_ComparatorSize_32Bits; 
    HalTmr.ticks               = HalTmrPeriodTicks;               /* Set the number of ticks after which the timer triggers an  interrupt for the first time */
    HalTmr.triggerPriority     = ISR_PRIORITY_HAL_TMR;                /* Set the priority of the interrupt */
    HalTmr.typeOfService       = IfxSrc_Tos_cpu0;                 /* Set the service provider for the interrupts */

    /* Configure the STM peripheral by passing the user configuration */
    /* IfxStm_initCompare returns true if init was successful */
    result = IfxStm_initCompare(HAL_TMR_STM, &HalTmr) ? ADI_WIL_HAL_ERR_SUCCESS : ADI_WIL_HAL_ERR_FAILURE;

    /* store the callback */
    if (result == ADI_WIL_HAL_ERR_SUCCESS)
    {
        pfTmrCb = pfCb;
    }

    return result;
}


adi_wil_hal_err_t adi_wil_hal_TmrStop(void)
{
    adi_wil_hal_err_t result = ADI_WIL_HAL_ERR_SUCCESS;

    /* to stop TMR, just disable interrupt. Since the STM timer may be used by other features, this timer should be left running. */
    IfxStm_disableComparatorInterrupt(HAL_TMR_STM, HAL_TMR_COMPARATOR);

    /* clear the callback as well */
    pfTmrCb = (void *)0;

    return result;
}

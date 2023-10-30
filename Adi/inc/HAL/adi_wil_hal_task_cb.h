/*******************************************************************************
 * @brief    HAL Task
 *
 * @details  Implement periodic call at user specified rate (intended to support calling processTask())
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#ifndef ADI_WIL_HAL_TASKCB_H
#define ADI_WIL_HAL_TASKCB_H

#include <stdint.h>
#include <stdbool.h>


bool adi_wil_hal_TaskCBStart(uint32_t iPeriodUs, void(*pfCb)(void));
void adi_wil_hal_TaskCBStop(void);


#endif  /*  ADI_WIL_HAL_TASKCB_H  */

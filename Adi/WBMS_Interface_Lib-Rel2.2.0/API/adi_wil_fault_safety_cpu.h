/******************************************************************************
 * @file     adi_wil_fault_safety_cpu.h
 *
 * @brief    WIL FE Safety CPU fault summary struct definition
 *
 * @details  Contains Safety CPU fault message structure definition
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_FAULT_SAFETY_CPU_H
#define ADI_WIL_FAULT_SAFETY_CPU_H

#include "adi_wil_types.h"
#include <stdint.h>

/**
 * @brief   Fault summary data received from Safety CPU
 */
struct adi_wil_fault_safety_cpu_t {
    adi_wil_device_t eDeviceId;                                /*!< Device that produced this fault report */
    uint8_t iLength;                                           /*!< Length of the fault message */
    uint8_t iData[ADI_WIL_MAX_SAFETY_CPU_FAULT_MESSAGE_SIZE];  /*!< Fault message received from the device */
};

#endif //ADI_WIL_FAULT_SAFETY_CPU_H

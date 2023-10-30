/******************************************************************************
 * @file    wb_wil_process_task.h
 *
 * @brief   Performs periodic processing on a given pack instance
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_PROCESS_TASK_H
#define WB_WIL_PROCESS_TASK_H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Public functions *
 *****************************************************************************/

adi_wil_err_t wb_wil_ProcessTaskAPI (adi_wil_pack_internals_t * const pInternals,
                                     uint32_t iCurrentTicks);

#ifdef __cplusplus
}
#endif
#endif //WB_WIL_PROCESS_TASK_H

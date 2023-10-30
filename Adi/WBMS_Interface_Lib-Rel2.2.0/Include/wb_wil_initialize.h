/*******************************************************************************
 * @file    wb_wil_initialize.h
 *
 * @brief   WBMS initialization source
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_WIL_INITIALIZE_H
#define WB_WIL_INITIALIZE_H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
adi_wil_err_t wb_wil_InitializeAPI (void);

adi_wil_err_t wb_wil_TerminateAPI (void);

adi_wil_err_t adi_wil_InitializePack (adi_wil_pack_t const * const pPack,
                                      adi_wil_pack_internals_t * const pInternals,
                                      adi_wil_port_t * const pManager0Port,
                                      adi_wil_port_t * const pManager1Port);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_INITIALIZE_H

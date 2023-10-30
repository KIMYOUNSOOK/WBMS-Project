/******************************************************************************
 * @file     wb_nil.h
 *
 * @brief    Internal header file for interfacing from the WIL to NIL
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_NIL__H
#define WB_NIL__H

#include "wb_packer.h"
#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"
#include "wbms_cmd_node_defs.h"
#include <stdbool.h>

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

adi_wil_err_t wb_nil_Initialize(void);

adi_wil_err_t wb_nil_Terminate(void);

adi_wil_err_t wb_nil_InitPort(adi_wil_pack_internals_t * const pInternals, adi_wil_port_t * const pPort);

adi_wil_err_t wb_nil_ClosePort(adi_wil_port_t const * const pPort);

bool wb_nil_CheckForPort(adi_wil_port_t const * const pPort);

adi_wil_err_t wb_nil_Process(adi_wil_port_t * const pPort);

void wb_nil_ProcessAllPorts(void);

void wb_nil_Login(adi_wil_port_t * const pPort, uint8_t iSessionId);

void wb_nil_Logout(adi_wil_port_t * const pPort);

adi_wil_err_t wb_nil_SubmitFrame (adi_wil_port_t * const pPort,
                                  wb_pack_element_t const * const pFrame);

#ifdef __cplusplus
}
#endif
#endif //WB_NIL__H

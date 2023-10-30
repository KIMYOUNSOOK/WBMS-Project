/*******************************************************************************
 * @file    wb_wil_error_notify.h
 *
 * @brief   Functions to handle error notifications.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#ifndef WB_WIL_ERROR_NOTIFY_H
#define WB_WIL_ERROR_NOTIFY_H

#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Public functions */
/******************************************************************************/
/**
 * @brief Handler to handle port link lost event (manager is resetting or sending erroneous frames)
 *
 * @param pInternals - pointer to pack instance internals structure
 * @param pPort - The port that generated the error
 */
void wb_wil_HandlePortDownEvent(adi_wil_pack_internals_t * const pInternals, adi_wil_port_t * const pPort);

/**
 * @brief Handler to handle the event of manager link up (manager is alive, but not connected to WIL)
 *
 * @param pInternals - pointer to pack instance internals structure
 * @param pPort - pointer to the port that generated the error
 */
void wb_wil_HandlePortAvailableEvent(adi_wil_pack_internals_t * const pInternals, adi_wil_port_t * const pPort);

#ifdef __cplusplus
}
#endif
#endif // #ifndef WB_WIL_ERROR_NOTIFY_H

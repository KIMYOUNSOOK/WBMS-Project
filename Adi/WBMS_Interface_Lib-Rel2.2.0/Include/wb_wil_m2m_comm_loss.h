/******************************************************************************
* @file    wb_wil_m2m_comm_loss.h
*
* @brief   Handle a m2m communication loss notification from a device
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_WIL_M2M_COMM_LOSS_H
#define WB_WIL_M2M_COMM_LOSS_H

#include "adi_wil_types.h"
#include "wbms_cmd_mgr_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
* Public functions
*****************************************************************************/

/**
* @brief Handle M2M communication loss notification error
*
* @param pPack      Pointer to pack instance struct
* @param eDeviceId  Device ID of the manager that generated this error notification
* @param pNotif     Pointer to notification data structure
*
*/
void wb_wil_HandleM2MCommLossNotif(adi_wil_pack_internals_t const * const pInternals, uint64_t iDeviceId, wbms_notif_m2m_comm_loss_t const * const pNotif);

#ifdef __cplusplus
}
#endif
#endif  //WB_WIL_M2M_COMM_LOSS_H

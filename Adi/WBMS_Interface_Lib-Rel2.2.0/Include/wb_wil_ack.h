/******************************************************************************
 * @file     wb_wil_ack.h
 *
 * @brief    Queue to add pending acknowledgments to be sent to the wBMS
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_WIL_ACK_QUEUE__H
#define WB_WIL_ACK_QUEUE__H

#include <stdint.h>
#include "adi_wil_port.h"

/******************************************************************************
 * Public function declarations
 *******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

adi_wil_err_t wb_wil_ack_Put (adi_wil_ack_queue_t * const pQueue,
							  uint16_t iValue,
							  uint8_t iCommandId);

adi_wil_err_t wb_wil_ack_Get (adi_wil_ack_queue_t * const pQueue,
							  uint16_t * const pValue,
							  uint8_t * const pCommandId);

#ifdef __cplusplus
}
#endif
#endif // WB_WIL_ACK_QUEUE__H

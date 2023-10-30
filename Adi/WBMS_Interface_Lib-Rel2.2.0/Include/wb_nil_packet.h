/******************************************************************************
 * @file     wb_nil_packet.h
 *
 * @brief    Application packet handling header
 *
 * @details  Contains application packet handling header definitions
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NIL_PACKET__H
#define WB_NIL_PACKET__H

#include "adi_wil_types.h"
#include "wb_packer.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

adi_wil_err_t wb_nil_packet_Process(adi_wil_port_t * const pPort, wb_pack_element_t * const pElement, uint8_t iMessageId);

adi_wil_err_t wb_nil_packet_ProcessLoggedOutPacket(adi_wil_port_t * const pPort, wb_pack_element_t * const pElement, uint8_t iMessageId);

#ifdef __cplusplus
}
#endif
#endif //WB_PACKET__H

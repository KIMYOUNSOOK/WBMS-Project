/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
******************************************************************************/

#ifndef WB_SERIALIZE_PROTOCOL__H
#define WB_SERIALIZE_PROTOCOL__H

#include "wb_packer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wbms_sfh_t wbms_sfh_t;
typedef struct wbms_sph_t wbms_sph_t;

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

void wb_pack_PacketHeader (wb_pack_element_t * packet, wbms_sph_t * obj);

void wb_pack_FrameHeader(wb_pack_element_t * const frame, wbms_sfh_t * const obj);

void wb_pack_GenerateCrc16(wb_pack_element_t * const frame, uint16_t offset, uint16_t length, uint16_t * crc);

void wb_pack_GenerateCrc32(wb_pack_element_t * const frame, uint16_t offset, uint16_t length, uint32_t * crc);

void wb_pack_Crc16(wb_pack_element_t * const frame, uint16_t * const crc);

void wb_pack_Crc32(wb_pack_element_t * const frame, uint32_t * const crc);

#ifdef __cplusplus
}
#endif
#endif //WB_SERIALIZE_FRAME_H

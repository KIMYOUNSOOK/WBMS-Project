/******************************************************************************
 * @file     wb_pack_core.h
 *
 * @brief
 *
 * @details
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_PACK_CORE_H
#define WB_PACK_CORE_H

#include "wb_packer.h"

typedef enum
{
    WB_PACK_SUCCESS,
    WB_PACK_FAILURE
} wb_pack_err_t;

#ifdef __cplusplus
extern "C" {
#endif

wb_pack_err_t wb_pack_InitElement (wb_pack_element_t * const element, wb_pack_direction_t direction, uint8_t * buffer, uint16_t offset, uint16_t size);

wb_pack_err_t wb_pack_AddSubElement(wb_pack_element_t * const child, wb_pack_element_t * const parent, const uint16_t offset, const uint16_t size);

void wb_pack_SeekWithinElement(wb_pack_element_t * const element, const uint16_t position);

wb_pack_err_t wb_packer_Init (wb_packer_t * const packer, wb_pack_direction_t direction);

wb_pack_err_t wb_packer_Config (wb_packer_t * const packer, uint8_t * const origin, uint16_t initial_offset);

#ifdef __cplusplus
}
#endif
#endif //WB_PACK_CORE_H

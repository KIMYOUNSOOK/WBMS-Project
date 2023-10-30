/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_PACKER_H
#define WB_PACKER_H

#include <stdint.h>

typedef enum
{
    WB_PACK_READ,  /* Read from buffer and write to property */
    WB_PACK_WRITE  /* Read from property and write to buffer */
} wb_pack_direction_t;

struct wb_packer; /* Forward declaration */

typedef struct wb_packer wb_packer_t;

struct wb_packer
{
    uint8_t * buf;
    uint16_t index;
    wb_pack_direction_t direction;
};

typedef struct
{
    uint8_t * origin;
    wb_packer_t packer;
    uint16_t initial_offset;    /* Initial offset inside parent from origin on creation */
    uint16_t offset;            /* Offset in the frame of this element */
    uint16_t data;              /* Bytes in use by this element */
    uint16_t size;              /* Max size that data + header cannot exceed */
} wb_pack_element_t;

#ifdef __cplusplus
extern "C" {
#endif

void wb_packer_uint8(wb_packer_t * const packer, uint8_t * const prop);

void wb_packer_uint16(wb_packer_t * const packer, uint16_t * const prop);

void wb_packer_uint32(wb_packer_t * const packer, uint32_t * const prop);

void wb_packer_uint64(wb_packer_t * const packer, uint64_t * const prop);

void wb_packer_block(wb_packer_t * const packer, uint8_t * prop, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif // WB_PACKER_H

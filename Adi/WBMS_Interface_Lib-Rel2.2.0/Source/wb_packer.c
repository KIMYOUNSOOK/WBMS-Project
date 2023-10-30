/******************************************************************************
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#include <stdint.h>
#include <string.h>
#include "wb_packer.h"

#define WB_PACK_INDEX_MAX           ((uint16_t) UINT16_MAX)
#define WB_PACK_UINT16_INDEX_MAX    (WB_PACK_INDEX_MAX - 1u)
#define WB_PACK_UINT32_INDEX_MAX    (WB_PACK_INDEX_MAX - 3u)
#define WB_PACK_UINT64_INDEX_MAX    (WB_PACK_INDEX_MAX - 7u)
#define WB_PACK_UINT16_WIDTH        (2u)
#define WB_PACK_UINT32_WIDTH        (4u)
#define WB_PACK_UINT64_WIDTH        (8u)

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

void wb_packer_uint8 (wb_packer_t * const packer,
                      uint8_t * const prop)
{
    /* Validate input parameters */
    if ((packer == (void *) 0) || (prop == (void *) 0))
    {
        /* Do nothing - invalid input parameters */
    }
    else
    {
        /* Validate index will not overflow */
        if ((packer->index < WB_PACK_INDEX_MAX))
        {
            /* Switch logic based on direction (read or write) */
            if (packer->direction == WB_PACK_READ)
            {
                /* Read a uint8_t from the buffer to the variable */
                *prop = packer->buf [packer->index];
            }
            else
            {
                /* Write a uint8_t from the variable to the buffer */
                packer->buf [packer->index] = *prop;
            }

            /* Increment index by bytes read or written */
            packer->index++;
        }
    }
}

void wb_packer_uint16 (wb_packer_t * const packer,
                       uint16_t * const prop)
{
    /* Validate input parameters */
    if ((packer == (void *) 0) || (prop == (void *) 0))
    {
        /* Do nothing - invalid input parameters */
    }
    else
    {
        /* Validate index will not overflow */
        if (packer->index < WB_PACK_UINT16_INDEX_MAX)
        {
            /* Switch logic based on direction (read or write) */
            if (packer->direction == WB_PACK_READ)
            {
                /* Read a uint16_t from the buffer to the variable */
                *prop = ((((packer->buf [packer->index + 0u] & 0xFFFFu) << 8u) & 0xFF00u) |
                         (((packer->buf [packer->index + 1u] & 0xFFFFu) << 0u) & 0x00FFu)) & 0xFFFFu;
            }
            else
            {
                /* Write a uint16_t from the variable to the buffer */
                packer->buf [packer->index + 0u] = (uint8_t) (((*prop & 0xFF00u) >> 8u) & 0xFFu);
                packer->buf [packer->index + 1u] = (uint8_t) (((*prop & 0x00FFu) >> 0u) & 0xFFu);
            }

            /* Increment index by bytes read or written */
            packer->index += WB_PACK_UINT16_WIDTH;
        }
    }
}

void wb_packer_uint32 (wb_packer_t * const packer,
                       uint32_t * const prop)
{
    /* Validate input parameters */
    if ((packer == (void *) 0) || (prop == (void *) 0))
    {
        /* Do nothing - invalid input parameters */
    }
    else
    {
        /* Validate index will not overflow */
        if (packer->index < WB_PACK_UINT32_INDEX_MAX)
        {
            /* Switch logic based on direction (read or write) */
            if (packer->direction == WB_PACK_READ)
            {
                /* Read a uint32_t from the buffer to the variable */
                *prop = ((((packer->buf [packer->index + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                         (((packer->buf [packer->index + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                         (((packer->buf [packer->index + 2u] & 0xFFFFFFFFu) << 8u)  & 0x0000FF00u) |
                         (((packer->buf [packer->index + 3u] & 0xFFFFFFFFu) << 0u)  & 0x000000FFu)) & 0xFFFFFFFFu;
            }
            else
            {
                /* Write a uint32_t from the variable to the buffer */
                packer->buf [packer->index + 0u] = (uint8_t) (((*prop & 0xFF000000u) >> 24u) & 0xFFu);
                packer->buf [packer->index + 1u] = (uint8_t) (((*prop & 0x00FF0000u) >> 16u) & 0xFFu);
                packer->buf [packer->index + 2u] = (uint8_t) (((*prop & 0x0000FF00u) >> 8u)  & 0xFFu);
                packer->buf [packer->index + 3u] = (uint8_t) (((*prop & 0x000000FFu) >> 0u)  & 0xFFu);
            }

            /* Increment index by bytes read or written */
            packer->index += WB_PACK_UINT32_WIDTH;
        }
    }
}

void wb_packer_uint64 (wb_packer_t * const packer,
                       uint64_t * const prop)
{
    /* Validate input parameters */
    if ((packer == (void *) 0) || (prop == (void *) 0))
    {
        /* Do nothing - invalid input parameters */
    }
    else
    {
        /* Validate index will not overflow */
        if (packer->index < WB_PACK_UINT64_INDEX_MAX)
        {
            /* Switch logic based on direction (read or write) */
            if (packer->direction == WB_PACK_READ)
            {
                /* Read a uint64_t from the buffer to the variable */
                *prop = ((((packer->buf [packer->index + 0u] & 0xFFFFFFFFFFFFFFFFu) << 56u) & 0xFF00000000000000u) |
                         (((packer->buf [packer->index + 1u] & 0xFFFFFFFFFFFFFFFFu) << 48u) & 0x00FF000000000000u) |
                         (((packer->buf [packer->index + 2u] & 0xFFFFFFFFFFFFFFFFu) << 40u) & 0x0000FF0000000000u) |
                         (((packer->buf [packer->index + 3u] & 0xFFFFFFFFFFFFFFFFu) << 32u) & 0x000000FF00000000u) |
                         (((packer->buf [packer->index + 4u] & 0xFFFFFFFFFFFFFFFFu) << 24u) & 0x00000000FF000000u) |
                         (((packer->buf [packer->index + 5u] & 0xFFFFFFFFFFFFFFFFu) << 16u) & 0x0000000000FF0000u) |
                         (((packer->buf [packer->index + 6u] & 0xFFFFFFFFFFFFFFFFu) << 8u)  & 0x000000000000FF00u) |
                         (((packer->buf [packer->index + 7u] & 0xFFFFFFFFFFFFFFFFu) << 0u)  & 0x00000000000000FFu)) & 0xFFFFFFFFFFFFFFFFu;
            }
            else
            {
                /* Store byte 0 to buffer */
                packer->buf [packer->index + 0u] = (uint8_t) (((*prop & 0xFF00000000000000u) >> 56u) & 0xFFu);

                /* Store byte 1 to buffer */
                packer->buf [packer->index + 1u] = (uint8_t) (((*prop & 0x00FF000000000000u) >> 48u) & 0xFFu);

                /* Store byte 2 to buffer */
                packer->buf [packer->index + 2u] = (uint8_t) (((*prop & 0x0000FF0000000000u) >> 40u) & 0xFFu);

                /* Store byte 3 to buffer */
                packer->buf [packer->index + 3u] = (uint8_t) (((*prop & 0x000000FF00000000u) >> 32u) & 0xFFu);

                /* Store byte 4 to buffer */
                packer->buf [packer->index + 4u] = (uint8_t) (((*prop & 0x00000000FF000000u) >> 24u) & 0xFFu);

                /* Store byte 5 to buffer */
                packer->buf [packer->index + 5u] = (uint8_t) (((*prop & 0x0000000000FF0000u) >> 16u) & 0xFFu);

                /* Store byte 6 to buffer */
                packer->buf [packer->index + 6u] = (uint8_t) (((*prop & 0x000000000000FF00u) >> 8u) & 0xFFu);

                /* Store byte 7 to buffer */
                packer->buf [packer->index + 7u] = (uint8_t) (((*prop & 0x00000000000000FFu) >> 0u) & 0xFFu);
            }

            /* Increment index by bytes read or written */
            packer->index += WB_PACK_UINT64_WIDTH;
        }
    }
}

void wb_packer_block (wb_packer_t * const packer,
                      uint8_t * prop,
                      uint16_t length)
{
    /* Validate input parameters */
    if ((packer == (void *) 0) || (prop == (void *) 0))
    {
        /* Do nothing - invalid input parameters */
    }
    else
    {
        /* Validate index will not overflow */
        if ((packer->index + length) < WB_PACK_INDEX_MAX)
        {
            /* Switch logic based on direction (read or write) */
            if (packer->direction == WB_PACK_READ)
            {
                /* Read a array from the buffer */
                (void) memcpy (prop, &packer->buf [packer->index], length);
            }
            else
            {
                /* Write a array to the buffer */
                (void) memcpy (&packer->buf [packer->index], prop, length);
            }

            /* Increment index by bytes read or written */
            packer->index += length;
        }
    }
}

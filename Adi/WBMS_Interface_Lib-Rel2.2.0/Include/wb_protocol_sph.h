/******************************************************************************
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef WB_PROTOCOL_SPH__H
#define WB_PROTOCOL_SPH__H

#include <stdint.h>

/**
 * @brief   wbms_sph_t
 * @desc    SPI packet header
 */
struct wbms_sph_t {
    uint8_t    iMessageId;				/* Message type identification */
    uint8_t    iMessagePayloadLength;   /* Length of payload area of this message */
};

#endif //WB_PROTOCOL_SPH__H

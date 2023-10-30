/******************************************************************************
 * @file     wb_rsp_gpio_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_GPIO_GET_H
#define WB_RSP_GPIO_GET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_resp_get_gpio_t
 */
struct wbms_cmd_resp_get_gpio_t {
    uint16_t iToken;                        /* Token to match request with response */
    uint8_t iValue;                         /* Current value of GPIO */
    uint8_t rc;                             /* RC indicating if the above values are valid */
};

#endif //WB_RSP_GPIO_GET_H

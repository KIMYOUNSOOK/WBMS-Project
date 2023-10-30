/******************************************************************************
 * @file     wb_req_gpio_get.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GPIO_GET_H
#define WB_REQ_GPIO_GET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_get_gpio_pin_t
 */
struct wbms_cmd_req_get_gpio_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iGPIOId;               /* GPIO to retrieve the value of */
};

#endif //WB_REQ_GPIO_GET_H

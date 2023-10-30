/******************************************************************************
 * @file     wb_req_gpio_set.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_GPIO_SET_H
#define WB_REQ_GPIO_SET_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_set_gpio_pin_t
 */
struct wbms_cmd_req_set_gpio_t {
    uint16_t iToken;                        /* Token to match request with response */
    uint8_t iGPIOId;                        /* GPIO to set the value of */
    uint8_t iValue;                         /* Value of GPIO to set */
};

#endif //WB_REQ_GPIO_SET_H

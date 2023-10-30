/******************************************************************************
 * @file     wb_req_dmh_apply.h
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_DMH_APPLY_H
#define WB_REQ_DMH_APPLY_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_dmh_apply_t
 */
struct wbms_cmd_req_dmh_apply_t {
    uint16_t iToken;               /*!< Token to match request with response */
    uint8_t iType;                 /*!< Topology type to apply */
};

#endif //WB_REQ_DMH_APPLY_H

/******************************************************************************
 * @file     wb_req_otap_data.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_OTAP_DATA_H
#define WB_REQ_OTAP_DATA_H

#include <stdint.h>

/**
 * @brief   wbms_cmd_req_otap_data_t
 */
struct wbms_cmd_req_otap_data_t  {
    uint16_t iToken;
    uint16_t iBlockNumber;
};

#endif //WB_REQ_OTAP_DATA_H

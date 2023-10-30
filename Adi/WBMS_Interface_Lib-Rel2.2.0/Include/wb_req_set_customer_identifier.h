/******************************************************************************
 * @file     wb_req_set_customer_identifier.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_REQ_SET_CUSTOMER_IDENTIFIER_H
#define WB_REQ_SET_CUSTOMER_IDENTIFIER_H

#include <stdint.h>

#define WB_MAX_CUSTOMER_ID_SIZE 31u

/**
 * @brief   wbms_cmd_req_set_customer_identifier_t
 */
struct wbms_cmd_req_set_customer_identifier_t {
    uint16_t iToken;               /* Token to match request with response */
    uint8_t iCustomerIdentifierId; /* ID of customer identifier data */
    uint8_t iLength;               /* Length of the data that follows this header */
};

#endif //WB_REQ_SET_CUSTOMER_IDENTIFIER_H

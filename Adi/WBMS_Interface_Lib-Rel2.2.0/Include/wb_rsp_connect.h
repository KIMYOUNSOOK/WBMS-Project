/******************************************************************************
 * @file     wb_rsp_connect.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RSP_CONNECT_H
#define WB_RSP_CONNECT_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_resp_connect_t
 */
struct wbms_cmd_resp_connect_t {
    uint16_t iToken;                                    /* Token to match response with request */
    uint8_t iSessionId;                                 /* Key to use for all further SPI communication */
    uint8_t iProtocolVersion;                           /* MSB: Dual or Single Manager configuration. MSB-1: DMH Enabled. 6 LSBs: SPI API version */
    uint8_t iManagerNumber;                             /* 1 or 2 to indicate if the manager is primary or secondary */
    uint8_t iMode;                                      /* Current operational mode of the network */
    uint8_t iNodeCount;                                 /* Number of nodes in the ACL */
    uint8_t iNodeStatusMask[WBMS_NODE_BITMAP_SIZE];     /* Bitmap of node status - 0 disconnected 1 connected */
    uint8_t iMaxNodeCount;                              /* Maximum number of nodes in the system as specified in the configuration file */
    uint8_t iMaxBMSPacketsPerNode;                      /* Maximum number of BMS packets per node in one measurement cycle */
    uint8_t iMaxPMSPackets;                             /* Maximum number of PMS packets produced by this network in one measurement cycle  */
	uint8_t iPMSEnabledManagers;                        /* Number of managers that generate PMS data */
    uint8_t iMaxEnvironmentalPackets;                   /* Maximum number of environmental monitoring packets produced by this network in one measurement cycle  */
    uint32_t iConfigurationHash;                        /* Hash of all configuration settings for verification */
    uint8_t rc;                                         /* RC indicating if manager is operating as expected, move to limited mode or poll for status */
};

#endif //WB_RSP_CONNECT_H

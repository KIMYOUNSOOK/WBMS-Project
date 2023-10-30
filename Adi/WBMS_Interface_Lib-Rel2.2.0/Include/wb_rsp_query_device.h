/******************************************************************************
 * @file     wb_rsp_query_device.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_RESP_QUERY_DEVICE_H
#define WB_RESP_QUERY_DEVICE_H

#include <stdint.h>
#include "wbms_cmd_mgr_defs.h"

/**
 * @brief   wbms_cmd_resp_query_device_t
 */
struct wbms_cmd_resp_query_device_t {
    uint8_t MAC[WBMS_MAC_ADDR_LEN];             /* MAC of this manager */
    uint8_t PeerMAC[WBMS_MAC_ADDR_LEN];         /* MAC of manager connected via M2M link */
    uint8_t isStandalone;                       /* From NetworkConfig structure/SystemConfig file) */
    uint8_t iMaxNodeCount;                      /* Maximum node count from configuration file */
    uint8_t iMaxBMSPacketsPerNode;              /* Maximum number of BMS packets per node in one measurement cycle */
    uint8_t iMaxPMSPackets;                     /* Maximum number of PMS packets produced by this network in one measurement cycle  */
    uint8_t iPMSEnabledManagers;                /* Number of managers that generate PMS data */
    uint8_t iMaxEnvironmentalPackets;           /* Maximum number of environmental monitoring packets produced by this network in one measurement cycle  */
    uint32_t iConfigurationHash;                /* Hash of all configuration settings for verification */
    uint8_t iEncryptionEnabledFlag;             /* Flag indicating if encrpytion is enabled on this port */
    uint8_t Nonce[WBMS_SPI_NONCE_SIZE];         /* Buffer containing nonce if encrpytion is enabled on this port */
    uint16_t iVersionMajor;                     /* Main processor SW major revision */
    uint16_t iVersionMinor;                     /* Main processor SW minor revision */
    uint16_t iVersionPatch;                     /* Main processor SW patch revision */
    uint16_t iVersionBuild;                     /* Main processor SW build revision */
    uint32_t iReserved0;                        /* Reserved field */
    uint32_t iReserved1;                        /* Reserved field */
    uint8_t rc;                                 /* RC indicating if the above values are valid or invalid */
};

#endif //WB_RESP_QUERY_DEVICE_H

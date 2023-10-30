/******************************************************************************
 * @file     wb_ntf_packet_received.h
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_NOTIF_PACKET_RECEIVED_H
#define WB_NOTIF_PACKET_RECEIVED_H

#include <stdint.h>

/**
 * @brief   wbms_notif_packet_received_t
 */
struct wbms_notif_packet_received_t {
    uint8_t iDeviceId;                         /* Index in ACL of device. */
    uint64_t iASN;                             /* Timestamp of packet generation. */
    uint32_t iSequenceNum;                     /* Sequence number of the packet. */
    uint16_t iLength;                          /* Application payload length. */
    uint16_t iLatency;                         /* Latency between packet generation and send to MngrApp (asn) in milliseconds.*/
    uint8_t iPort;                             /* Port id. */
    uint8_t iTwoHopFlag;                       /* Whether the node which transmitted the packet is two hop away. */
    int8_t  iRSSI;                             /* Received packet rssi */
    uint8_t iChannel;                          /* Received channel number */
};

#endif //WB_NOTIF_PACKET_RECEIVED_H

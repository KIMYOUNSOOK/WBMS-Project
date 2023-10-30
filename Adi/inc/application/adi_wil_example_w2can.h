/*******************************************************************************
* @file      adi_wil_example_w2can.h
*
* @brief     Wireless to CAN conversion header
*
* @details   Contains global signal enumeration and exported API prototypes 
*
* Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#ifndef __W2CAN__
#define __W2CAN__

#include <stdint.h>
#include "adi_wil_health_report.h"
/*******************************************************************************/
/*                      Local MACRO Declarations                               */
/*******************************************************************************/
/* 8 Bytes datafield*/
#define ADI_W2CAN_CANMESSAGE_SIZE               8U

/* BMS Signals Offset and length (in bytes) */
#define ADI_W2CAN_SIG_ASN_OFFSET                0U
#define ADI_W2CAN_SIG_ASN_LENGTH                8U

#define ADI_W2CAN_SIG_TIMESTAMP_OFFSET          1U
#define ADI_W2CAN_SIG_TIMESTAMP_LENGTH          3U

#define ADI_W2CAN_SIG_LATENCY_OFFSET            0U
#define ADI_W2CAN_SIG_LATENCY_LENGTH            2U

#define ADI_W2CAN_SIG_SEQ_OFFSET                2U
#define ADI_W2CAN_SIG_SEQ_LENGTH                4U

#define ADI_W2CAN_SIG_CHANNEL_OFFSET            6U
#define ADI_W2CAN_SIG_CHANNEL_LENGTH            1U

#define ADI_W2CAN_SIG_RSSI_OFFSET               7U
#define ADI_W2CAN_SIG_RSSI_LENGTH               1U

#define ADI_W2CAN_SIG_C1V_OFFSET                0U
#define ADI_W2CAN_SIG_C1V_LENGTH                2U

#define ADI_W2CAN_SIG_C2V_OFFSET                2U
#define ADI_W2CAN_SIG_C2V_LENGTH                2U

#define ADI_W2CAN_SIG_C3V_OFFSET                4U
#define ADI_W2CAN_SIG_C3V_LENGTH                2U

#define ADI_W2CAN_SIG_CVARPEC_OFFSET            6U
#define ADI_W2CAN_SIG_CVARPEC_LENGTH            2U

#define ADI_W2CAN_SIG_C4V_OFFSET                0U
#define ADI_W2CAN_SIG_C4V_LENGTH                2U

#define ADI_W2CAN_SIG_C5V_OFFSET                2U
#define ADI_W2CAN_SIG_C5V_LENGTH                2U

#define ADI_W2CAN_SIG_C6V_OFFSET                4U
#define ADI_W2CAN_SIG_C6V_LENGTH                2U

#define ADI_W2CAN_SIG_CVBRPEC_OFFSET            6U
#define ADI_W2CAN_SIG_CVBRPEC_LENGTH            2U

#define ADI_W2CAN_SIG_C7V_OFFSET                0U
#define ADI_W2CAN_SIG_C7V_LENGTH                2U

#define ADI_W2CAN_SIG_C8V_OFFSET                2U
#define ADI_W2CAN_SIG_C8V_LENGTH                2U

#define ADI_W2CAN_SIG_C9V_OFFSET                4U
#define ADI_W2CAN_SIG_C9V_LENGTH                2U

#define ADI_W2CAN_SIG_CVCRPEC_OFFSET            6U
#define ADI_W2CAN_SIG_CVCRPEC_LENGTH            2U

#define ADI_W2CAN_SIG_C10V_OFFSET               0U
#define ADI_W2CAN_SIG_C10V_LENGTH               2U

#define ADI_W2CAN_SIG_C11V_OFFSET               2U
#define ADI_W2CAN_SIG_C11V_LENGTH               2U

#define ADI_W2CAN_SIG_C12V_OFFSET               4U
#define ADI_W2CAN_SIG_C12V_LENGTH               2U

#define ADI_W2CAN_SIG_CVDRPEC_OFFSET            6U
#define ADI_W2CAN_SIG_CVDRPEC_LENGTH            2U

/* PMS Signals Offset and length (in bytes) */
#define ADI_W2CAN_SIG_CURR_I1_OFFSET            0U
#define ADI_W2CAN_SIG_CURR_I1_LENGTH            2U

#define ADI_W2CAN_SIG_CURR_I2_OFFSET            2U
#define ADI_W2CAN_SIG_CURR_I2_LENGTH            2U

#define ADI_W2CAN_SIG_VBAT_OFFSET               4U
#define ADI_W2CAN_SIG_VBAT_LENGTH               2U

#define ADI_W2CAN_SIG_I1_I2_VBAT_PEC_OFFSET     6U
#define ADI_W2CAN_SIG_I1_I2_VBAT_PEC_LENGTH     2U

#define ADI_W2CAN_SIG_AUX_PM1_OFFSET            0U
#define ADI_W2CAN_SIG_AUX_PM1_LENGTH            2U

#define ADI_W2CAN_SIG_HS1_PM1_OFFSET            2U
#define ADI_W2CAN_SIG_HS1_PM1_LENGTH            2U

#define ADI_W2CAN_SIG_HS2_PM1_OFFSET            4U
#define ADI_W2CAN_SIG_HS2_PM1_LENGTH            2U

#define ADI_W2CAN_SIG_AUX_HS1_HS2_PEC_OFFSET    6U
#define ADI_W2CAN_SIG_AUX_HS1_HS2_PEC_LENGTH    2U

/* EMS Signals Offset and length (in bytes) */
#define ADI_W2CAN_EMS_SIG_C1V_OFFSET            0U
#define ADI_W2CAN_EMS_SIG_C1V_LENGTH            2U

#define ADI_W2CAN_EMS_SIG_C2V_OFFSET            2U
#define ADI_W2CAN_EMS_SIG_C2V_LENGTH            2U

#define ADI_W2CAN_EMS_SIG_C3V_OFFSET            4U
#define ADI_W2CAN_EMS_SIG_C3V_LENGTH            2U

#define ADI_W2CAN_EMS_SIG_CVARPEC_OFFSET        6U
#define ADI_W2CAN_EMS_SIG_CVARPEC_LENGTH        2U

#define ADI_W2CAN_SIG_CD1V_OFFSET               0U
#define ADI_W2CAN_EMS_SIG_CD1V_LENGTH           2U

#define ADI_W2CAN_SIG_CD2V_OFFSET               2U
#define ADI_W2CAN_EMS_SIG_CD2V_LENGTH           2U

#define ADI_W2CAN_SIG_CD3V_OFFSET               4U
#define ADI_W2CAN_EMS_SIG_CD3V_LENGTH           2U

#define ADI_W2CAN_SIG_CSAPEC_OFFSET             6U
#define ADI_W2CAN_EMS_SIG_CSAPEC_LENGTH         2U


/* Health Report Signals Offset and length (in bytes)*/
#define ADI_HR01_TOTAL_PKTS_GEN_OFFSET          0U
#define ADI_HR01_TOTAL_PKTS_GEN_LENGTH          2U

#define ADI_HR01_PKTS_NACKED_OFFSET             2U
#define ADI_HR01_PKTS_NACKED_LENGTH             2U

#define ADI_HR01_NETMICFAILS_OFFSET             4U
#define ADI_HR01_NETMICFAILS_LENGTH             2U

#define ADI_HR01_MACMICFAILS_OFFSET             6U
#define ADI_HR01_MACMICFAILS_LENGTH             2U

#define ADI_HR02_JOINATTEMPTS_OFFSET            0U
#define ADI_HR02_JOINATTEMPTS_LENGTH            2U

#define ADI_HR02_RESET_CNTR_OFFSET              2U
#define ADI_HR02_RESET_CNTR_LENGTH              2U

#define ADI_HR02_TWOHOP_CNTR_OFFSET             4U
#define ADI_HR02_TWOHOP_CNTR_LENGTH             2U

#define ADI_HR02_SYSDBGINFO_OFFSET              6U
#define ADI_HR02_SYSDBGINFO_LENGTH              2U

#define ADI_HR03_PATH_STABILITY_OFFSET          0U
#define ADI_HR03_PATH_STABILITY_LENGTH          8U

#define ADI_HR04_PATH_STABILITY_OFFSET          0U
#define ADI_HR04_PATH_STABILITY_LENGTH          8U

#define ADI_HR05_PATH_STABILITY_OFFSET          0U
#define ADI_HR05_PATH_STABILITY_LENGTH          8U

#define ADI_HR06_PATH_STABILITY_OFFSET          0U
#define ADI_HR06_PATH_STABILITY_LENGTH          8U

#define ADI_HR07_SIGNAL_RSSI_OFFSET             0U
#define ADI_HR07_SIGNAL_RSSI_LENGTH             8U

#define ADI_HR08_SIGNAL_RSSI_OFFSET             0U
#define ADI_HR08_SIGNAL_RSSI_LENGTH             8U

#define ADI_HR09_SIGNAL_RSSI_OFFSET             0U
#define ADI_HR09_SIGNAL_RSSI_LENGTH             8U

#define ADI_HR10_SIGNAL_RSSI_OFFSET             0U
#define ADI_HR10_SIGNAL_RSSI_LENGTH             2U

#define ADI_HR10_QUEUE_FAILURE_OFFSET           2U
#define ADI_HR10_QUEUE_FAILURE_LENGTH           1U

#define ADI_HR10_ADC_TEMPERATURE_OFFSET         3U
#define ADI_HR10_ADC_TEMPERATURE_LENGTH         1U

#define ADI_HR11_BCKGNDRSSI_OFFSET              0U
#define ADI_HR11_BCKGNDRSSI_LENGTH              8U

#define ADI_HR12_BCKGNDRSSI_OFFSET              0U
#define ADI_HR12_BCKGNDRSSI_LENGTH              8U

#define ADI_HR13_BCKGNDRSSI_OFFSET              0U
#define ADI_HR13_BCKGNDRSSI_LENGTH              8U

#define ADI_HR14_BCKGNDRSSI_OFFSET              0U
#define ADI_HR14_BCKGNDRSSI_LENGTH              8U

#define ADI_HR15_BCKGNDRSSI_OFFSET              0U
#define ADI_HR15_BCKGNDRSSI_LENGTH              8U

#define ADI_HR16_BCKGNDRSSI_OFFSET              0U
#define ADI_HR16_BCKGNDRSSI_LENGTH              8U

#define ADI_HR17_BCKGNDRSSI_OFFSET              0U
#define ADI_HR17_BCKGNDRSSI_LENGTH              8U

#define ADI_HR18_BCKGNDRSSI_OFFSET              0U
#define ADI_HR18_BCKGNDRSSI_LENGTH              8U

#define ADI_HR19_BCKGNDRSSI_OFFSET              0U
#define ADI_HR19_BCKGNDRSSI_LENGTH              8U

#define ADI_HR20_BCKGNDRSSI_OFFSET              0U
#define ADI_HR20_BCKGNDRSSI_LENGTH              3U

#define ADI_HR21_RESET_STATUS_OFFSET            0U
#define ADI_HR21_RESET_STATUS_LENGTH            1U

#define ADI_HR21_RESET_CNTR_OFFSET              1U
#define ADI_HR21_RESET_CNTR_LENGTH              2U

#define ADI_HR21_FREE_SPACE_OFFSET              3U
#define ADI_HR21_FREE_SPACE_LENGTH              1U

/* Network MetaData Signals Offset and length (in bytes) */
#define ADI_MD01_ASN_OFFSET                     0U
#define ADI_MD01_ASN_LENGTH                     8U

#define ADI_MD02_LATENCY_OFFSET                 0U
#define ADI_MD02_LATENCY_LENGTH                 2U

#define ADI_MD02_SEQNO_OFFSET                   2U
#define ADI_MD02_SEQNO_LENGTH                   4U

#define ADI_MD02_CHANNEL_OFFSET                 6U
#define ADI_MD02_CHANNEL_LENGTH                 1U

#define ADI_MD02_RSSI_OFFSET                    7U
#define ADI_MD02_RSSI_LENGTH                    1U

#define ADI_MD03_TWOHOP_OFFSET                  0U
#define ADI_MD03_TWOHOP_LENGTH                  1U

#define ADI_MD03_PORTID_OFFSET                  1U
#define ADI_MD03_PORTID_LENGTH                  1U

#define ADI_MD03_SRCID_OFFSET                   2U
#define ADI_MD03_SRCID_LENGTH                   1U

#define ADI_BMS_PACKET_PKTID_OFFSET             0U
#define ADI_BMS_PACKET_PKTID_LENGTH             1U

#define ADI_BMS_PACKET_PKTTIMESTAMP_OFFSET      1U
#define ADI_BMS_PACKET_PKTTIMESTAMP_LENGTH      3U

#define ADI_BMS_PACKET_PKTCRC_OFFSET            4U
#define ADI_BMS_PACKET_PKTCRC_LENGTH            2U

#define ADI_MD05_ASN_OFFSET                     0U
#define ADI_MD05_ASN_LENGTH                     8U

#define ADI_MD06_LATENCY_OFFSET                 0U
#define ADI_MD06_LATENCY_LENGTH                 2U

#define ADI_MD06_SEQNO_OFFSET                   2U
#define ADI_MD06_SEQNO_LENGTH                   4U

#define ADI_MD06_CHANNEL_OFFSET                 6U
#define ADI_MD06_CHANNEL_LENGTH                 1U

#define ADI_MD06_RSSI_OFFSET                    7U
#define ADI_MD06_RSSI_LENGTH                    1U

#define ADI_MD07_TWOHOP_OFFSET                  0U
#define ADI_MD07_TWOHOP_LENGTH                  1U

#define ADI_MD07_PORTID_OFFSET                  1U
#define ADI_MD07_PORTID_LENGTH                  1U

#define ADI_MD07_SRCID_OFFSET                   2U
#define ADI_MD07_SRCID_LENGTH                   1U

/* Max number of CAN messages of each type per node*/
#define ADI_W2CAN_MAXIMUM_WIRELESS_NODES        12U     /* 12 Nodes */
#define ADI_W2CAN_CANMESSAGES_PER_NODE          11U     /* 11 CAN Messages for each node (5 BMS Messages and 6 Metadata Messages) */
#define ADI_W2CAN_HRCANMESSAGES_PER_NODE        21U     /* 21 Health Report CAN Messages for each node */
#define ADI_W2CAN_EMS_CAN_MESSAGES              20U     /* 20 EMS CAN messages resulted from EMS script primary manager */

/* Node 1 Message IDs */
#define ADI_W2CAN_G2B_N1_CELL_GRPA              0x103
#define ADI_W2CAN_G2B_N1_CELL_GRPB              0x104
#define ADI_W2CAN_G2B_N1_CELL_GRPC              0x105
#define ADI_W2CAN_G2B_N1_CELL_GRPD              0x106

/* Node 2 Message IDs */
#define ADI_W2CAN_G2B_N2_CELL_GRPA              0x107
#define ADI_W2CAN_G2B_N2_CELL_GRPB              0x108
#define ADI_W2CAN_G2B_N2_CELL_GRPC              0x109
#define ADI_W2CAN_G2B_N2_CELL_GRPD              0x10A

/* Node 3 Message IDs */
#define ADI_W2CAN_G2B_N3_CELL_GRPA              0x10B
#define ADI_W2CAN_G2B_N3_CELL_GRPB              0x10C
#define ADI_W2CAN_G2B_N3_CELL_GRPC              0x10D
#define ADI_W2CAN_G2B_N3_CELL_GRPD              0x10E

/* Node 4 Message IDs */
#define ADI_W2CAN_G2B_N4_CELL_GRPA              0x10F
#define ADI_W2CAN_G2B_N4_CELL_GRPB              0x110
#define ADI_W2CAN_G2B_N4_CELL_GRPC              0x111
#define ADI_W2CAN_G2B_N4_CELL_GRPD              0x112

/* Node 5 Message IDs */
#define ADI_W2CAN_G2B_N5_CELL_GRPA              0x113
#define ADI_W2CAN_G2B_N5_CELL_GRPB              0x114
#define ADI_W2CAN_G2B_N5_CELL_GRPC              0x115
#define ADI_W2CAN_G2B_N5_CELL_GRPD              0x116

/* Node 6 Message IDs */
#define ADI_W2CAN_G2B_N6_CELL_GRPA              0x117
#define ADI_W2CAN_G2B_N6_CELL_GRPB              0x118
#define ADI_W2CAN_G2B_N6_CELL_GRPC              0x119
#define ADI_W2CAN_G2B_N6_CELL_GRPD              0x11A

/* Node 7 Message IDs */
#define ADI_W2CAN_G2B_N7_CELL_GRPA              0x11B
#define ADI_W2CAN_G2B_N7_CELL_GRPB              0x11C
#define ADI_W2CAN_G2B_N7_CELL_GRPC              0x11D
#define ADI_W2CAN_G2B_N7_CELL_GRPD              0x11E

/* Node 8 Message IDs */
#define ADI_W2CAN_G2B_N8_CELL_GRPA              0x11F
#define ADI_W2CAN_G2B_N8_CELL_GRPB              0x120
#define ADI_W2CAN_G2B_N8_CELL_GRPC              0x121
#define ADI_W2CAN_G2B_N8_CELL_GRPD              0x122

/* Node 9 Message IDs */
#define ADI_W2CAN_G2B_N9_CELL_GRPA              0x123
#define ADI_W2CAN_G2B_N9_CELL_GRPB              0x124
#define ADI_W2CAN_G2B_N9_CELL_GRPC              0x125
#define ADI_W2CAN_G2B_N9_CELL_GRPD              0x126

/* Node 10 Message IDs */
#define ADI_W2CAN_G2B_N10_CELL_GRPA             0x127
#define ADI_W2CAN_G2B_N10_CELL_GRPB             0x128
#define ADI_W2CAN_G2B_N10_CELL_GRPC             0x129
#define ADI_W2CAN_G2B_N10_CELL_GRPD             0x12A

/* Node 11 Message IDs */
#define ADI_W2CAN_G2B_N11_CELL_GRPA             0x12B
#define ADI_W2CAN_G2B_N11_CELL_GRPB             0x12C
#define ADI_W2CAN_G2B_N11_CELL_GRPC             0x12D
#define ADI_W2CAN_G2B_N11_CELL_GRPD             0x12E

/* Node 12 Message IDs */
#define ADI_W2CAN_G2B_N12_CELL_GRPA             0x12F
#define ADI_W2CAN_G2B_N12_CELL_GRPB             0x130
#define ADI_W2CAN_G2B_N12_CELL_GRPC             0x131
#define ADI_W2CAN_G2B_N12_CELL_GRPD             0x132

/* PMS data I1, I2, VBAT and AUX CAN IDs */
#define ADI_W2CAN_G2B_PMS_I1_I2_VBAT            0x164
#define ADI_W2CAN_G2B_PMS_AUX                   0x163

/* EMS CAN Messages */
#define ADI_W2CAN_G2B_G2B_EMS1_CELL_GRP_A       0x291
#define ADI_W2CAN_G2B_G2B_EMS1_CELL_DIAG_GRP_A  0x292

/* Health Report CAN Message  IDs */
#define ADI_W2CAN_C2B_HR01_N01                  0x2B0
#define ADI_W2CAN_C2B_HR02_N01                  0x2C8
#define ADI_W2CAN_C2B_HR03_N01                  0x2E0
#define ADI_W2CAN_C2B_HR04_N01                  0x2F8
#define ADI_W2CAN_C2B_HR05_N01                  0x310
#define ADI_W2CAN_C2B_HR06_N01                  0x328
#define ADI_W2CAN_C2B_HR07_N01                  0x340
#define ADI_W2CAN_C2B_HR08_N01                  0x358
#define ADI_W2CAN_C2B_HR09_N01                  0x370
#define ADI_W2CAN_C2B_HR10_N01                  0x388
#define ADI_W2CAN_C2B_HR11_N01                  0x3A0
#define ADI_W2CAN_C2B_HR12_N01                  0x3B8
#define ADI_W2CAN_C2B_HR13_N01                  0x3D0
#define ADI_W2CAN_C2B_HR14_N01                  0x3E8
#define ADI_W2CAN_C2B_HR15_N01                  0x400
#define ADI_W2CAN_C2B_HR16_N01                  0x418
#define ADI_W2CAN_C2B_HR17_N01                  0x430
#define ADI_W2CAN_C2B_HR18_N01                  0x448
#define ADI_W2CAN_C2B_HR19_N01                  0x460
#define ADI_W2CAN_C2B_HR20_N01                  0x478
#define ADI_W2CAN_C2B_HR21_N01                  0x490

#define ADI_W2CAN_C2B_HR01_N02                  0x2B1
#define ADI_W2CAN_C2B_HR02_N02                  0x2C9
#define ADI_W2CAN_C2B_HR03_N02                  0x2E1
#define ADI_W2CAN_C2B_HR04_N02                  0x2F9
#define ADI_W2CAN_C2B_HR05_N02                  0x311
#define ADI_W2CAN_C2B_HR06_N02                  0x329
#define ADI_W2CAN_C2B_HR07_N02                  0x341
#define ADI_W2CAN_C2B_HR08_N02                  0x359
#define ADI_W2CAN_C2B_HR09_N02                  0x371
#define ADI_W2CAN_C2B_HR10_N02                  0x389
#define ADI_W2CAN_C2B_HR11_N02                  0x3A1
#define ADI_W2CAN_C2B_HR12_N02                  0x3B9
#define ADI_W2CAN_C2B_HR13_N02                  0x3D1
#define ADI_W2CAN_C2B_HR14_N02                  0x3E9
#define ADI_W2CAN_C2B_HR15_N02                  0x401
#define ADI_W2CAN_C2B_HR16_N02                  0x419
#define ADI_W2CAN_C2B_HR17_N02                  0x431
#define ADI_W2CAN_C2B_HR18_N02                  0x449
#define ADI_W2CAN_C2B_HR19_N02                  0x461
#define ADI_W2CAN_C2B_HR20_N02                  0x479
#define ADI_W2CAN_C2B_HR21_N02                  0x491

#define ADI_W2CAN_C2B_HR01_N03                  0x2B2
#define ADI_W2CAN_C2B_HR02_N03                  0x2CA
#define ADI_W2CAN_C2B_HR03_N03                  0x2E2
#define ADI_W2CAN_C2B_HR04_N03                  0x2FA
#define ADI_W2CAN_C2B_HR05_N03                  0x312
#define ADI_W2CAN_C2B_HR06_N03                  0x32A
#define ADI_W2CAN_C2B_HR07_N03                  0x342
#define ADI_W2CAN_C2B_HR08_N03                  0x35A
#define ADI_W2CAN_C2B_HR09_N03                  0x372
#define ADI_W2CAN_C2B_HR10_N03                  0x38A
#define ADI_W2CAN_C2B_HR11_N03                  0x3A2
#define ADI_W2CAN_C2B_HR12_N03                  0x3BA
#define ADI_W2CAN_C2B_HR13_N03                  0x3D2
#define ADI_W2CAN_C2B_HR14_N03                  0x3EA
#define ADI_W2CAN_C2B_HR15_N03                  0x402
#define ADI_W2CAN_C2B_HR16_N03                  0x41A
#define ADI_W2CAN_C2B_HR17_N03                  0x432
#define ADI_W2CAN_C2B_HR18_N03                  0x44A
#define ADI_W2CAN_C2B_HR19_N03                  0x462
#define ADI_W2CAN_C2B_HR20_N03                  0x47A
#define ADI_W2CAN_C2B_HR21_N03                  0x492

#define ADI_W2CAN_C2B_HR01_N04                  0x2B3
#define ADI_W2CAN_C2B_HR02_N04                  0x2CB
#define ADI_W2CAN_C2B_HR03_N04                  0x2E3
#define ADI_W2CAN_C2B_HR04_N04                  0x2FB
#define ADI_W2CAN_C2B_HR05_N04                  0x313
#define ADI_W2CAN_C2B_HR06_N04                  0x32B
#define ADI_W2CAN_C2B_HR07_N04                  0x343
#define ADI_W2CAN_C2B_HR08_N04                  0x35B
#define ADI_W2CAN_C2B_HR09_N04                  0x373
#define ADI_W2CAN_C2B_HR10_N04                  0x38B
#define ADI_W2CAN_C2B_HR11_N04                  0x3A3
#define ADI_W2CAN_C2B_HR12_N04                  0x3BB
#define ADI_W2CAN_C2B_HR13_N04                  0x3D3
#define ADI_W2CAN_C2B_HR14_N04                  0x3EB
#define ADI_W2CAN_C2B_HR15_N04                  0x403
#define ADI_W2CAN_C2B_HR16_N04                  0x41B
#define ADI_W2CAN_C2B_HR17_N04                  0x433
#define ADI_W2CAN_C2B_HR18_N04                  0x44B
#define ADI_W2CAN_C2B_HR19_N04                  0x463
#define ADI_W2CAN_C2B_HR20_N04                  0x47B
#define ADI_W2CAN_C2B_HR21_N04                  0x493

#define ADI_W2CAN_C2B_HR01_N05                  0x2B4
#define ADI_W2CAN_C2B_HR02_N05                  0x2CC
#define ADI_W2CAN_C2B_HR03_N05                  0x2E4
#define ADI_W2CAN_C2B_HR04_N05                  0x2FC
#define ADI_W2CAN_C2B_HR05_N05                  0x314
#define ADI_W2CAN_C2B_HR06_N05                  0x32C
#define ADI_W2CAN_C2B_HR07_N05                  0x344
#define ADI_W2CAN_C2B_HR08_N05                  0x35C
#define ADI_W2CAN_C2B_HR09_N05                  0x374
#define ADI_W2CAN_C2B_HR10_N05                  0x38C
#define ADI_W2CAN_C2B_HR11_N05                  0x3A4
#define ADI_W2CAN_C2B_HR12_N05                  0x3BC
#define ADI_W2CAN_C2B_HR13_N05                  0x3D4
#define ADI_W2CAN_C2B_HR14_N05                  0x3EC
#define ADI_W2CAN_C2B_HR15_N05                  0x404
#define ADI_W2CAN_C2B_HR16_N05                  0x41C
#define ADI_W2CAN_C2B_HR17_N05                  0x434
#define ADI_W2CAN_C2B_HR18_N05                  0x44C
#define ADI_W2CAN_C2B_HR19_N05                  0x464
#define ADI_W2CAN_C2B_HR20_N05                  0x47C
#define ADI_W2CAN_C2B_HR21_N05                  0x494

#define ADI_W2CAN_C2B_HR01_N06                  0x2B5
#define ADI_W2CAN_C2B_HR02_N06                  0x2CD
#define ADI_W2CAN_C2B_HR03_N06                  0x2E5
#define ADI_W2CAN_C2B_HR04_N06                  0x2FD
#define ADI_W2CAN_C2B_HR05_N06                  0x315
#define ADI_W2CAN_C2B_HR06_N06                  0x32D
#define ADI_W2CAN_C2B_HR07_N06                  0x345
#define ADI_W2CAN_C2B_HR08_N06                  0x35D
#define ADI_W2CAN_C2B_HR09_N06                  0x375
#define ADI_W2CAN_C2B_HR10_N06                  0x38D
#define ADI_W2CAN_C2B_HR11_N06                  0x3A5
#define ADI_W2CAN_C2B_HR12_N06                  0x3BD
#define ADI_W2CAN_C2B_HR13_N06                  0x3D5
#define ADI_W2CAN_C2B_HR14_N06                  0x3ED
#define ADI_W2CAN_C2B_HR15_N06                  0x405
#define ADI_W2CAN_C2B_HR16_N06                  0x41D
#define ADI_W2CAN_C2B_HR17_N06                  0x435
#define ADI_W2CAN_C2B_HR18_N06                  0x44D
#define ADI_W2CAN_C2B_HR19_N06                  0x465
#define ADI_W2CAN_C2B_HR20_N06                  0x47D
#define ADI_W2CAN_C2B_HR21_N06                  0x495

#define ADI_W2CAN_C2B_HR01_N07                  0x2B6
#define ADI_W2CAN_C2B_HR02_N07                  0x2CE
#define ADI_W2CAN_C2B_HR03_N07                  0x2E6
#define ADI_W2CAN_C2B_HR04_N07                  0x2FE
#define ADI_W2CAN_C2B_HR05_N07                  0x316
#define ADI_W2CAN_C2B_HR06_N07                  0x32E
#define ADI_W2CAN_C2B_HR07_N07                  0x346
#define ADI_W2CAN_C2B_HR08_N07                  0x35E
#define ADI_W2CAN_C2B_HR09_N07                  0x376
#define ADI_W2CAN_C2B_HR10_N07                  0x38E
#define ADI_W2CAN_C2B_HR11_N07                  0x3A6
#define ADI_W2CAN_C2B_HR12_N07                  0x3BE
#define ADI_W2CAN_C2B_HR13_N07                  0x3D6
#define ADI_W2CAN_C2B_HR14_N07                  0x3EE
#define ADI_W2CAN_C2B_HR15_N07                  0x406
#define ADI_W2CAN_C2B_HR16_N07                  0x41E
#define ADI_W2CAN_C2B_HR17_N07                  0x436
#define ADI_W2CAN_C2B_HR18_N07                  0x44E
#define ADI_W2CAN_C2B_HR19_N07                  0x466
#define ADI_W2CAN_C2B_HR20_N07                  0x47E
#define ADI_W2CAN_C2B_HR21_N07                  0x496

#define ADI_W2CAN_C2B_HR01_N08                  0x2B7
#define ADI_W2CAN_C2B_HR02_N08                  0x2CF
#define ADI_W2CAN_C2B_HR03_N08                  0x2E7
#define ADI_W2CAN_C2B_HR04_N08                  0x2FF
#define ADI_W2CAN_C2B_HR05_N08                  0x317
#define ADI_W2CAN_C2B_HR06_N08                  0x32F
#define ADI_W2CAN_C2B_HR07_N08                  0x347
#define ADI_W2CAN_C2B_HR08_N08                  0x35F
#define ADI_W2CAN_C2B_HR09_N08                  0x377
#define ADI_W2CAN_C2B_HR10_N08                  0x38F
#define ADI_W2CAN_C2B_HR11_N08                  0x3A7
#define ADI_W2CAN_C2B_HR12_N08                  0x3BF
#define ADI_W2CAN_C2B_HR13_N08                  0x3D7
#define ADI_W2CAN_C2B_HR14_N08                  0x3EF
#define ADI_W2CAN_C2B_HR15_N08                  0x407
#define ADI_W2CAN_C2B_HR16_N08                  0x41F
#define ADI_W2CAN_C2B_HR17_N08                  0x437
#define ADI_W2CAN_C2B_HR18_N08                  0x44F
#define ADI_W2CAN_C2B_HR19_N08                  0x467
#define ADI_W2CAN_C2B_HR20_N08                  0x47F
#define ADI_W2CAN_C2B_HR21_N08                  0x497

#define ADI_W2CAN_C2B_HR01_N09                  0x2B8
#define ADI_W2CAN_C2B_HR02_N09                  0x2D0
#define ADI_W2CAN_C2B_HR03_N09                  0x2E8
#define ADI_W2CAN_C2B_HR04_N09                  0x300
#define ADI_W2CAN_C2B_HR05_N09                  0x318
#define ADI_W2CAN_C2B_HR06_N09                  0x330
#define ADI_W2CAN_C2B_HR07_N09                  0x348
#define ADI_W2CAN_C2B_HR08_N09                  0x360
#define ADI_W2CAN_C2B_HR09_N09                  0x378
#define ADI_W2CAN_C2B_HR10_N09                  0x390
#define ADI_W2CAN_C2B_HR11_N09                  0x3A8
#define ADI_W2CAN_C2B_HR12_N09                  0x3C0
#define ADI_W2CAN_C2B_HR13_N09                  0x3D8
#define ADI_W2CAN_C2B_HR14_N09                  0x3F0
#define ADI_W2CAN_C2B_HR15_N09                  0x408
#define ADI_W2CAN_C2B_HR16_N09                  0x420
#define ADI_W2CAN_C2B_HR17_N09                  0x438
#define ADI_W2CAN_C2B_HR18_N09                  0x450
#define ADI_W2CAN_C2B_HR19_N09                  0x468
#define ADI_W2CAN_C2B_HR20_N09                  0x480
#define ADI_W2CAN_C2B_HR21_N09                  0x498

#define ADI_W2CAN_C2B_HR01_N10                  0x2B9
#define ADI_W2CAN_C2B_HR02_N10                  0x2D1
#define ADI_W2CAN_C2B_HR03_N10                  0x2E9
#define ADI_W2CAN_C2B_HR04_N10                  0x301
#define ADI_W2CAN_C2B_HR05_N10                  0x319
#define ADI_W2CAN_C2B_HR06_N10                  0x331
#define ADI_W2CAN_C2B_HR07_N10                  0x349
#define ADI_W2CAN_C2B_HR08_N10                  0x361
#define ADI_W2CAN_C2B_HR09_N10                  0x379
#define ADI_W2CAN_C2B_HR10_N10                  0x391
#define ADI_W2CAN_C2B_HR11_N10                  0x3A9
#define ADI_W2CAN_C2B_HR12_N10                  0x3C1
#define ADI_W2CAN_C2B_HR13_N10                  0x3D9
#define ADI_W2CAN_C2B_HR14_N10                  0x3F1
#define ADI_W2CAN_C2B_HR15_N10                  0x409
#define ADI_W2CAN_C2B_HR16_N10                  0x421
#define ADI_W2CAN_C2B_HR17_N10                  0x439
#define ADI_W2CAN_C2B_HR18_N10                  0x451
#define ADI_W2CAN_C2B_HR19_N10                  0x469
#define ADI_W2CAN_C2B_HR20_N10                  0x481
#define ADI_W2CAN_C2B_HR21_N10                  0x499

#define ADI_W2CAN_C2B_HR01_N11                  0x2BA
#define ADI_W2CAN_C2B_HR02_N11                  0x2D2
#define ADI_W2CAN_C2B_HR03_N11                  0x2EA
#define ADI_W2CAN_C2B_HR04_N11                  0x302
#define ADI_W2CAN_C2B_HR05_N11                  0x31A
#define ADI_W2CAN_C2B_HR06_N11                  0x332
#define ADI_W2CAN_C2B_HR07_N11                  0x34A
#define ADI_W2CAN_C2B_HR08_N11                  0x362
#define ADI_W2CAN_C2B_HR09_N11                  0x37A
#define ADI_W2CAN_C2B_HR10_N11                  0x392
#define ADI_W2CAN_C2B_HR11_N11                  0x3AA
#define ADI_W2CAN_C2B_HR12_N11                  0x3C2
#define ADI_W2CAN_C2B_HR13_N11                  0x3DA
#define ADI_W2CAN_C2B_HR14_N11                  0x3F2
#define ADI_W2CAN_C2B_HR15_N11                  0x40A
#define ADI_W2CAN_C2B_HR16_N11                  0x422
#define ADI_W2CAN_C2B_HR17_N11                  0x43A
#define ADI_W2CAN_C2B_HR18_N11                  0x452
#define ADI_W2CAN_C2B_HR19_N11                  0x46A
#define ADI_W2CAN_C2B_HR20_N11                  0x482
#define ADI_W2CAN_C2B_HR21_N11                  0x49A

#define ADI_W2CAN_C2B_HR01_N12                  0x2BB
#define ADI_W2CAN_C2B_HR02_N12                  0x2D3
#define ADI_W2CAN_C2B_HR03_N12                  0x2EB
#define ADI_W2CAN_C2B_HR04_N12                  0x303
#define ADI_W2CAN_C2B_HR05_N12                  0x31B
#define ADI_W2CAN_C2B_HR06_N12                  0x333
#define ADI_W2CAN_C2B_HR07_N12                  0x34B
#define ADI_W2CAN_C2B_HR08_N12                  0x363
#define ADI_W2CAN_C2B_HR09_N12                  0x37B
#define ADI_W2CAN_C2B_HR10_N12                  0x393
#define ADI_W2CAN_C2B_HR11_N12                  0x3AB
#define ADI_W2CAN_C2B_HR12_N12                  0x3C3
#define ADI_W2CAN_C2B_HR13_N12                  0x3DB
#define ADI_W2CAN_C2B_HR14_N12                  0x3F3
#define ADI_W2CAN_C2B_HR15_N12                  0x40B
#define ADI_W2CAN_C2B_HR16_N12                  0x423
#define ADI_W2CAN_C2B_HR17_N12                  0x43B
#define ADI_W2CAN_C2B_HR18_N12                  0x453
#define ADI_W2CAN_C2B_HR19_N12                  0x46B
#define ADI_W2CAN_C2B_HR20_N12                  0x483
#define ADI_W2CAN_C2B_HR21_N12                  0x49B

/* Metadata CAN IDs */
#define ADI_W2CAN_C2B_MD01_P01                  0x16D
#define ADI_W2CAN_C2B_MD02_P01                  0x185
#define ADI_W2CAN_C2B_MD03_P01                  0x19D
#define ADI_W2CAN_G2B_N01_BMS_PACKET            0x1B5
#define ADI_W2CAN_C2B_MD05_P01                  0x1CD
#define ADI_W2CAN_C2B_MD06_P01                  0x1E5
#define ADI_W2CAN_C2B_MD07_P01                  0x1FD

#define ADI_W2CAN_C2B_MD01_P02                  0x16E
#define ADI_W2CAN_C2B_MD02_P02                  0x186
#define ADI_W2CAN_C2B_MD03_P02                  0x19E
#define ADI_W2CAN_G2B_N02_BMS_PACKET            0x1B6
#define ADI_W2CAN_C2B_MD05_P02                  0x1CE
#define ADI_W2CAN_C2B_MD06_P02                  0x1E6
#define ADI_W2CAN_C2B_MD07_P02                  0x1FE

#define ADI_W2CAN_C2B_MD01_P03                  0x16F
#define ADI_W2CAN_C2B_MD02_P03                  0x187
#define ADI_W2CAN_C2B_MD03_P03                  0x19F
#define ADI_W2CAN_G2B_N03_BMS_PACKET            0x1B7
#define ADI_W2CAN_C2B_MD05_P03                  0x1CF
#define ADI_W2CAN_C2B_MD06_P03                  0x1E7
#define ADI_W2CAN_C2B_MD07_P03                  0x1FF

#define ADI_W2CAN_C2B_MD01_P04                  0x170
#define ADI_W2CAN_C2B_MD02_P04                  0x188
#define ADI_W2CAN_C2B_MD03_P04                  0x1A0
#define ADI_W2CAN_G2B_N04_BMS_PACKET            0x1B8
#define ADI_W2CAN_C2B_MD05_P04                  0x1D0
#define ADI_W2CAN_C2B_MD06_P04                  0x1E8
#define ADI_W2CAN_C2B_MD07_P04                  0x200

#define ADI_W2CAN_C2B_MD01_P05                  0x171
#define ADI_W2CAN_C2B_MD02_P05                  0x189
#define ADI_W2CAN_C2B_MD03_P05                  0x1A1
#define ADI_W2CAN_G2B_N05_BMS_PACKET            0x1B9
#define ADI_W2CAN_C2B_MD05_P05                  0x1D1
#define ADI_W2CAN_C2B_MD06_P05                  0x1E9
#define ADI_W2CAN_C2B_MD07_P05                  0x201

#define ADI_W2CAN_C2B_MD01_P06                  0x172
#define ADI_W2CAN_C2B_MD02_P06                  0x18A
#define ADI_W2CAN_C2B_MD03_P06                  0x1A2
#define ADI_W2CAN_G2B_N06_BMS_PACKET            0x1BA
#define ADI_W2CAN_C2B_MD05_P06                  0x1D2
#define ADI_W2CAN_C2B_MD06_P06                  0x1EA
#define ADI_W2CAN_C2B_MD07_P06                  0x202

#define ADI_W2CAN_C2B_MD01_P07                  0x173
#define ADI_W2CAN_C2B_MD02_P07                  0x18B
#define ADI_W2CAN_C2B_MD03_P07                  0x1A3
#define ADI_W2CAN_G2B_N07_BMS_PACKET            0x1BB
#define ADI_W2CAN_C2B_MD05_P07                  0x1D3
#define ADI_W2CAN_C2B_MD06_P07                  0x1EB
#define ADI_W2CAN_C2B_MD07_P07                  0x203

#define ADI_W2CAN_C2B_MD01_P08                  0x174
#define ADI_W2CAN_C2B_MD02_P08                  0x18C
#define ADI_W2CAN_C2B_MD03_P08                  0x1A4
#define ADI_W2CAN_G2B_N08_BMS_PACKET            0x1BC
#define ADI_W2CAN_C2B_MD05_P08                  0x1D4
#define ADI_W2CAN_C2B_MD06_P08                  0x1EC
#define ADI_W2CAN_C2B_MD07_P08                  0x204

#define ADI_W2CAN_C2B_MD01_P09                  0x175
#define ADI_W2CAN_C2B_MD02_P09                  0x18D
#define ADI_W2CAN_C2B_MD03_P09                  0x1A5
#define ADI_W2CAN_G2B_N09_BMS_PACKET            0x1BD
#define ADI_W2CAN_C2B_MD05_P09                  0x1D5
#define ADI_W2CAN_C2B_MD06_P09                  0x1ED
#define ADI_W2CAN_C2B_MD07_P09                  0x205

#define ADI_W2CAN_C2B_MD01_P10                  0x176
#define ADI_W2CAN_C2B_MD02_P10                  0x18E
#define ADI_W2CAN_C2B_MD03_P10                  0x1A6
#define ADI_W2CAN_G2B_N10_BMS_PACKET            0x1BE
#define ADI_W2CAN_C2B_MD05_P10                  0x1D6
#define ADI_W2CAN_C2B_MD06_P10                  0x1EE
#define ADI_W2CAN_C2B_MD07_P10                  0x206

#define ADI_W2CAN_C2B_MD01_P11                  0x177
#define ADI_W2CAN_C2B_MD02_P11                  0x18F
#define ADI_W2CAN_C2B_MD03_P11                  0x1A7
#define ADI_W2CAN_G2B_N11_BMS_PACKET            0x1BF
#define ADI_W2CAN_C2B_MD05_P11                  0x1D7
#define ADI_W2CAN_C2B_MD06_P11                  0x1EF
#define ADI_W2CAN_C2B_MD07_P11                  0x207

#define ADI_W2CAN_C2B_MD01_P12                  0x178
#define ADI_W2CAN_C2B_MD02_P12                  0x190
#define ADI_W2CAN_C2B_MD03_P12                  0x1A8
#define ADI_W2CAN_G2B_N12_BMS_PACKET            0x1C0
#define ADI_W2CAN_C2B_MD05_P12                  0x1D8
#define ADI_W2CAN_C2B_MD06_P12                  0x1F0
#define ADI_W2CAN_C2B_MD07_P12                  0x208

/* Message Mapping Index for BMS Data Signal Table */
#define ADI_W2CAN_G2B_CELLGRPA_MSGINDEX         0U
#define ADI_W2CAN_G2B_CELLGRPB_MSGINDEX         1U
#define ADI_W2CAN_G2B_CELLGRPC_MSGINDEX         2U
#define ADI_W2CAN_G2B_CELLGRPD_MSGINDEX         3U

/* MetaData Msg Index */
#define ADI_W2CAN_C2B_MD01_MSGINDEX             4U
#define ADI_W2CAN_C2B_MD02_MSGINDEX             5U
#define ADI_W2CAN_C2B_MD03_MSGINDEX             6U
#define ADI_W2CAN_G2B_BMS_PACKET_MSGINDEX       7U
#define ADI_W2CAN_C2B_MD05_MSGINDEX             8U
#define ADI_W2CAN_C2B_MD06_MSGINDEX             9U
#define ADI_W2CAN_C2B_MD07_MSGINDEX             10U

/* Message Mapping Index for Health Report Signal Table */
#define ADI_W2CAN_C2B_HR01_MSGINDEX             0U
#define ADI_W2CAN_C2B_HR02_MSGINDEX             1U
#define ADI_W2CAN_C2B_HR03_MSGINDEX             2U
#define ADI_W2CAN_C2B_HR04_MSGINDEX             3U
#define ADI_W2CAN_C2B_HR05_MSGINDEX             4U
#define ADI_W2CAN_C2B_HR06_MSGINDEX             5U
#define ADI_W2CAN_C2B_HR07_MSGINDEX             6U
#define ADI_W2CAN_C2B_HR08_MSGINDEX             7U
#define ADI_W2CAN_C2B_HR09_MSGINDEX             8U
#define ADI_W2CAN_C2B_HR10_MSGINDEX             9U
#define ADI_W2CAN_C2B_HR11_MSGINDEX             10U
#define ADI_W2CAN_C2B_HR12_MSGINDEX             11U
#define ADI_W2CAN_C2B_HR13_MSGINDEX             12U
#define ADI_W2CAN_C2B_HR14_MSGINDEX             13U
#define ADI_W2CAN_C2B_HR15_MSGINDEX             14U
#define ADI_W2CAN_C2B_HR16_MSGINDEX             15U
#define ADI_W2CAN_C2B_HR17_MSGINDEX             16U
#define ADI_W2CAN_C2B_HR18_MSGINDEX             17U
#define ADI_W2CAN_C2B_HR19_MSGINDEX             18U
#define ADI_W2CAN_C2B_HR20_MSGINDEX             19U
#define ADI_W2CAN_C2B_HR21_MSGINDEX             20U

#define ADI_W2CAN_TXMSG_SLOTTIME                1U              /* 1 millisecond*/
#define ADI_W2CAN_MAXTXMSG_PERSLOT              1U              /* Number of messages to be transmitted in each slot*/
#define ADI_W2CAN_BMS_MSGPERIODICITY            100U            /* 100ms */
#define ADI_W2CAN_PMS_MSGPERIODICITY            25U             /* 25ms */
#define ADI_W2CAN_RESETVAL                      0U

/* Health Report Handler States */
#define ADI_W2CAN_HRHANDLER_IDLE                0U
#define ADI_W2CAN_HRHANDLER_SETSIGNALS          1U
#define ADI_W2CAN_HRHANDLER_TRANSMIT            2U

/* Health Report Tx status */
#define ADI_W2CAN_HEALTHREPORT_TX_INPROGRESS    0U
#define ADI_W2CAN_HEALTHREPORT_TX_FINISHED      1U
#define ADI_W2CAN_HEALTHREPORT_TX_ABORTED       2U

/* Health Report Maximum Retry Count for a Single Message*/
#define ADI_W2CAN_HEALTHREPORT_MSG_RETRY        10U

/* Health report Maximum Wait Time for Sending all messages */
#define ADI_W2CAN_HRTX_TIMEOUT                  20000           /* 20 seconds */

/* Health Report Task Cycle */
#define ADI_W2CAN_HRTASK_PERIOD                 10U             /* 10ms */

/* Health report Transmission control */
#define ADI_W2CAN_HR_TX_START                   0U
#define ADI_W2CAN_HR_TX_STOP                    1U

/* Macro definitions for PMS Buffer */
#define ADI_PMS_HEADER_LEN                      6
#define ADI_PMS_VI_OFFSET                       ADI_PMS_HEADER_LEN + 5
#define ADI_PMS_I1_OFFSET                       ADI_PMS_VI_OFFSET
#define ADI_PMS_I2_OFFSET                       ADI_PMS_I1_OFFSET + 2
#define ADI_PMS_VBAT_OFFSET                     ADI_PMS_I2_OFFSET + 2
#define ADI_PMS_I1_I2_VBAT_PEC_OFFSET           ADI_PMS_VBAT_OFFSET + 2 
#define ADI_PMS_AUX_OFFSET                      ADI_PMS_I1_I2_VBAT_PEC_OFFSET + 2
#define ADI_PMS_HS1_OFFSET                      ADI_PMS_AUX_OFFSET + 2
#define ADI_PMS_HS2_OFFSET                      ADI_PMS_HS1_OFFSET + 2
#define ADI_PMS_AUX_HS1_HS2_PEC_OFFSET          ADI_PMS_HS2_OFFSET + 2

/* Macro definitions for EMS Buffer */
#define ADI_EMS_HEADER_LEN                      6
#define ADI_EMS_PACKET_C1V_OFFSET               ADI_EMS_HEADER_LEN
#define ADI_EMS_PACKET_C2V_OFFSET               ADI_EMS_HEADER_LEN + 2
#define ADI_EMS_PACKET_C3V_OFFSET               ADI_EMS_HEADER_LEN + 4
#define ADI_EMS_PACKET_CVAR_PEC_OFFSET          ADI_EMS_HEADER_LEN + 6
#define ADI_EMS_PACKET_CD1V_OFFSET              ADI_EMS_HEADER_LEN + 8
#define ADI_EMS_PACKET_CD2V_OFFSET              ADI_EMS_HEADER_LEN + 10
#define ADI_EMS_PACKET_CD3V_OFFSET              ADI_EMS_HEADER_LEN + 12
#define ADI_EMS_PACKET_CSA_PEC_OFFSET           ADI_EMS_HEADER_LEN + 14

/* Macro definitions for node health report packet ids */
#define PACKET_ID_NODE_DEVICE                   0x00
#define PACKET_ID_NODE_AVERAGE_RSSI             0x01
#define PACKET_ID_NODE_BACKGROUND_RSSI          0x02
#define PACKET_ID_MANAGER_APPLICATION           0x10
#define PACKET_ID_MANAGER_BACKGROUND_RSSI       0x11
#define PACKET_ID_MANAGER_DEVICE                0x12
#define PACKET_ID_NODE_MANAGER_0_4_RSSI         0x14
#define PACKET_ID_NODE_MANAGER_5_9_RSSI         0x15
#define PACKET_ID_NODE_MANAGER_10_14_RSSI       0x16
#define PACKET_ID_NODE_MANAGER_15_19_RSSI       0x17
#define PACKET_ID_NODE_MANAGER_20_23_RSSI       0x18
#define PACKET_ID_NODE_APPLICATION              0x80

/*
    Macro definition for number of health reports to recieve before sending to CAN.
    Currently need to receive 3 health reports for each node - device,
    background RSSI and application reports.
*/
#define NUM_HEALTH_REPORT_PER_NODE              3

/*******************************************************************************/
/* User Defined Types                                                          */
/*******************************************************************************/
/* G2B Signals  BMS, PMS and NetworkMetaData Signals*/
enum
{
    ADI_BMS_PACKET_C1V,
    ADI_BMS_PACKET_C2V,
    ADI_BMS_PACKET_C3V,
    ADI_BMS_PACKET_CVAR_PEC,
    ADI_BMS_PACKET_C4V,
    ADI_BMS_PACKET_C5V,
    ADI_BMS_PACKET_C6V,
    ADI_BMS_PACKET_CVBR_PEC,
    ADI_BMS_PACKET_C7V,
    ADI_BMS_PACKET_C8V,
    ADI_BMS_PACKET_C9V,
    ADI_BMS_PACKET_CVCR_PEC,
    ADI_BMS_PACKET_C10V,
    ADI_BMS_PACKET_C11V,
    ADI_BMS_PACKET_C12V,
    ADI_BMS_PACKET_CVDR_PEC,

    ADI_EMS_PACKET_C1V,
    ADI_EMS_PACKET_C2V,
    ADI_EMS_PACKET_C3V,
    ADI_EMS_PACKET_CVAR_PEC,
    ADI_EMS_PACKET_CD1V,
    ADI_EMS_PACKET_CD2V,
    ADI_EMS_PACKET_CD3V,
    ADI_EMS_PACKET_CSA_PEC,

    ADI_PMS_CURR_I1,
    ADI_PMS_CURR_I2,
    ADI_PMS_VBAT,
    ADI_PMS_I1_I2_VBAT_PEC,
    ADI_PMS_AUX_PM1,
    ADI_PMS_HS1_PM1,
    ADI_PMS_HS2_PM1,
    ADI_PMS_AUX_HS1_HS2_PEC,

    ADI_MD01_ASN,
    ADI_MD02_LATENCY,
    ADI_MD02_SEQ_NO,
    ADI_MD02_CHANNEL,
    ADI_MD02_RSSI,
    ADI_MD03_TWOHOP,
    ADI_MD03_PORTID,
    ADI_MD03_SRCID,
    ADI_BMS_PACKET_PKTID,
    ADI_BMS_PACKET_PKTTIMESTAMP,
    ADI_BMS_PACKET_PKTCRC,
    ADI_MD05_ASN,
    ADI_MD06_LATENCY,
    ADI_MD06_SEQ_NO,
    ADI_MD06_CHANNEL,
    ADI_MD06_RSSI,
    ADI_MD07_TWOHOP,
    ADI_MD07_PORTID,
    ADI_MD07_SRCID,
};

/* C2B Signals : Health Report Signals */
enum
{
    ADI_HR01_TOTAL_PKTS_GEN,
    ADI_HR01_PKTS_NACKED,
    ADI_HR01_NETMICFAIL,
    ADI_HR01_MACMICFAIL,

    ADI_HR02_JOINATTEMPTS,
    ADI_HR02_RESET_CNTR,
    ADI_HR02_TWOHOP_CNTR,
    ADI_HR02_SYSDEBUG_INFO,

    ADI_HR03_PATH_STABILITY,

    ADI_HR04_PATH_STABILITY,

    ADI_HR05_PATH_STABILITY,

    ADI_HR06_PATH_STABILITY,

    ADI_HR07_SIGNAL_RSSI,

    ADI_HR08_SIGNAL_RSSI,

    ADI_HR09_SIGNAL_RSSI,

    ADI_HR10_SIGNAL_RSSI,
    ADI_HR10_QUEUE_FAILURE,
    ADI_HR10_ADC_TEMPERATURE,

    ADI_HR11_BACKGROUND_RSSI,

    ADI_HR12_BACKGROUND_RSSI,

    ADI_HR13_BACKGROUND_RSSI,

    ADI_HR14_BACKGROUND_RSSI,

    ADI_HR15_BACKGROUND_RSSI,

    ADI_HR16_BACKGROUND_RSSI,

    ADI_HR17_BACKGROUND_RSSI,

    ADI_HR18_BACKGROUND_RSSI,

    ADI_HR19_BACKGROUND_RSSI,

    ADI_HR20_BACKGROUND_RSSI,

    ADI_HR21_RESET_STATUS,
    ADI_HR21_RESET_COUNTER,
    ADI_HR21_FREE_SPACE,
};

typedef struct 
{
    uint16_t iSignalID;
    uint8_t iMsgTableIndex;
    uint8_t iStartByte;
    uint8_t iSignalLen;             /* no. of bits */
}adi_w2can_CanSignals_t;

typedef struct 
{
    uint32_t iCanFrameID;
    uint8_t iTxBuffId;
}adi_w2can_NodeMsgs_t;


#if 0

/************************************************************************************
 * adi_wil_example_w2canBmsMsgHandler
 *
 * Cyclic task to pass the BMS CAN Messages to the CAN Driver.
 *
 * @arguments : None
 *
 * @return none
 ***********************************************************************************/
void adi_wil_example_w2canBmsMsgHandler(void);

/************************************************************************************
 * adi_wil_example_w2canPmsMsgHandler
 *
 * Cyclic task to pass the PMS CAN Messages to the CAN Driver.
 *
 * @arguments : None
 *
 * @return none
 ***********************************************************************************/
void adi_wil_example_w2canPmsMsgHandler(void);

/************************************************************************************
 * adi_wil_example_w2canEmsMsgHandler
 *
 * Cyclic task to pass the EMS CAN Messages to the CAN Driver.
 *
 * @arguments : None
 *
 * @return none
 ***********************************************************************************/
void adi_wil_example_w2canEmsMsgHandler(void);

/************************************************************************************
 * adi_wil_example_w2canSetSignal
 *
 * API to set the value a particular signal.
 *
 * @arguments : Node ID : Node Index (range : 0 to Maximum Nodes) 
 *              Signal ID : Signal Index (enumerated List of the Signals)
 *              SrcBuff : pointer to Source Buffer
 *
 * @return none
 ***********************************************************************************/
void adi_wil_example_w2canSetSignal(uint8_t NodeId, uint16_t SignalID, uint8_t *SrcBuff);

/************************************************************************************
  * adi_wil_example_w2canSetPM1AuxSignal
  *
  * API to set the value a particular signal of CAN Message PMS
  *
  * @arguments : Signal ID : Signal Index (enumerated List of the Signals)
  *              SrcBuff : pointer to Source Buffer
  * @return none
***********************************************************************************/
void adi_wil_example_w2canSetPM1AuxSignal(uint16_t iSignal, uint8_t *pSrcBuff);

/************************************************************************************
  * adi_wil_example_w2canSetPM1Signal
  *
  * API to set the value a particular signal of CAN Message PMS
  *
  * @arguments : Signal ID : Signal Index (enumerated List of the Signals)
  *              SrcBuff : pointer to Source Buffer
  * @return none
***********************************************************************************/
void adi_wil_example_w2canSetPM1Signal(uint16_t iSignal, uint8_t *pSrcBuff);

/************************************************************************************
  * adi_wil_example_w2canSetEMS1Signal
  *
  * API to set the value a particular signal of CAN Message EMS
  *
  * @arguments : Signal ID : Signal Index (enumerated List of the Signals)
  *              SrcBuff : pointer to Source Buffer
  * @return none
***********************************************************************************/
void adi_wil_example_w2canSetEMS1Signal(uint16_t iSignal, uint8_t *pSrcBuff);

/************************************************************************************
  * adi_wil_example_w2canSetEMS1DiagSignal
  *
  * API to set the value a particular signal of CAN Message EMS DIAG
  *
  * @arguments : Signal ID : Signal Index (enumerated List of the Signals)
  *              SrcBuff : pointer to Source Buffer
  * @return none
***********************************************************************************/
void adi_wil_example_w2canSetEMS1DiagSignal(uint16_t iSignal, uint8_t *pSrcBuff);
    
/************************************************************************************
 * adi_wil_example_w2canHealthReportHandler
 *
 * Cyclic task to parse the Health Report and
 * to Send Health report CAN Messages to the CAN Driver 
 * (dedicated CAN Tx Buffer No. 31)
 *
 * @arguments : None
 *
 * @return none
 ***********************************************************************************/
void adi_wil_example_w2canHealthReportHandler(void);

/************************************************************************************
 * adi_w2can_NotifyHR_availability
 *
 * API to notify that 1 health report has been received by WIL
 *
 * @arguments : none
 *
 * @return none
 ***********************************************************************************/
void adi_w2can_NotifyHR_availability(void);

/************************************************************************************
  * adi_wil_example_w2canSetHRSignal
  *
  * API to set the value a Health Report Signals.
  *
  * @arguments : Node ID : Node Index (range : 0 to Maximum Nodes) 
                 Signal ID : Signal Index (enumerated List of the Signals)
  *              SrcBuff : pointer to Source Buffer
  * @return none
***********************************************************************************/
void adi_wil_example_w2canSetHRSignal(uint8_t iNodeId, uint16_t iSignal, uint8_t *pSrcBuff);
#endif

#endif

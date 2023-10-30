/*******************************************************************************
* Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_BMS_TYPES_H_
#define ADI_BMS_TYPES_H_

#include <stdint.h>
#include "adi_bms_defs.h"

typedef struct {
    uint8_t	iPacket_id;                                     // Packet Format Id
    uint8_t	iPacket_timestamp_0;                            // Packet Timestamp, byte 0
    uint8_t	iPacket_timestamp_1;                            // Packet Timestamp, byte 1
    uint8_t	iPacket_timestamp_2;                            // Packet Timestamp, byte 2
    uint8_t	iPacket_crc[ADI_BMS_SIZE_PACKET_CRC];           // Bit reversed packet CRC
} adi_bms_packetheader_t;

typedef struct {
    uint8_t	iAc1v[ADI_BMS_SIZE_AC1V];                       // Averaged Cell 1 Voltage
    uint8_t	iAc2v[ADI_BMS_SIZE_AC2V];                       // Averaged Cell 2 Voltage
    uint8_t	iAc3v[ADI_BMS_SIZE_AC3V];                       // Averaged Cell 3 Voltage
    uint8_t	iRdaca_pec[ADI_BMS_SIZE_RDACA_PEC];             // PEC for Averaged Cell Voltage Register Group A
} adi_bms_rdaca_t;

typedef struct {
    uint8_t	iAc4v[ADI_BMS_SIZE_AC4V];                       // Averaged Cell 4 Voltage
    uint8_t	iAc5v[ADI_BMS_SIZE_AC5V];                       // Averaged Cell 5 Voltage
    uint8_t	iAc6v[ADI_BMS_SIZE_AC6V];                       // Averaged Cell 6 Voltage
    uint8_t	iRdacb_pec[ADI_BMS_SIZE_RDACB_PEC];             // PEC for Averaged Cell Voltage Register Group B
} adi_bms_rdacb_t;

typedef struct {
    uint8_t	iAc7v[ADI_BMS_SIZE_AC7V];                       // Averaged Cell 7 Voltage
    uint8_t	iAc8v[ADI_BMS_SIZE_AC8V];                       // Averaged Cell 8 Voltage
    uint8_t	iAc9v[ADI_BMS_SIZE_AC9V];                       // Averaged Cell 9 Voltage
    uint8_t	iRdacc_pec[ADI_BMS_SIZE_RDACC_PEC];             // PEC for Averaged Cell Voltage Register Group C
} adi_bms_rdacc_t;

typedef struct {
    uint8_t	iAc10v[ADI_BMS_SIZE_AC10V];                     // Averaged Cell 10 Voltage
    uint8_t	iAc11v[ADI_BMS_SIZE_AC11V];                     // Averaged Cell 11 Voltage
    uint8_t	iAc12v[ADI_BMS_SIZE_AC12V];                     // Averaged Cell 12 Voltage
    uint8_t	iRdacd_pec[ADI_BMS_SIZE_RDACD_PEC];             // PEC for Averaged Cell Voltage Register Group D
} adi_bms_rdacd_t;

typedef struct {
    uint8_t	iAc13v[ADI_BMS_SIZE_AC13V];                     // Averaged Cell 13 Voltage
    uint8_t	iAc14v[ADI_BMS_SIZE_AC14V];                     // Averaged Cell 14 Voltage
    uint8_t	iAc15v[ADI_BMS_SIZE_AC15V];                     // Averaged Cell 15 Voltage
    uint8_t	iRdace_pec[ADI_BMS_SIZE_RDACE_PEC];             // PEC for Averaged Cell Voltage Register Group E
} adi_bms_rdace_t;

typedef struct {
    uint8_t	iAc16v[ADI_BMS_SIZE_AC16V];                     // Averaged Cell 16 Voltage
    uint8_t	iAc17v[ADI_BMS_SIZE_AC17V];                     // Averaged Cell 17 Voltage
    uint8_t	iAc18v[ADI_BMS_SIZE_AC18V];                     // Averaged Cell 18 Voltage
    uint8_t	iRdacf_pec[ADI_BMS_SIZE_RDACF_PEC];             // PEC for Averaged Cell Voltage Register Group F
} adi_bms_rdacf_t;

typedef struct {
    uint8_t	iG1v[ADI_BMS_SIZE_G1V];                         // Auxiliary Register 1
    uint8_t	iG2v[ADI_BMS_SIZE_G2V];                         // Auxiliary Register 2
    uint8_t	iGa11v[ADI_BMS_SIZE_GA11V];                     // Auxiliary Register A11
    uint8_t	iRdauxa_pec[ADI_BMS_SIZE_RDAUXA_PEC];           // PEC for Auxiliary Register Group A
} adi_bms_rdauxa_t;

typedef struct {
    uint8_t	iG3v[ADI_BMS_SIZE_G3V];                         // Auxiliary Register 3
    uint8_t	iG4v[ADI_BMS_SIZE_G4V];                         // Auxiliary Register 4
    uint8_t	iGper4;                                         // GPER4 = 255
    uint8_t	iGper5;                                         // GPER5 = 255
    uint8_t	iRdauxe_pec[ADI_BMS_SIZE_RDAUXE_PEC];           // PEC for Auxiliary Register Group E
} adi_bms_rdauxe_t;

typedef struct {
    uint8_t	iStcr0;                                         // RDSTATC register STCR0
    uint8_t	iStcr1;                                         // RDSTATC register STCR1
    uint8_t	iStcr2;                                         // RDSTATC register STCR2
    uint8_t	iStcr3;                                         // RDSTATC register STCR3
    uint8_t	iStcr4;                                         // RDSTATC register STCR4
    uint8_t	iStcr5;                                         // RDSTATC register STCR5
    uint8_t	iRdstatc_pec[ADI_BMS_SIZE_RDSTATC_PEC];         // PEC for Status Register Group C
} adi_bms_rdstatc_t;

typedef struct {
    uint8_t	iG10v[ADI_BMS_SIZE_G10V];                       // Auxiliary Register 10
    uint8_t	iVmv[ADI_BMS_SIZE_VMV];                         // V+ to V- Measurement
    uint8_t	iVpv[ADI_BMS_SIZE_VPV];                         // V- to V- Measurement
    uint8_t	iRdauxd_pec[ADI_BMS_SIZE_RDAUXD_PEC];           // PEC for Auxiliary Register Group D
} adi_bms_rdauxd_t;

typedef struct {
    uint8_t	iS1v[ADI_BMS_SIZE_S1V];                         // Spin Voltage 1
    uint8_t	iS2v[ADI_BMS_SIZE_S2V];                         // Spin Voltage 2
    uint8_t	iS3v[ADI_BMS_SIZE_S3V];                         // Spin Voltage 3
    uint8_t	iRdsva_pec[ADI_BMS_SIZE_RDSVA_PEC];             // PEC for Spin Voltage Register Group A
} adi_bms_rdsva_t;

typedef struct {
    uint8_t	iS4v[ADI_BMS_SIZE_S4V];                         // Spin Voltage 4
    uint8_t	iS5v[ADI_BMS_SIZE_S5V];                         // Spin Voltage 5
    uint8_t	iS6v[ADI_BMS_SIZE_S6V];                         // Spin Voltage 6
    uint8_t	iRdsvb_pec[ADI_BMS_SIZE_RDSVB_PEC];             // PEC for Spin Voltage Register Group B
} adi_bms_rdsvb_t;

typedef struct {
    uint8_t	iS7v[ADI_BMS_SIZE_S7V];                         // Spin Voltage 7
    uint8_t	iS8v[ADI_BMS_SIZE_S8V];                         // Spin Voltage 8
    uint8_t	iS9v[ADI_BMS_SIZE_S9V];                         // Spin Voltage 9
    uint8_t	iRdsvc_pec[ADI_BMS_SIZE_RDSVC_PEC];             // PEC for Spin Voltage Register Group C
} adi_bms_rdsvc_t;

typedef struct {
    uint8_t	iS10v[ADI_BMS_SIZE_S10V];                       // Spin Voltage 10
    uint8_t	iS11v[ADI_BMS_SIZE_S11V];                       // Spin Voltage 11
    uint8_t	iS12v[ADI_BMS_SIZE_S12V];                       // Spin Voltage 12
    uint8_t	iRdsvd_pec[ADI_BMS_SIZE_RDSVD_PEC];             // PEC for Spin Voltage Register Group D
} adi_bms_rdsvd_t;

typedef struct {
    uint8_t	iS13v[ADI_BMS_SIZE_S13V];                       // Spin Voltage 13
    uint8_t	iS14v[ADI_BMS_SIZE_S14V];                       // Spin Voltage 14
    uint8_t	iS15v[ADI_BMS_SIZE_S15V];                       // Spin Voltage 15
    uint8_t	iRdsve_pec[ADI_BMS_SIZE_RDSVE_PEC];             // PEC for Spin Voltage Register Group E
} adi_bms_rdsve_t;

typedef struct {
    uint8_t	iS16v[ADI_BMS_SIZE_S16V];                       // Spin Voltage 16
    uint8_t	iS17v[ADI_BMS_SIZE_S17V];                       // Spin Voltage 17
    uint8_t	iS18v[ADI_BMS_SIZE_S18V];                       // Spin Voltage 18
    uint8_t	iRdsvf_pec[ADI_BMS_SIZE_RDSVF_PEC];             // PEC for Spin Voltage Register Group F
} adi_bms_rdsvf_t;

typedef struct {
    uint8_t	iCfgar0;                                        // Config Group A, register 0
    uint8_t	iCfgar1;                                        // Config Group A, register 1
    uint8_t	iCfgar2;                                        // Config Group A, register 2
    uint8_t	iCfgar3;                                        // Config Group A, register 3
    uint8_t	iCfgar4;                                        // Config Group A, register 4
    uint8_t	iCfgar5;                                        // Config Group A, register 5
    uint8_t	iRdcfga_pec[ADI_BMS_SIZE_RDCFGA_PEC];           // PEC for Cell Config Register Group A
} adi_bms_rdcfga_t;

typedef struct {
    uint8_t	iCfgbr0;                                        // Config Group B, register 0
    uint8_t	iCfgbr1;                                        // Config Group B, register 1
    uint8_t	iCfgbr2;                                        // Config Group B, register 2
    uint8_t	iCfgbr3;                                        // Config Group B, register 3
    uint8_t	iCfgbr4;                                        // Config Group B, register 4
    uint8_t	iCfgbr5;                                        // Config Group B, register 5
    uint8_t	iRdcfgb_pec[ADI_BMS_SIZE_RDCFGB_PEC];           // PEC for Cell Config Register Group B
} adi_bms_rdcfgb_t;

typedef struct {
    uint8_t	iVref2[ADI_BMS_SIZE_VREF2];                     // Second Reference Voltage
    uint8_t	iItmp[ADI_BMS_SIZE_ITMP];                       // Internal Die Temperature
    uint8_t	iStar4;                                         // Reserved
    uint8_t	iStar5;                                         // Reserved
    uint8_t	iRdstata_pec[ADI_BMS_SIZE_RDSTATA_PEC];         // PEC for Status Register Group A
} adi_bms_rdstata_t;

typedef struct {
    uint8_t	iVd[ADI_BMS_SIZE_VD];                           // Digital Power Supply Voltage
    uint8_t	iVa[ADI_BMS_SIZE_VA];                           // Analog Power Supply Voltage
    uint8_t	iVres[ADI_BMS_SIZE_VRES];                       // Voltage Across Resistor
    uint8_t	iRdstatb_pec[ADI_BMS_SIZE_RDSTATB_PEC];         // PEC for Status Register Group B
} adi_bms_rdstatb_t;

typedef struct {
    uint8_t	iR_g1v[ADI_BMS_SIZE_R_G1V];                     // Redundant Auxiliary Register 1
    uint8_t	iR_g2v[ADI_BMS_SIZE_R_G2V];                     // Redundant Auxiliary Register 2
    uint8_t	iR_ga11v[ADI_BMS_SIZE_R_GA11V];                 // Redundant Auxiliary Register A11
    uint8_t	iRdraxa_pec[ADI_BMS_SIZE_RDRAXA_PEC];           // PEC for Redundant Auxiliary Register Group A
} adi_bms_rdraxa_t;

typedef struct {
    uint8_t	iR_g10v[ADI_BMS_SIZE_R_G10V];                   // Redundant Auxiliary Register 10
    uint8_t	iR_g3v[ADI_BMS_SIZE_R_G3V];                     // Redundant Auxiliary Register 3
    uint8_t	iR_g4v[ADI_BMS_SIZE_R_G4V];                     // Redundant Auxiliary Register 4
    uint8_t	iRdraxd_pec[ADI_BMS_SIZE_RDRAXD_PEC];           // PEC for Redundant Auxiliary Register Group D
} adi_bms_rdraxd_t;

typedef struct {
    uint8_t	iPwmr0;                                         // PWM Register 0
    uint8_t	iPwmr1;                                         // PWM Register 1
    uint8_t	iPwmr2;                                         // PWM Register 2
    uint8_t	iPwmr3;                                         // PWM Register 3
    uint8_t	iPwmr4;                                         // PWM Register 4
    uint8_t	iPwmr5;                                         // PWM Register 5
    uint8_t	iRdpwma_pec[ADI_BMS_SIZE_RDPWMA_PEC];           // PEC for PWMR[0:5]
} adi_bms_rdpwma_t;

typedef struct {
    uint8_t	iSidr0;                                         // Serial ID register, byte 0
    uint8_t	iSidr1;                                         // Serial ID register, byte 1
    uint8_t	iSidr2;                                         // Serial ID register, byte 2
    uint8_t	iSidr3;                                         // Serial ID register, byte 3
    uint8_t	iSidr4;                                         // Serial ID register, byte 4
    uint8_t	iSidr5;                                         // Serial ID register, byte 5
    uint8_t	iRdsid_pec[ADI_BMS_SIZE_RDSID_PEC];             // PEC for Serial Identification Code
} adi_bms_rdsid_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdaca_t           Rdaca;
    adi_bms_rdacb_t           Rdacb;
    adi_bms_rdacc_t           Rdacc;
    adi_bms_rdacd_t           Rdacd;
    adi_bms_rdace_t           Rdace;
    adi_bms_rdacf_t           Rdacf;
    adi_bms_rdauxa_t          Rdauxa;
    adi_bms_rdauxe_t          Rdauxe;
    adi_bms_rdstatc_t         Rdstatc;
} adi_bms_init_pkt_0_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdaca_t           Rdaca_1;
    adi_bms_rdacb_t           Rdacb_1;
    adi_bms_rdacc_t           Rdacc_1;
    adi_bms_rdacd_t           Rdacd_1;
    adi_bms_rdace_t           Rdace_1;
    adi_bms_rdacf_t           Rdacf_1;
    adi_bms_rdauxa_t          Rdauxa_1;
    adi_bms_rdauxe_t          Rdauxe_1;
    adi_bms_rdstatc_t         Rdstatc_1;
} adi_bms_init_pkt_1_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdaca_t           Rdaca_2;
    adi_bms_rdacb_t           Rdacb_2;
    adi_bms_rdacc_t           Rdacc_2;
    adi_bms_rdacd_t           Rdacd_2;
    adi_bms_rdace_t           Rdace_2;
    adi_bms_rdacf_t           Rdacf_2;
    adi_bms_rdauxa_t          Rdauxa_2;
    adi_bms_rdauxe_t          Rdauxe_2;
    adi_bms_rdstatc_t         Rdstatc_2;
} adi_bms_init_pkt_2_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdaca_t           Rdaca;
    adi_bms_rdacb_t           Rdacb;
    adi_bms_rdacc_t           Rdacc;
    adi_bms_rdacd_t           Rdacd;
    adi_bms_rdace_t           Rdace;
    adi_bms_rdacf_t           Rdacf;
    adi_bms_rdauxa_t          Rdauxa;
    adi_bms_rdauxe_t          Rdauxe;
    adi_bms_rdauxd_t          Rdauxd;
} adi_bms_base_pkt_0_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdsva_t           Rdsva;
    adi_bms_rdsvb_t           Rdsvb;
    adi_bms_rdsvc_t           Rdsvc;
    adi_bms_rdsvd_t           Rdsvd;
    adi_bms_rdsve_t           Rdsve;
    adi_bms_rdsvf_t           Rdsvf;
    adi_bms_rdcfga_t          Rdcfga;
    adi_bms_rdcfgb_t          Rdcfgb;
    adi_bms_rdstata_t         Rdstata;
} adi_bms_base_pkt_1_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdstatb_t         Rdstatb;
    adi_bms_rdstatc_t         Rdstatc;
    adi_bms_rdraxa_t          Rdraxa;
    adi_bms_rdraxd_t          Rdraxd;
} adi_bms_base_pkt_2_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdstatc_t         Rdstatc;
    adi_bms_rdstatc_t         Rdstatc_1;
    adi_bms_rdstatc_t         Rdstatc_2;
    adi_bms_rdstatc_t         Rdstatc_3;
    adi_bms_rdstatc_t         Rdstatc_4;
    adi_bms_rdstatc_t         Rdstatc_5;
    adi_bms_rdstatc_t         Rdstatc_6;
    adi_bms_rdpwma_t          Rdpwma;
    adi_bms_rdpwma_t          Rdpwma_1;
} adi_bms_latent0_pkt_0_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdpwma_t          Rdpwma_2;
    adi_bms_rdstatc_t         Rdstatc_7;
    adi_bms_rdauxd_t          Rdauxd;
    adi_bms_rdsid_t           Rdsid;
    adi_bms_rdstatc_t         Rdstatc_8;
    adi_bms_rdaca_t           Rdaca;
    adi_bms_rdacb_t           Rdacb;
    adi_bms_rdacc_t           Rdacc;
    adi_bms_rdacd_t           Rdacd;
} adi_bms_latent0_pkt_1_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdace_t           Rdace;
    adi_bms_rdacf_t           Rdacf;
    adi_bms_rdsva_t           Rdsva;
    adi_bms_rdsvb_t           Rdsvb;
    adi_bms_rdsvc_t           Rdsvc;
    adi_bms_rdsvd_t           Rdsvd;
    adi_bms_rdsve_t           Rdsve;
    adi_bms_rdsvf_t           Rdsvf;
    adi_bms_rdstatc_t         Rdstatc_9;
} adi_bms_latent0_pkt_2_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdaca_t           Rdaca;
    adi_bms_rdacb_t           Rdacb;
    adi_bms_rdacc_t           Rdacc;
    adi_bms_rdacd_t           Rdacd;
    adi_bms_rdace_t           Rdace;
    adi_bms_rdacf_t           Rdacf;
    adi_bms_rdsva_t           Rdsva;
    adi_bms_rdsvb_t           Rdsvb;
    adi_bms_rdsvc_t           Rdsvc;
} adi_bms_latent1_pkt_0_t;

typedef struct {
    adi_bms_packetheader_t    Packetheader;
    adi_bms_rdsvd_t           Rdsvd;
    adi_bms_rdsve_t           Rdsve;
    adi_bms_rdsvf_t           Rdsvf;
    adi_bms_rdstatc_t         Rdstatc;
} adi_bms_latent1_pkt_1_t;

#endif  /* ADI_BMS_TYPES_H_ */

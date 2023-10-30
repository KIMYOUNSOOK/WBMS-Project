/*******************************************************************************
 * @file            adi_wil_example_cell_balance.c
 *
 * @brief           Cell balance handler file
 *
 * @details  Provide functions to support the cell balance operation using ADBMS683x BMS IC.
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "adi_wil.h"
#include "adi_wil_api.h"
#include "adi_wil_example_functions.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_cell_balance.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_acl.h"

#include "adi_wil_example_debug_functions.h"

/*******************************************************************************/
/* Embedded WIL Function Declarations                                          */
/*******************************************************************************/
extern void WaitForWilAPI(adi_wil_pack_t * const pPack);

/*******************************************************************************/
/* Global Variable Declarations                                                */
/*******************************************************************************/
extern adi_wil_sensor_data_t    userBMSBuffer[BMS_DATA_PACKET_COUNT];

extern bool adi_gNotifyBms;
extern bool adi_gNotifyNetworkMeta;
extern adi_wil_api_t logAPIInProgress;
extern adi_wil_err_t gNotifRc;
extern volatile adi_wil_err_t adi_gProcessTaskErrorCode;

extern DISPLAYSTR ADK_DEMO;

uint8_t CB_NODE_CNT = 0;
uint8_t CB_CELL_CNT = 0;

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
uint16_t iDecTemp[12] = {0,};
uint16_t iDecFinal[12] = {0,};
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
uint32_t iDecTemp[12] = {0,};
uint32_t iDecFinal[12] = {0,};
#else   /* Not supported */
#endif

adi_wil_app_cell_balance_change_t pScriptChange;

/*******************************************************************************/
/* Local Variable Declarations                                                */
/*******************************************************************************/
/* Precomputed CRC15 Table */
const uint16_t adi_Crc15Table[256] ={
    0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e,
    0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5,
    0x7558, 0xb0c1, 0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6,
    0x4ff4, 0x8a6d, 0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3,
    0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320,
    0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544, 0x02be, 0xc727, 0xcc15, 0x098c,
    0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93,
    0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0, 0xdf0d, 0x1a94, 0x11a6, 0xd43f,
    0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf, 0xaa55, 0x6fcc,
    0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640, 0xa3d9,
    0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
    0x4a88, 0x8f11, 0x057c, 0xc0e5, 0xcbd7, 0x0e4e, 0xddb3, 0x182a, 0x1318, 0xd681,
    0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a,
    0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26,
    0x8edb, 0x4b42, 0x4070, 0x85e9, 0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2,
    0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e,
    0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0, 0x0d3a, 0xc8a3,
    0xc391, 0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01, 0x5f98,
    0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb,
    0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b,
    0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8,
    0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374,
    0xd089, 0x1510, 0x1e22, 0xdbbb, 0x0af8, 0xcf61, 0xc453, 0x01ca, 0xd237, 0x17ae,
    0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
    0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1,
    0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
};
/*******************************************************************************/
/*                          Private Functions                                  */
/*******************************************************************************/

/**
 * @brief Calculates  and returns the CRC15.
 *
 *
 */
uint16_t adi_pec15_Calc(uint8_t len, /* Number of bytes that will be used to calculate a PEC */
                        uint8_t *data /* Array of data that will be used to calculate  a PEC */
                        )
{
    uint16_t remainder, addr;
    remainder = 16; /* initialize the PEC */

    for (uint8_t i = 0; i<len; i++) /* loops for each byte in data array */
    {
        addr = (((remainder>>7)^data[i])&0xff); /* calculate PEC table address */
        remainder = ((remainder<<8)^adi_Crc15Table[addr]);
    }

    return(remainder*2); /* The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2 */
}

/**
 * @brief Calculates  and returns the CRC10.
 *
 *
 */
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
static uint16_t adi_pec10_calc(bool rx_cmd, int len, uint8_t *data)
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
static uint16_t adi_pec10_calc(bool rx_cmd, int offset, int len, uint8_t *data)
#else   /* Not supported */
#endif
{
    uint16_t remainder = 16; /* PEC_SEED;   0000010000 */
    uint16_t polynom = 0x8F; /* x10 + x7 + x3 + x2 + x + 1 <- the CRC15 polynomial         100 1000 1111   48F */

    /* Perform modulo-2 division, a byte at a time. */
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    for (int pbyte = 0; pbyte < len; ++pbyte)
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    for (int pbyte = offset; pbyte < (offset + len); ++pbyte)
#else   /* Not supported */
#endif
    {
        /* Bring the next byte into the remainder. */
        remainder ^= (uint16_t)(data[pbyte] << 2);

        /* Perform modulo-2 division, a bit at a time.*/
        for (uint8_t bit_ = 8; bit_ > 0; --bit_)
        {
            /* Try to divide the current data bit. */
            if ((remainder & 0x200) > 0) { /* equivalent to remainder & 2^14 simply check for MSB */
                remainder = (uint16_t)((remainder << 1));
                remainder = (uint16_t)(remainder ^ polynom);
            } else {
                remainder = (uint16_t)(remainder << 1);
            }
        }
    }

    if (rx_cmd == true) {
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
        remainder ^= (uint16_t)((data[len] & 0xFC) << 2);
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
        remainder ^= (uint16_t)((data[offset + len] & 0xFC) << 2);
#else   /* Not supported */
#endif

        /* Perform modulo-2 division, a bit at a time */
        for (uint8_t bit_ = 6; bit_ > 0; --bit_)
        {
            /* Try to divide the current data bit */
            if ((remainder & 0x200) > 0) { /* equivalent to remainder & 2^14 simply check for MSB */
                remainder = (uint16_t)((remainder << 1));
                remainder = (uint16_t)(remainder ^ polynom);
            } else {
                remainder = (uint16_t)((remainder << 1));
            }
        }
    }
    return ((uint16_t)(remainder & 0x3FF));
}

/**
 * @brief Function checks if the DCC field of the configuration register B after
 *        cell balancing operation was performed.
 *
 */
static void checkUpdatedDccField(adi_wil_pack_t *pPack, adi_wil_app_cell_balance_change_t *pDeviceListInfo,
                                 adi_wil_sensor_data_t *pBmsData,
                                 bool *bDccCheckFailureFlag, uint8_t CB_OPTION)
{
    bool bDcc1To8 = false, bDcc9To16 = false;
    uint8_t iReqPacketIdPosition = 0, currDeviceID = 0;

    for (int i=0; i<pDeviceListInfo->iCount; i++) {
        adi_wil_mac_deviceID_return(pPack, false,&(pDeviceListInfo->iDeviceList[i*ADI_WIL_MAC_ADDR_SIZE]), &currDeviceID);
        if((pDeviceListInfo->bCellBalanceSuccessFlag[i] == true) || !(pDeviceListInfo->iRetryCount[i] < MAX_NUM_OF_CELL_BALANCE_RETRY)) {
            continue;
        }
        else {
            for(uint8_t j=0; j < (BMS_PACKETS_PER_INTERVAL*ADI_WIL_MAX_NODES); j+=BMS_PACKETS_PER_INTERVAL) {         /* Finding required packet ID corresponding to given MAC */
                if(pBmsData[j].eDeviceId == currDeviceID) {
                    if (pBmsData[j].Data[BMS_PAYLOAD_PACKET_ID_OFFSET] == BMS_PAYLOAD_WRCFGB_PACKET_ID) {
                        iReqPacketIdPosition = j;
                        }
                    else if (pBmsData[j+1].Data[BMS_PAYLOAD_PACKET_ID_OFFSET] == BMS_PAYLOAD_WRCFGB_PACKET_ID) {
                       iReqPacketIdPosition = j+1;
                       }
                    else {
                        if( j == (BMS_PACKETS_PER_INTERVAL*ADI_WIL_MAX_NODES)) {
                            bDccCheckFailureFlag[i] = true;                        /* Failed to find required packet ID in the current BMS notification */
                        }
                        continue;
                    }
                    bDcc1To8 = (pDeviceListInfo->iDeviceChangeScriptInfo[i].iChangeData[WRCFGB_DCC_OFFSET] ==  pBmsData[iReqPacketIdPosition].Data[BMS_PAYLOAD_WRCFGB_OFFSET]);
                    bDcc9To16 = (pDeviceListInfo->iDeviceChangeScriptInfo[i].iChangeData[WRCFGB_DCC_OFFSET+1] == (pBmsData[iReqPacketIdPosition].Data[BMS_PAYLOAD_WRCFGB_OFFSET+1] & 0xFF));

                    if (bDcc1To8 && bDcc9To16) {                                /* Checking DCC fields */
                        bDccCheckFailureFlag[i] = false;
                        pDeviceListInfo->bCellBalanceSuccessFlag[i] = true;
                        break;
                        }
                    else {
                        bDccCheckFailureFlag[i] = true;
                        break;
                        }
                    }
                }
            }
        }
    }

/**
 * @brief Function retrieves latest BMS data and passes the same to checkUpdatedDccField function,
 *        function returns after max BMS data frame checks or DCC check success for all nodes in the pDeviceListInfo->iDeviceList.
 *
 */
static void checkModifiedScript(adi_wil_pack_t * const pPack,
                                adi_wil_app_cell_balance_change_t *pDeviceListInfo, uint8_t CB_OPTION)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    uint8_t iNumOfNotifFramesCheckedForUpdatedDcc = 0;
    bool bNextNotifDccCheckFlag = true, bAccumFlag = false;

    while ((iNumOfNotifFramesCheckedForUpdatedDcc < MAX_NUM_OF_NOTIF_BUFF_DCC_CHECK) && bNextNotifDccCheckFlag) {
        /* Retreive the latest BMS data to obtain the updated configuration B register values from the nodes in the network */
        while(!adi_gNotifyBms) {
            errorCode = adi_gProcessTaskErrorCode;

            if (errorCode != ADI_WIL_ERR_SUCCESS) {
                // adi_wil_ex_error("Call to WIL process task returned with error code = %s",
                //       adi_wil_ErrorToString(errorCode));
            }
        }
        adi_gNotifyBms = false;

        checkUpdatedDccField(pPack, pDeviceListInfo, userBMSBuffer, pDeviceListInfo->bFailureFlag, CB_OPTION);      /* Check the DCC field of the modified script */

        bAccumFlag = false;
        for (int i=0; i<pDeviceListInfo->iCount; i++) {
            bAccumFlag |= pDeviceListInfo->bFailureFlag[i];
        }

        if ( bAccumFlag == true) {
            bNextNotifDccCheckFlag = true;
        } else {
            bNextNotifDccCheckFlag = false;
        }
        iNumOfNotifFramesCheckedForUpdatedDcc++;
    }
}

bool adi_wil_example_ExecuteModifyScript(adi_wil_pack_t * const pPack, uint8_t CB_OPTION)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    memset(&pScriptChange,0, sizeof(pScriptChange));
    uint8_t devID = 0, iNodesFailedIndex[ADI_WIL_MAX_NODES], iNodesFailedCount = 0, iLocRetryCount = 0;
    memset(iNodesFailedIndex,0, sizeof(iNodesFailedIndex));
    uint16_t pecVal = 0, currPktTimestamp = 0, calPktTimestamp = 0;
    // uint16_t cellCheck = 0;/* @remark  : Not used */
    // int iDelayStart = 0, iDelayEnd = 0; /* @remark  : Not used */
    uint16_t node_count = 0; 
    bool bModifyScriptMainFlag = true, bModifyScriptLoopFlag = true, bFailureAccumFlag = false, bMaxRetryFlag = true;
    int j = 0;
    #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    int k = 0;
    #endif

    /**
     * @remark : Parse CB map
     */
    for(CB_NODE_CNT = 0; CB_NODE_CNT < realAcl.iCount; CB_NODE_CNT++){
        ADK_DEMO.CB_NODE[CB_NODE_CNT] = false;
        iDecTemp[CB_NODE_CNT] = 0;
        iDecFinal[CB_NODE_CNT] = 0;
    }

    for(CB_NODE_CNT = 0; CB_NODE_CNT < realAcl.iCount; CB_NODE_CNT++){
        for(CB_CELL_CNT = 0; CB_CELL_CNT < ADK_MAX_cell; CB_CELL_CNT++){
            if(ADK_DEMO.CB_CELL[CB_NODE_CNT][CB_CELL_CNT] == TRUE){
                iDecTemp[CB_NODE_CNT] += (1 << CB_CELL_CNT);
                ADK_DEMO.CB_NODE[CB_NODE_CNT] = true;
            }
        }

        /**
         * @remark : Cell balancing user input goes here
         */
    
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
        if(CB_OPTION == 0){
            iDecFinal[CB_NODE_CNT] = iDecTemp[CB_NODE_CNT];
            iDecFinal[CB_NODE_CNT] &= 0xAAAA; //EVEN for [6830]
        }
        else if(CB_OPTION == 1){
            iDecFinal[CB_NODE_CNT] = iDecTemp[CB_NODE_CNT];
            iDecFinal[CB_NODE_CNT] &= 0x5555; //ODD for  [6830]
        }
        else{
            iDecFinal[CB_NODE_CNT] = 0; //DCC default roll back
        }       
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
        if(CB_OPTION == 0){
            iDecFinal[CB_NODE_CNT] = iDecTemp[CB_NODE_CNT];
            iDecFinal[CB_NODE_CNT] &= 0x2A954; //EVEN for [6833]
        }
        else if(CB_OPTION == 1){
            iDecFinal[CB_NODE_CNT] = iDecTemp[CB_NODE_CNT];
            iDecFinal[CB_NODE_CNT] &= 0x154AA; //ODD for  [6833]
        }
        else{
            iDecFinal[CB_NODE_CNT] = 0; //DCC default roll back
        }    
#else   /* Not supported */
#endif
        
    }

    pScriptChange.iCount = realAcl.iCount; //node count
    memcpy(pScriptChange.iDeviceList, realAcl.Data, sizeof(uint8_t) * 8 * realAcl.iCount); //MAC address

    for(node_count = 0; node_count < pScriptChange.iCount; node_count++){
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[0] = 0x80;
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[1] = 0x00;        
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGA_DCC_OFFSET] = 0x40;
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGA_DCC_OFFSET] |= (uint8_t)((iDecFinal[node_count] / 0x10000 ) & BMS_CELLS_1_TO_2_MASK);            // for WRCFGA DCC
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGA_DCC_OFFSET+1] = 0xFF;
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGA_DCC_OFFSET+2] = 0x03;
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGA_DCC_OFFSET+3] = 0x07;
#else   /* Not supported */
#endif
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGB_DCC_OFFSET] = (uint8_t)(iDecFinal[node_count] & BMS_CELLS_1_TO_8_MASK);
        pScriptChange.iDeviceChangeScriptInfo[node_count].iChangeData[WRCFGB_DCC_OFFSET+1] = (uint8_t)((iDecFinal[node_count] & BMS_CELLS_9_TO_16_MASK) >> 8);
    }

    while(bModifyScriptLoopFlag) {
        if(iLocRetryCount > 0) {
            // adi_wil_ex_info("Cell balance retry counter = %d", iLocRetryCount);
        }
        for (j = 0; j < pScriptChange.iCount; j++) {
            if(ADK_DEMO.CB_NODE[j] == true){
                if((pScriptChange.bCellBalanceSuccessFlag[j] == false) && (pScriptChange.iRetryCount[j] <= MAX_NUM_OF_CELL_BALANCE_RETRY)) {

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
                    /* Intialise the parameters necessary to change a specfic entry in the node */
                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeDataLength = BMS_SCRIPT_WRCFGB_DATA_LENGTH;    /* Length of the data to change in the script at the node */
                    pScriptChange.iDeviceChangeScriptInfo[j].iEntryOffset = BMS_SCRIPT_WRCFGB_OFFSET;              /* Offset to the DCC bits to be changed in Config B register */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
                    /* Intialise the parameters necessary to change a specfic entry in the node */
                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeDataLength = BMS_SCRIPT_WRCFGA_DATA_LENGTH;    /* Length of the data to change in the script at the node */
                    pScriptChange.iDeviceChangeScriptInfo[j].iEntryOffset = BMS_SCRIPT_WRCFGA_OFFSET;              /* Offset to the DCC bits to be changed in Config B register */
#else   /* Not supported */
#endif
                    /* Retreive latest BMS data to obtain the most updated timestamp */
                    while(!adi_gNotifyBms) {
                        errorCode = adi_gProcessTaskErrorCode;

                        if (errorCode != ADI_WIL_ERR_SUCCESS) {
                            // adi_wil_ex_error("Call to WIL process task returned with error code = %s",
                            //         adi_wil_ErrorToString(errorCode));
                        }
                    }
                    adi_gNotifyBms = false;

                    /* iActivationTime needs to be calculated from latest BMS packet timestamp and ACTIVATION_DELAY specified. Latest BMS packet will be retrieved from the timestamp
                        field from the latest BMS data packet */
                    if(j == 0)
                    {
                        currPktTimestamp = userBMSBuffer[0].Data[PACKET_HEADER_TIMESTAMP_OFFSET] << 8;
                        currPktTimestamp |= userBMSBuffer[0].Data[PACKET_HEADER_TIMESTAMP_OFFSET+1];
                        currPktTimestamp = (currPktTimestamp & 0x7FFF) + ACTIVATION_DELAY;

                        calPktTimestamp = (currPktTimestamp & BMS_CELLS_9_TO_16_MASK) >> 8;
                        calPktTimestamp |= (currPktTimestamp & BMS_CELLS_1_TO_8_MASK) << 8;
                    }
                    pScriptChange.iDeviceChangeScriptInfo[j].iActivationTime = calPktTimestamp;

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
                    /* Calculating the PEC for updated configuration B register to be set on the node */
                    pecVal = adi_pec10_calc(true,
                                            BMS_SCRIPT_WRCFGB_DATA_LENGTH-2,
                                            pScriptChange.iDeviceChangeScriptInfo[j].iChangeData);
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
                    /* Calculating the PEC for updated configuration A register to be set on the node */
                    pecVal = adi_pec10_calc(true, 0,
                                            BMS_SCRIPT_WRCFGA_DATA_LENGTH - 2,
                                            pScriptChange.iDeviceChangeScriptInfo[j].iChangeData);

                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[WRCFGA_PEC_OFFSET] = (uint8_t)(pecVal >> 8);
                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[WRCFGA_PEC_OFFSET+1] = (uint8_t)(pecVal >> 0);

                    /* Calculating the PEC for updated configuration B register to be set on the node */
                    pecVal = adi_pec10_calc(true, WRCFGB_PEC_OFFSET - WRCFGA_PEC_OFFSET,
                                            BMS_SCRIPT_WRCFGB_DATA_LENGTH - 2,
                                            pScriptChange.iDeviceChangeScriptInfo[j].iChangeData);
#else   /* Not supported */
#endif

                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[WRCFGB_PEC_OFFSET] = (uint8_t)(pecVal >> 8);
                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[WRCFGB_PEC_OFFSET+1] = (uint8_t)(pecVal >> 0);
                    adi_wil_mac_deviceID_return(pPack, false, &pScriptChange.iDeviceList[ADI_WIL_MAC_ADDR_SIZE*j], &devID);

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
                    /* Calling ModifyScript node API */
                    logAPIInProgress = ADI_WIL_API_MODIFY_SCRIPT;
                    errorCode = adi_wil_ModifyScript(pPack,
                                                        (adi_wil_device_t)(ADI_WIL_DEV_NODE_0 << devID),
                                                        ADI_WIL_SENSOR_ID_BMS,
                                                        &pScriptChange.iDeviceChangeScriptInfo[j]);
                    if (errorCode != ADI_WIL_ERR_SUCCESS) {
                        pScriptChange.bFailureFlag[j] = true;
                        continue;
                        }
                    else {
                        /* Wait for non-blocking API to complete */
                        WaitForWilAPI(pPack);
                        /* Check Callback RC for success */
                        if (gNotifRc != ADI_WIL_ERR_SUCCESS) {
                            pScriptChange.bFailureFlag[j] = true;
                            continue;
                        }
                        else if(gNotifRc == ADI_WIL_ERR_SUCCESS) {
                            /* Success */
                        }                    
                    }

                    for (k = 0; k < BMS_SCRIPT_WRCFGB_DATA_LENGTH; k++)
                    {
                        pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[k] = pScriptChange.iDeviceChangeScriptInfo[j].iChangeData[k+BMS_SCRIPT_WRCFGA_DATA_LENGTH];
                    }
                    pScriptChange.iDeviceChangeScriptInfo[j].iChangeDataLength = BMS_SCRIPT_WRCFGB_DATA_LENGTH;    /* Length of the data to change in the script at the node */
                    pScriptChange.iDeviceChangeScriptInfo[j].iEntryOffset = BMS_SCRIPT_WRCFGB_OFFSET;

#else   /* Not supported */
#endif

                     /* Calling ModifyScript node API */
                    logAPIInProgress = ADI_WIL_API_MODIFY_SCRIPT;
                    errorCode = adi_wil_ModifyScript(pPack,
                                                        (adi_wil_device_t)(ADI_WIL_DEV_NODE_0 << devID),
                                                        ADI_WIL_SENSOR_ID_BMS,
                                                        &pScriptChange.iDeviceChangeScriptInfo[j]);
                    
                    if (errorCode != ADI_WIL_ERR_SUCCESS) {
                        pScriptChange.bFailureFlag[j] = true;
                        continue;
                        }
                    else {
                        /* Wait for non-blocking API to complete */
                        WaitForWilAPI(pPack);
                        /* Check Callback RC for success */
                        if (gNotifRc != ADI_WIL_ERR_SUCCESS) {
                            pScriptChange.bFailureFlag[j] = true;
                            continue;
                        }
                        else if(gNotifRc == ADI_WIL_ERR_SUCCESS) {
                            /* Success */
                        }                    
                    }
                    
                }//if-cell balance success & retry flag check
            }
        }//for- node loop

        // adi_wil_ex_info("Busy waiting for time = %dsec", CELL_BALANCE_DELAY_IN_SEC);
        // adi_wil_ex_info("\t\t...Once the node receives the cell balancing command, it waits %d seconds to apply the configuration", CELL_BALANCE_DELAY_IN_SEC);

        /**
         * @remark : (Optional) please add a delay function here that waits for a time of "CELL_BALANCE_DELAY_IN_SEC"
         */
        // iDelayStart = adi_wil_hal_TickerGetTimestamp();
        // while(iDelayEnd < (iDelayStart + (CELL_BALANCE_DELAY_IN_SEC*1000))) {    /* Wait for a time of "CELL_BALANCE_DELAY_IN_SEC" */
        //     iDelayEnd = adi_wil_hal_TickerGetTimestamp();
        // } 

        /* Check if the DCC configurations returned from the nodes matches intended DCC configurations */
        checkModifiedScript(pPack, &pScriptChange, CB_OPTION);

        bMaxRetryFlag = true;
        bFailureAccumFlag = false;
        iNodesFailedCount = 0;
        memset(iNodesFailedIndex,0, sizeof(iNodesFailedIndex));
        for(uint8_t i=0 ; i<pScriptChange.iCount ; i++) {
            bFailureAccumFlag |= pScriptChange.bFailureFlag[i];
            if(pScriptChange.bFailureFlag[i] == true) {
                iNodesFailedIndex[iNodesFailedCount] = i;
                iNodesFailedCount++;
            }
        }
        if(bFailureAccumFlag == false) {
            bModifyScriptLoopFlag = false;
            bModifyScriptMainFlag = true;
        }
        else {
            iLocRetryCount++;
            for(uint8_t i=0 ; i<iNodesFailedCount ;i++) {
                if(!(pScriptChange.iRetryCount[iNodesFailedIndex[i]] < MAX_NUM_OF_CELL_BALANCE_RETRY)) {
                    bMaxRetryFlag &= true;
                    }
                else {
                    bMaxRetryFlag &= false;
                    pScriptChange.iRetryCount[iNodesFailedIndex[i]]++;
                    }
                }
                if(bMaxRetryFlag) {
                    // adi_wil_ex_info("Max number of retries reached!");
                    bModifyScriptLoopFlag = false;
                    bModifyScriptMainFlag = false;
                }
        }
    }//While-main loop for retries

    return bModifyScriptMainFlag; // return status of modify script functionality
}

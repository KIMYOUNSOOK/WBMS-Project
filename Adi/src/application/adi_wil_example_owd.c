/*******************************************************************************
 * @file     adi_wil_example_owd.c
 *
 * @brief    Open Wire Detection Example
 *
 * @details  Contains function for evaluation of Open Wire detection
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#include "adi_wil_example_owd.h"
#include "adi_bms_types.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_functions.h"

#include "adi_wil_example_debug_functions.h"


/*******************************************************************************/
/* Local Variable Declarations                                                 */
/*******************************************************************************/

/* aiOWStatus - Array to store all Open Wire status for each Node in the network */
static uint8_t  aiOWStatus[ADI_WIL_MAX_NODES];

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
/* aiOWCellData - Array to store Open Wire cell data for all the Nodes */
static int16_t aiOWCellData[ADI_WIL_MAX_NODES][SM_ADBMS6830_TOTAL_CELLS_PER_DEVICE];
/* aiOWCellData - Array to store nominal cell data for all the Nodes */
static int16_t aiNominalCellData[ADI_WIL_MAX_NODES][SM_ADBMS6830_TOTAL_CELLS_PER_DEVICE];
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
/* aiOWCellData - Array to store Open Wire cell data for all the Nodes */
static int16_t aiOWCellData[ADI_WIL_MAX_NODES][SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE];
/* aiOWCellData - Array to store nominal cell data for all the Nodes */
static int16_t aiNominalCellData[ADI_WIL_MAX_NODES][SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE];
#else   /* Not supported */
#endif

/* iOWDPcktsRcvd - Status flag to indicate if all the packets for Open Wire detection are received */
uint8_t  iOWDPcktsRcvd[ADI_WIL_MAX_NODES];

/*******************************************************************************/
/* Extern Variable Declarations                                                */
/*******************************************************************************/
extern void adi_wil_example_ADK_readBms(void);

extern bool                         adi_gNotifyBms;
extern uint8_t                      gTotalNodes;
extern uint16_t                     nTotalPcktsRcvd;
extern adi_wil_pack_t               packInstance;
extern adi_wil_sensor_data_t        userBMSBuffer[BMS_DATA_PACKET_COUNT];
extern DISPLAYSTR ADK_DEMO;
extern bool OWD_PACKET_RECEIVED;
extern void adk_debug_Report(ADK_FAIL_API_NAME api, adi_wil_err_t rc );

/*******************************************************************************/
/* Local function Declarations                                                 */
/*******************************************************************************/


/*******************************************************************************/
/* Functions                                                                   */
/*******************************************************************************/

void Adbms683x_Monitor_Cell_OWD(adi_wil_device_t eNode);

/**
 * @brief   Interface to read required SM data from Auxiliary Base, Packet 1
 *
 * @details This function reads registers required for Open Wire dectection from Base script, Packet 1
 *
 * @param  adi_wil_sensor_data_t[in]  Pointer to BMS packet received
 *
 * @return None
 */
void Adbms683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr)
{
    uint8_t eNode   = 100;
    adi_bms_base_pkt_1_t* pPacketPtr;

    /* Get Node ID from the BMS packet */
    eNode       = ADK_ConvertDeviceId(BMSBufferPtr->eDeviceId);

    /* Use corresponding packet structure to easily parse data from the BMS packet */
    pPacketPtr  = (adi_bms_base_pkt_1_t*) &BMSBufferPtr->iLength;

    #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    /******************** Get S ch Data for Open Wire detection ********************/
    aiOWCellData[eNode][3]  = (pPacketPtr->Rdsvb.iS4v[1] << 8) + pPacketPtr->Rdsvb.iS4v[0];
    aiOWCellData[eNode][4]  = (pPacketPtr->Rdsvb.iS5v[1] << 8) + pPacketPtr->Rdsvb.iS5v[0];
    aiOWCellData[eNode][5]  = (pPacketPtr->Rdsvb.iS6v[1] << 8) + pPacketPtr->Rdsvb.iS6v[0];
    aiOWCellData[eNode][6]  = (pPacketPtr->Rdsvc.iS7v[1] << 8) + pPacketPtr->Rdsvc.iS7v[0];
    aiOWCellData[eNode][7]  = (pPacketPtr->Rdsvc.iS8v[1] << 8) + pPacketPtr->Rdsvc.iS8v[0];
    aiOWCellData[eNode][8]  = (pPacketPtr->Rdsvc.iS9v[1] << 8) + pPacketPtr->Rdsvc.iS9v[0];
    aiOWCellData[eNode][9]  = (pPacketPtr->Rdsvd.iS10v[1] << 8) + pPacketPtr->Rdsvd.iS10v[0];
    aiOWCellData[eNode][10] = (pPacketPtr->Rdsvd.iS11v[1] << 8) + pPacketPtr->Rdsvd.iS11v[0];
    aiOWCellData[eNode][11] = (pPacketPtr->Rdsvd.iS12v[1] << 8) + pPacketPtr->Rdsvd.iS12v[0];
    aiOWCellData[eNode][12] = (pPacketPtr->Rdsve.iS13v[1] << 8) + pPacketPtr->Rdsve.iS13v[0];
    aiOWCellData[eNode][13] = (pPacketPtr->Rdsve.iS14v[1] << 8) + pPacketPtr->Rdsve.iS14v[0];
    aiOWCellData[eNode][14] = (pPacketPtr->Rdsve.iS15v[1] << 8) + pPacketPtr->Rdsve.iS15v[0];
    aiOWCellData[eNode][15] = (pPacketPtr->Rdsvf.iS16v[1] << 8) + pPacketPtr->Rdsvf.iS16v[0];

    iOWDPcktsRcvd[eNode] |= SM_ADBMS6830_S_CH_EVEN_RCVD;
    #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
     aiOWCellData[eNode][0]  = (pPacketPtr->Rdsva.iS2v[1] << 8) + pPacketPtr->Rdsva.iS2v[0];
     aiOWCellData[eNode][1]  = (pPacketPtr->Rdsva.iS3v[1] << 8) + pPacketPtr->Rdsva.iS3v[0];

    /******************** Get S ch Data for Open Wire detection ********************/
    aiOWCellData[eNode][2]  = (pPacketPtr->Rdsvb.iS4v[1] << 8) + pPacketPtr->Rdsvb.iS4v[0];
    aiOWCellData[eNode][3]  = (pPacketPtr->Rdsvb.iS5v[1] << 8) + pPacketPtr->Rdsvb.iS5v[0];
    aiOWCellData[eNode][4]  = (pPacketPtr->Rdsvb.iS6v[1] << 8) + pPacketPtr->Rdsvb.iS6v[0];
    aiOWCellData[eNode][5]  = (pPacketPtr->Rdsvc.iS7v[1] << 8) + pPacketPtr->Rdsvc.iS7v[0];
    aiOWCellData[eNode][6]  = (pPacketPtr->Rdsvc.iS8v[1] << 8) + pPacketPtr->Rdsvc.iS8v[0];
    aiOWCellData[eNode][7]  = (pPacketPtr->Rdsvc.iS9v[1] << 8) + pPacketPtr->Rdsvc.iS9v[0];
    aiOWCellData[eNode][8]  = (pPacketPtr->Rdsvd.iS11v[1] << 8) + pPacketPtr->Rdsvd.iS11v[0];
    aiOWCellData[eNode][9]  = (pPacketPtr->Rdsvd.iS12v[1] << 8) + pPacketPtr->Rdsvd.iS12v[0];
    aiOWCellData[eNode][10] = (pPacketPtr->Rdsve.iS13v[1] << 8) + pPacketPtr->Rdsve.iS13v[0];
    aiOWCellData[eNode][11] = (pPacketPtr->Rdsve.iS14v[1] << 8) + pPacketPtr->Rdsve.iS14v[0];
    aiOWCellData[eNode][12] = (pPacketPtr->Rdsve.iS15v[1] << 8) + pPacketPtr->Rdsve.iS15v[0];
    aiOWCellData[eNode][13] = (pPacketPtr->Rdsvf.iS16v[1] << 8) + pPacketPtr->Rdsvf.iS16v[0];
    aiOWCellData[eNode][14] = (pPacketPtr->Rdsvf.iS17v[1] << 8) + pPacketPtr->Rdsvf.iS17v[0];
    aiOWCellData[eNode][15] = (pPacketPtr->Rdsvf.iS18v[1] << 8) + pPacketPtr->Rdsvf.iS18v[0];

    iOWDPcktsRcvd[eNode] |= SM_ADBMS6833_S_CH_EVEN_RCVD;
    #else   /* Not supported */
    #endif

}

/**
 * @brief   Interface to read required SM data from Base, Packet 0
 *
 * @details This function reads registers required for Open Wire dectection from Base script, Packet 0
 *
 * @param   adi_wil_sensor_data_t[in]  Pointer to BMS packet received
 *
 * @return None
 */
void Adbms683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr)
{
    uint8_t eNode   = 100;
    adi_bms_base_pkt_0_t* pPacketPtr;

    /* Get Node ID from the BMS packet */
    eNode       = ADK_ConvertDeviceId(BMSBufferPtr->eDeviceId);

    /* Use corresponding packet structure to easily parse data from the BMS packet */
    pPacketPtr  = (adi_bms_base_pkt_0_t*) &BMSBufferPtr->iLength;

    #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    /******************** Get C ch Data for Open Wire detection ********************/
    aiNominalCellData[eNode][0]  = (pPacketPtr->Rdaca.iAc1v[1]  << 8) + pPacketPtr->Rdaca.iAc1v[0];
    aiNominalCellData[eNode][1]  = (pPacketPtr->Rdaca.iAc2v[1]  << 8) + pPacketPtr->Rdaca.iAc2v[0];
    aiNominalCellData[eNode][2]  = (pPacketPtr->Rdaca.iAc3v[1]  << 8) + pPacketPtr->Rdaca.iAc3v[0];
    aiNominalCellData[eNode][3]  = (pPacketPtr->Rdacb.iAc4v[1]  << 8) + pPacketPtr->Rdacb.iAc4v[0];
    aiNominalCellData[eNode][4]  = (pPacketPtr->Rdacb.iAc5v[1]  << 8) + pPacketPtr->Rdacb.iAc5v[0];
    aiNominalCellData[eNode][5]  = (pPacketPtr->Rdacb.iAc6v[1]  << 8) + pPacketPtr->Rdacb.iAc6v[0];
    aiNominalCellData[eNode][6]  = (pPacketPtr->Rdacc.iAc7v[1]  << 8) + pPacketPtr->Rdacc.iAc7v[0];
    aiNominalCellData[eNode][7]  = (pPacketPtr->Rdacc.iAc8v[1]  << 8) + pPacketPtr->Rdacc.iAc8v[0];
    aiNominalCellData[eNode][8]  = (pPacketPtr->Rdacc.iAc9v[1]  << 8) + pPacketPtr->Rdacc.iAc9v[0];
    aiNominalCellData[eNode][9]  = (pPacketPtr->Rdacd.iAc10v[1] << 8) + pPacketPtr->Rdacd.iAc10v[0];
    aiNominalCellData[eNode][10] = (pPacketPtr->Rdacd.iAc11v[1] << 8) + pPacketPtr->Rdacd.iAc11v[0];
    aiNominalCellData[eNode][11] = (pPacketPtr->Rdacd.iAc12v[1] << 8) + pPacketPtr->Rdacd.iAc12v[0];
    aiNominalCellData[eNode][12] = (pPacketPtr->Rdace.iAc13v[1] << 8) + pPacketPtr->Rdace.iAc13v[0];
    aiNominalCellData[eNode][13] = (pPacketPtr->Rdace.iAc14v[1] << 8) + pPacketPtr->Rdace.iAc14v[0];
    aiNominalCellData[eNode][14] = (pPacketPtr->Rdace.iAc15v[1] << 8) + pPacketPtr->Rdace.iAc15v[0];
    aiNominalCellData[eNode][15] = (pPacketPtr->Rdacf.iAc16v[1] << 8) + pPacketPtr->Rdacf.iAc16v[0];

    /******************** Get S ch Data for Open Wire detection ********************/
    aiOWCellData[eNode][0]  = (pPacketPtr->Rdsva.iS1v[1] << 8) + pPacketPtr->Rdsva.iS1v[0]; 
    aiOWCellData[eNode][1]  = (pPacketPtr->Rdsva.iS2v[1] << 8) + pPacketPtr->Rdsva.iS2v[0];     
    aiOWCellData[eNode][2]  = (pPacketPtr->Rdsva.iS3v[1] << 8) + pPacketPtr->Rdsva.iS3v[0];
    
    iOWDPcktsRcvd[eNode] |= SM_ADBMS6830_C_CH_ODD_RCVD;
    iOWDPcktsRcvd[eNode] |= SM_ADBMS6830_C_CH_EVEN_RCVD;
    #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    /******************** Get C ch Data for Open Wire detection ********************/
    aiNominalCellData[eNode][0]  = (pPacketPtr->Rdaca.iAc2v[1] << 8) +  pPacketPtr->Rdaca.iAc2v[0];    
    aiNominalCellData[eNode][1]  = (pPacketPtr->Rdaca.iAc3v[1] << 8) +  pPacketPtr->Rdaca.iAc3v[0];   
    aiNominalCellData[eNode][2]  = (pPacketPtr->Rdacb.iAc4v[1] << 8) +  pPacketPtr->Rdacb.iAc4v[0];  
    aiNominalCellData[eNode][3]  = (pPacketPtr->Rdacb.iAc5v[1] << 8) +  pPacketPtr->Rdacb.iAc5v[0]; 
    aiNominalCellData[eNode][4]  = (pPacketPtr->Rdacb.iAc6v[1] << 8) +  pPacketPtr->Rdacb.iAc6v[0];  
    aiNominalCellData[eNode][5]  = (pPacketPtr->Rdacc.iAc7v[1] << 8) +  pPacketPtr->Rdacc.iAc7v[0];  
    aiNominalCellData[eNode][6]  = (pPacketPtr->Rdacc.iAc8v[1] << 8) +  pPacketPtr->Rdacc.iAc8v[0];
    aiNominalCellData[eNode][7]  = (pPacketPtr->Rdacc.iAc9v[1] << 8) +  pPacketPtr->Rdacc.iAc9v[0];
    aiNominalCellData[eNode][8]  = (pPacketPtr->Rdacd.iAc11v[1] << 8) + pPacketPtr->Rdacd.iAc11v[0];
    aiNominalCellData[eNode][9]  = (pPacketPtr->Rdacd.iAc12v[1] << 8) + pPacketPtr->Rdacd.iAc12v[0];
    aiNominalCellData[eNode][10] = (pPacketPtr->Rdace.iAc13v[1] << 8) + pPacketPtr->Rdace.iAc13v[0];
    aiNominalCellData[eNode][11] = (pPacketPtr->Rdace.iAc14v[1] << 8) + pPacketPtr->Rdace.iAc14v[0];
    aiNominalCellData[eNode][12] = (pPacketPtr->Rdace.iAc15v[1] << 8) + pPacketPtr->Rdace.iAc15v[0];
    aiNominalCellData[eNode][13] = (pPacketPtr->Rdacf.iAc16v[1] << 8) + pPacketPtr->Rdacf.iAc16v[0];
    aiNominalCellData[eNode][14] = (pPacketPtr->Rdacf.iAc17v[1] << 8) + pPacketPtr->Rdacf.iAc17v[0];
    aiNominalCellData[eNode][15] = (pPacketPtr->Rdacf.iAc18v[1] << 8) + pPacketPtr->Rdacf.iAc18v[0];

    /******************** Get S ch Data for Open Wire detection ********************/
 //   aiOWCellData[eNode][0]  = (pPacketPtr->Rdsva.iS2v[1] << 8) + pPacketPtr->Rdsva.iS2v[0];
 //   aiOWCellData[eNode][1]  = (pPacketPtr->Rdsva.iS3v[1] << 8) + pPacketPtr->Rdsva.iS3v[0];
        
    iOWDPcktsRcvd[eNode] |= SM_ADBMS6833_C_CH_ODD_RCVD;
    iOWDPcktsRcvd[eNode] |= SM_ADBMS6833_C_CH_EVEN_RCVD;
    #else   /* Not supported */
    #endif

}


/**
 * @brief   Interface to read required SM data from Base, Packet 2
 *
 * @details This function reads registers required for Open Wire dectection from Base script, Packet 2
 *
 * @param  adi_wil_sensor_data_t[in]  Pointer to BMS packet received
 *
 * @return None
 */

void Adbms683x_Monitor_Base_Pkt2(adi_wil_sensor_data_t* BMSBufferPtr)
{
    uint8_t eNode   = 100;
    adi_bms_base_pkt_2_t* pPacketPtr;

    /* Get Node ID from the BMS packet */
    eNode       = ADK_ConvertDeviceId(BMSBufferPtr->eDeviceId);

    /* Use corresponding packet structure to easily parse data from the BMS packet */
    pPacketPtr  = (adi_bms_base_pkt_2_t*) &BMSBufferPtr->iLength;

}


/**
 * @brief   Interface to evaluate Cell Open Wire
 *
 * @details This function checks for Open Wire from the nominal cell volages and the Open wire cell voltage
 *
 * @param  eNode[in]  Node for which Open Wire condtion needs to be checked
 *
 * @return None
 */
void Adbms683x_Monitor_Cell_OWD(adi_wil_device_t eNode)
{
    uint8_t iCellIndx       = 0U;
    int32_t iCellData       = 0U;
    int32_t iCellOWData     = 0U;
    int32_t iCellDrop       = 0U;
    int32_t iCellOWThres_p    = 0U;
    int32_t iCellOWThres_m    = 0U;

    #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    aiOWStatus[eNode] = SM_ADBMS6830_STATUS_PASS;
    ADK_DEMO.NODE[eNode].OWD_STAT = SM_ADBMS6830_STATUS_PASS;

    for(iCellIndx = 0U; iCellIndx < SM_ADBMS6830_TOTAL_CELLS_PER_DEVICE; iCellIndx++)
    {
        iCellData    = (aiNominalCellData[eNode][iCellIndx] * SM_ADBMS6830_VTG_REG_RESOLUTION) + SM_ADBMS6830_VTG_REG_OFFSET;
        iCellOWData  = (aiOWCellData[eNode][iCellIndx]      * SM_ADBMS6830_VTG_REG_RESOLUTION) + SM_ADBMS6830_VTG_REG_OFFSET;
        iCellDrop    = iCellData - iCellOWData;

        iCellOWThres_p = ((iCellData * SM_ADBMS6830_OW_THRESHOLD_PERCENT) / 100);
        iCellOWThres_m = -((iCellData * SM_ADBMS6830_OW_THRESHOLD_PERCENT) / 100);

        if(aiNominalCellData[eNode][iCellIndx] < SM_ADBMS6830_OW_ZERO_VOL)
        {
            ADK_DEMO.NODE[eNode].OWD_FAIL[iCellIndx]++;
        }
        else
        {
            if ((iCellDrop > iCellOWThres_p) || (iCellDrop < iCellOWThres_m))
            {
                ADK_DEMO.NODE[eNode].OWD_FAIL[iCellIndx]++;
                aiOWStatus[eNode] = SM_ADBMS6830_STATUS_FAIL;
                ADK_DEMO.NODE[eNode].OWD_STAT = SM_ADBMS6830_STATUS_FAIL;
            }
            else{ /* DEBUG */
                ADK_DEMO.NODE[eNode].OWD_SUCCESS[iCellIndx]++;
            }
        }
    
    }
    #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    aiOWStatus[eNode] = SM_ADBMS6833_STATUS_PASS;
    ADK_DEMO.NODE[eNode].OWD_STAT = SM_ADBMS6833_STATUS_PASS;

    for(iCellIndx = 0U; iCellIndx < SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE; iCellIndx++)
    {
        iCellData    = (aiNominalCellData[eNode][iCellIndx] * SM_ADBMS6833_VTG_REG_RESOLUTION) + SM_ADBMS6833_VTG_REG_OFFSET;
        iCellOWData  = (aiOWCellData[eNode][iCellIndx]      * SM_ADBMS6833_VTG_REG_RESOLUTION) + SM_ADBMS6833_VTG_REG_OFFSET;
        iCellDrop    = iCellData - iCellOWData;

        iCellOWThres_p = ((iCellData * SM_ADBMS6833_OW_THRESHOLD_PERCENT) / 100);
        iCellOWThres_m = -((iCellData * SM_ADBMS6833_OW_THRESHOLD_PERCENT) / 100);

        if(aiNominalCellData[eNode][iCellIndx] < SM_ADBMS6833_OW_ZERO_VOL)
        {
            ADK_DEMO.NODE[eNode].OWD_FAIL[iCellIndx]++;
        }
        else
        {
            if ((iCellDrop > iCellOWThres_p) || (iCellDrop < iCellOWThres_m))
            {
                ADK_DEMO.NODE[eNode].OWD_FAIL[iCellIndx]++;
                aiOWStatus[eNode] = SM_ADBMS6833_STATUS_FAIL;
                ADK_DEMO.NODE[eNode].OWD_STAT = SM_ADBMS6833_STATUS_FAIL;
            }
            else{ /* DEBUG */
                ADK_DEMO.NODE[eNode].OWD_SUCCESS[iCellIndx]++;
            }
        }
    
    }
    #else   /* Not supported */
    #endif

}

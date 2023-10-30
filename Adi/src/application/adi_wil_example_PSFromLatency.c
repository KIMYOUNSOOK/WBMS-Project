/*******************************************************************************
 * @file     adi_wil_example_PSFromLatency.c
 *
 * @brief    Platform Application WIL support functions
 *
 * @details  Include wrapping WIL functionality.
 *
 * Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "adi_wil_example_PSFromLatency.h"
#include "adi_wil_example_functions.h"

/*******************************************************************************/
/* Global Variable Declarations                                                */
/*******************************************************************************/
extern uint16_t         NWBufferSize;
extern volatile bool    adi_gNotifyNetworkMeta;
extern adi_wil_network_data_t   userNetworkBuffer[ADI_WIL_MAX_NODES * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];

bool bUpdateExpectedGenTime = TRUE;

/******************************************************************************
 * Static variable declarations
 *****************************************************************************/
static PSInfoType PerNodePSInfo[ADI_WIL_MAX_NODES];

/* Lookup table for Tx Attempts based on Latency */
const TxAttemptsLookupType TxAttemptsLookup[TX_ATTEMPTS_LOOKUP_SIZE] =
{
    //Min  Max    TxAttempts
    {0,     23,     3},
    {24,    29,     4},
    {30,    35,     5},
    {36,    42,     6},
    {43,    48,     7},
    {49,    0xFFFF, 8},
};

/*******************************************************************************
 * Local Funtion Declarations
 ******************************************************************************/
static void adi_wil_example_EstimatePSFromLatency(uint8_t NodeID);

/*******************************************************************************
 * Funtion definitions
 ******************************************************************************/
void adi_task_ProcNWData(void)
{
    if(adi_gNotifyNetworkMeta == true)
    {
        adi_wil_example_StoreNWDataForPS(userNetworkBuffer);
        adi_gNotifyNetworkMeta = false;
    }
}

uint8_t findBitPosition(adi_wil_device_t iDeviceId)
{
    uint64_t i = 1; 
    uint8_t pos = 0;

    /* Iterate through bits of iDeviceId until we find a set bit */
    while (!(i & iDeviceId)) {
        /* Unset current bit and set the next bit in 'i' */
        i = i << 1;
        /* increment position */
        ++pos;
    }

    return pos;
}

/******************************************************************************
 * Function to store NW data which will be further used to estimate Path Stability
 *****************************************************************************/
void adi_wil_example_StoreNWDataForPS(adi_wil_network_data_t* NWData)
{   
    uint8_t     NodeID              = 0;
    uint16_t    Indx                = 0;
    int64_t     TimeDiff            = 0;
    uint64_t    CurrPacketGenTime   = 0;

    /* Check if you need to update the expected Packet Generation Time.
       This should happen every time you enter Active Mode */
    if(bUpdateExpectedGenTime == TRUE)
    {
        /* Check if atleast 2/3 of total NW data packets have same packet generation time. If so we can store this as the 
           Expected Generation Time for all Nodes.
           For this we use a Hash Map. Every time we receive packet with same time, we update its count in the Hash Map */

        uint8_t  TotalCountSamePGT  = 0;    /*PGT - Packet Generation Time*/
        uint8_t  HashMapIndx        = 0;
        uint8_t  HashMapSamePGT[TOTAL_SLOTS_PER_INTERVAL];  

        memset(HashMapSamePGT, 0, TOTAL_SLOTS_PER_INTERVAL);

        for(Indx = 0; Indx < NWBufferSize; Indx++)
        {
            HashMapIndx = userNetworkBuffer[Indx].iPacketGenerationTime % TOTAL_SLOTS_PER_INTERVAL;
            HashMapSamePGT[HashMapIndx]++;

            TotalCountSamePGT = HashMapSamePGT[HashMapIndx];

            if(HashMapIndx > 0)
            {
                TotalCountSamePGT += HashMapSamePGT[HashMapIndx-1];
            }

            if(TotalCountSamePGT >= ((NWBufferSize*2)/3))
            {
                for(int i = 0; i < ADI_WIL_MAX_NODES; i++)
                {
                    PerNodePSInfo[i].ExpectedPacketGenTime = userNetworkBuffer[Indx].iPacketGenerationTime + TOTAL_SLOTS_PER_INTERVAL;
                }

                bUpdateExpectedGenTime = FALSE;
            }
        }
    }
    /* Expected Packet Generation Time is updated, so now check if packets are received at expected time */
    else
    {
        for(Indx = 0; Indx < NWBufferSize; Indx++)
        {
            NodeID                  = findBitPosition(userNetworkBuffer[Indx].eSrcDeviceId);
            CurrPacketGenTime       = userNetworkBuffer[Indx].iPacketGenerationTime;

            TimeDiff = CurrPacketGenTime - PerNodePSInfo[NodeID].ExpectedPacketGenTime;

            /* Check if packet received belongs to same measurement interval. 
               Handle for +/-1 offsets in packet generation time */
            if( (-1 <= TimeDiff) && (TimeDiff <= 1) )
            {
                /* Check if there is space left to store this NW data */
                if(PerNodePSInfo[NodeID].NwDataCount < ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL)
                {
                    memcpy( &PerNodePSInfo[NodeID].NWData[PerNodePSInfo[NodeID].NwDataCount++], 
                            &userNetworkBuffer[Indx], 
                            sizeof(adi_wil_network_data_t) );

                    /* Check if you received all packets*/
                    if(PerNodePSInfo[NodeID].NwDataCount == ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL)
                    {
                        /* Analyze Latency Data */
                        adi_wil_example_EstimatePSFromLatency(NodeID);

                        PerNodePSInfo[NodeID].NwDataCount           = 0;
                        PerNodePSInfo[NodeID].ExpectedPacketGenTime = CurrPacketGenTime + TOTAL_SLOTS_PER_INTERVAL; 
                    }
                }
                else
                {
                    /* If buffer is full and you still receive data with same generation time, then this is the 
                       Health Report packet or duplicate packet and you can ignore this */
                }
            }

            /* Check if packet received belongs to next measurement inerval */
            if( ((TOTAL_SLOTS_PER_INTERVAL - 1) <= TimeDiff) && (TimeDiff <= (TOTAL_SLOTS_PER_INTERVAL + 1)) )
            {
                /* Analyze latency data of packets received till now */
                adi_wil_example_EstimatePSFromLatency(NodeID);

                PerNodePSInfo[NodeID].NwDataCount           = 0;
                PerNodePSInfo[NodeID].ExpectedPacketGenTime = CurrPacketGenTime;

                memcpy( &PerNodePSInfo[NodeID].NWData[PerNodePSInfo[NodeID].NwDataCount++], 
                        &userNetworkBuffer[Indx], 
                        sizeof(adi_wil_network_data_t) );
            }
        }
    }
}



/******************************************************************************
 * Example function to estimate path stability from Latency 
 *****************************************************************************/
static void adi_wil_example_EstimatePSFromLatency(uint8_t NodeID)
{
    uint16_t Indx            = 0;
    uint16_t WorstLatency    = 0;
    uint16_t TotalPcktsRcvd  = 0;
    uint16_t TotalTxAttempts = 0;
    

    TotalPcktsRcvd = PerNodePSInfo[NodeID].NwDataCount;

    /* Check if you received less than 3 packets */
    if(TotalPcktsRcvd < ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL)
    {
        /* Since you did not receive all packets, it means you utilized all Tx Attempts */
        TotalTxAttempts = MAX_TX_ATTEMPTS_PER_INTERVAL;
    }
    /* You received all 3 packets */
    else
    {
        /* Find worst latency from all the packets */
        for(Indx = 0; Indx < PerNodePSInfo[NodeID].NwDataCount; Indx++)
        {
            /* If latency is > 54 than this is a HR and not BMS packet. Decrement Total Packets Received */
            if( (PerNodePSInfo[NodeID].NWData[Indx].iLatency > TOTAL_SLOTS_PER_INTERVAL) )
            {
                TotalPcktsRcvd--;
            }
            /* Else update the worst latency */
            else if(PerNodePSInfo[NodeID].NWData[Indx].iLatency > WorstLatency)
            {
                WorstLatency = PerNodePSInfo[NodeID].NWData[Indx].iLatency;
            }
        }

        /* Identify total Tx Attempts for this worst latency from lookup */
        for(Indx = 0; Indx < TX_ATTEMPTS_LOOKUP_SIZE; Indx++)
        {
            if( (TxAttemptsLookup[Indx].Min <= WorstLatency) && (WorstLatency <= TxAttemptsLookup[Indx].Max) )
            {
                TotalTxAttempts = TxAttemptsLookup[Indx].TxAttempts;
                break;
            }
        }
    }

    /* Calculate Per Interval and Average Path Stability */
    PerNodePSInfo[NodeID].PerIntervalPS = ((float)TotalPcktsRcvd/(float)TotalTxAttempts) * 100;

    PerNodePSInfo[NodeID].TotalPcktsRcvd  += TotalPcktsRcvd;
    PerNodePSInfo[NodeID].TotalTxAttempts += TotalTxAttempts;

    PerNodePSInfo[NodeID].AvgPS = ((float)PerNodePSInfo[NodeID].TotalPcktsRcvd/(float)PerNodePSInfo[NodeID].TotalTxAttempts) * 100; 

    // printf("PS_%d : %0.2f\tAvgPS_%d : %0.2f\tLatency_%d : %d\r\n", NodeID, PerNodePSInfo[NodeID].PerIntervalPS,
    //                                                                NodeID, PerNodePSInfo[NodeID].AvgPS,
    //                                                                NodeID, WorstLatency );
}

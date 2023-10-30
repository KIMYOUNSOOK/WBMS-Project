/*******************************************************************************
 * @file adi_wil_example_PSFromLatency.h
 *
 * Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef ADI_WIL_EXAMPLE_PS_FROM_LATENCY_H
#define ADI_WIL_EXAMPLE_PS_FROM_LATENCY_H

#include <stdint.h>
#include <stdbool.h>

#include "adi_wil_types.h"
#include "adi_wil.h"
#include "adi_wil_example_config.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

#define TX_ATTEMPTS_LOOKUP_SIZE                 6u
#define TOTAL_SLOTS_PER_INTERVAL                54u
#define MAX_TX_ATTEMPTS_PER_INTERVAL            8u

/*******************************************************************************
 * Structures
 *******************************************************************************/
typedef struct
{
    uint16_t    NwDataCount;
    uint64_t    ExpectedPacketGenTime;
    adi_wil_network_data_t NWData[ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
    
    float       PerIntervalPS;
    
    float       AvgPS;
    uint64_t    TotalPcktsRcvd;
    uint64_t    TotalTxAttempts;
}PSInfoType;

typedef struct
{
    uint16_t Min;
    uint16_t Max;
    uint16_t TxAttempts;
}TxAttemptsLookupType;


/*******************************************************************************
 * Functions
 *******************************************************************************/
extern void adi_task_ProcNWData(void);
extern void adi_wil_example_StoreNWDataForPS(adi_wil_network_data_t* NWData);


#endif /* ADI_WIL_EXAMPLE_PS_FROM_LATENCY_H */

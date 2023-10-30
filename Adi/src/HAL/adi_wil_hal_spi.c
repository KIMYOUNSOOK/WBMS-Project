/*******************************************************************************
 * @brief    HAL SPI layer
 *
 * @details  Implements low-level SPI operations required by WIL
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include "adi_wil_hal.h"
#include "adi_wil_hal_spi.h"
#include "IfxQspi_SpiMaster.h"
#include "IfxPort.h"
#include "adi_wil_example_isr_priorities.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_functions.h"
#include "string.h"

#include "adi_wil_example_debug_functions.h"

#if     (ADK_SPI_0 == 4 )
#define SPI0_MASTER                 &MODULE_QSPI4               /* Hardware module for SPI 0 */
#else
#define SPI0_MASTER                 &MODULE_QSPI1               /* Hardware module for SPI 0 */
#endif

#if(ADK_SPI_1 == 0)
#define SPI1_MASTER                 &MODULE_QSPI0               /* Hardware module for SPI 1 */
#else
#define SPI1_MASTER                 &MODULE_QSPI3               /* Hardware module for SPI 1 */
#endif

#define ISR_PROVIDER                IfxSrc_Tos_cpu0             /* Define the QSPI interrupt provider */

/* SPI protocol timings per WIL Integration Guide
 * Delays must be defined in units of half SCLK cycle => 0.5us for SCLK speed of 1MHz */
#define SPI_SCLK                    1000000u                    /* Master channel SCLK speed 1Mhz : default             */
#define WAIT_DELAY                  0u                          /* Delay between SPI transactions (200us/1Mhz)  */
#define LEAD_DELAY                  2u                          /* Lead delay for SPI transaction (1us)    */
#define TRAIL_DELAY                 2u                          /* Trail delay for SPI transaction (1us)   */
#define BIG_ENDIAN_SWAP             2u                          /* Special register value for permute enable
                                                                   register, enables hardware-level swap of
                                                                   bytes to/from 32-bit big-endian format */

bool G_SPI_0_DMA_break = false;
bool G_SPI_1_DMA_break = false;

typedef struct {
    IfxQspi_SpiMaster               spiMaster;                  /* QSPI Master handle         */
    IfxQspi_SpiMaster_Channel       spiMasterChannel;           /* QSPI Master Channel handle */
    adi_wil_hal_spi_cb_t            cbPtr;                      /* SPI callback function ptr  */
} spi_port_state_t;

typedef struct {
    IfxQspi_SpiMaster_Pins          portPins;                   /* Port pins                         */
    IfxQspi_SpiMaster_Output        csPin;                      /* Chip Select pin                   */
    IfxDma_ChannelId                txDMACh;                    /* Tx DMA Channel No.                */
    IfxDma_ChannelId                rxDMACh;                    /* Rx DMA Channel No.                */
    uint8                           txPrio;                     /* Tx DMA Channel Interrupt Priority */
    uint8                           rxPrio;                     /* Rx DMA Channel Interrupt Priority */
    uint8                           erPrio;                     /* Er DMA Channel Interrupt Priority */
    IfxQspi_ChannelId               channelId;                  /* The transfer channel used         */
} spi_port_config_t;


/* A port instance for each port needed */
static spi_port_state_t spiPort[PORT_COUNT];

/* Array of configuration data, one element for each port which will be instantiated */
const spi_port_config_t spiPortConfig[PORT_COUNT] = {
    /* SPI 0 */
    {
        .portPins = {
            #if(ADK_SPI_0 == 4)
            &IfxQspi4_SCLK_P22_3_OUT, IfxPort_OutputMode_pushPull,              /* SCLK Pin                          (CLK)  */
            &IfxQspi4_MTSR_P22_0_OUT, IfxPort_OutputMode_pushPull,              /* Master Transmit Slave Receive Pin (MOSI) */
            &IfxQspi4_MRSTB_P22_1_IN, IfxPort_InputMode_noPullDevice,           /* Master Receive Slave Transmit Pin (MISO) */
            #else
            &IfxQspi1_SCLK_P10_2_OUT, IfxPort_OutputMode_pushPull,              /* SCLK Pin                          (CLK)  */
            &IfxQspi1_MTSR_P10_3_OUT, IfxPort_OutputMode_pushPull,              /* Master Transmit Slave Receive Pin (MOSI) */
            &IfxQspi1_MRSTA_P10_1_IN, IfxPort_InputMode_pullDown,               /* Master Receive Slave Transmit Pin (MISO) */
            #endif
            IfxPort_PadDriver_cmosAutomotiveSpeed3                              /* Pad driver mode                          */
        },
        .csPin = {
            #if(ADK_SPI_0 == 4)
            &IfxQspi4_SLSO3_P22_2_OUT, IfxPort_OutputMode_pushPull,             /* Slave Select Pin (CS)                    */
            #else
            &IfxQspi1_SLSO10_P10_0_OUT, IfxPort_OutputMode_pushPull,            /* Slave Select Pin (CS)                    */
            #endif
            IfxPort_PadDriver_cmosAutomotiveSpeed1                              /* Pad driver mode                          */
        },
        .txDMACh = IfxDma_ChannelId_1,
        .rxDMACh = IfxDma_ChannelId_2,
        .txPrio  = ISR_PRIORITY_DMA_CH1,
        .rxPrio  = ISR_PRIORITY_DMA_CH2,
        #if(ADK_SPI_0 == 4)
        .erPrio  = ISR_PRIORITY_QSPI4_ER,
        .channelId = IfxQspi_ChannelId_3,                                       /* SPI 0 uses SLSO3 therefore channel is 3 */
        #else
        .erPrio  = ISR_PRIORITY_QSPI1_ER,
        .channelId = IfxQspi_ChannelId_10,                                      /* SPI 0 uses SLS10 therefore channel is 10 */
        #endif
    },
    /* SPI 1 */
    {
        .portPins = {
            #if(ADK_SPI_1 == 0)
            #if(!ADK_SPI_P20)
            &IfxQspi0_SCLK_P22_7_OUT, IfxPort_OutputMode_pushPull,              /* SCLK Pin                          (CLK)  */
            &IfxQspi0_MTSR_P22_5_OUT, IfxPort_OutputMode_pushPull,              /* Master Transmit Slave Receive Pin (MOSI) */
            &IfxQspi0_MRSTC_P22_6_IN, IfxPort_InputMode_noPullDevice,           /* Master Receive Slave Transmit Pin (MISO) */
            #else
            &IfxQspi0_SCLK_P20_11_OUT, IfxPort_OutputMode_pushPull,             /* SCLK Pin                          (CLK)  */
            &IfxQspi0_MTSR_P20_14_OUT, IfxPort_OutputMode_pushPull,             /* Master Transmit Slave Receive Pin (MOSI) */
            &IfxQspi0_MRSTA_P20_12_IN, IfxPort_InputMode_pullDown,              /* Master Receive Slave Transmit Pin (MISO) */
            #endif
            #else
            &IfxQspi3_SCLK_P02_7_OUT, IfxPort_OutputMode_pushPull,              /* SCLK Pin                          (CLK)  */
            &IfxQspi3_MTSR_P02_6_OUT, IfxPort_OutputMode_pushPull,              /* Master Transmit Slave Receive Pin (MOSI) */
            &IfxQspi3_MRSTA_P02_5_IN, IfxPort_InputMode_pullDown,               /* Master Receive Slave Transmit Pin (MISO) */
            #endif
            IfxPort_PadDriver_cmosAutomotiveSpeed3
        },
        .csPin = {
            #if(ADK_SPI_1 == 0)
            #if(!ADK_SPI_P20)
            &IfxQspi0_SLSO12_P22_4_OUT, IfxPort_OutputMode_pushPull,            /* Slave Select Pin (CS)  */
            #else
            &IfxQspi0_SLSO2_P20_13_OUT, IfxPort_OutputMode_pushPull,            /* Slave Select Pin (CS)  */
            #endif
            #else
            &IfxQspi3_SLSO0_P02_4_OUT, IfxPort_OutputMode_pushPull,             /* Slave Select Pin (CS)  */
            #endif            
            IfxPort_PadDriver_cmosAutomotiveSpeed1                              /* Pad driver mode       */
        },
        .txDMACh = IfxDma_ChannelId_3,
        .rxDMACh = IfxDma_ChannelId_4,
        .txPrio  = ISR_PRIORITY_DMA_CH3,
        .rxPrio  = ISR_PRIORITY_DMA_CH4,
        #if(ADK_SPI_1 == 0)
        .erPrio  = ISR_PRIORITY_QSPI0_ER,
        .channelId = IfxQspi_ChannelId_12,                                      /* SPI 1 uses SLSO12 therefore channel is 12 */
        #else
        .erPrio  = ISR_PRIORITY_QSPI3_ER,
        .channelId = IfxQspi_ChannelId_0,                                       /* SPI 1 uses SLSO0 therefore channel is 0 */
        #endif
    }
};


#if(ADK_SPI_0 == 4)
IFX_INTERRUPT(QSPI4ErrorISR, 0, ISR_PRIORITY_QSPI4_ER);         /* SPI Master Error Interrupt definition */
#else
IFX_INTERRUPT(QSPI1ErrorISR, 0, ISR_PRIORITY_QSPI1_ER);         /* SPI Master Error Interrupt definition */
#endif
#if(ADK_SPI_1 == 0)
IFX_INTERRUPT(QSPI0ErrorISR, 0, ISR_PRIORITY_QSPI0_ER);         /* SPI Slave Error Interrupt definition  */
#else
IFX_INTERRUPT(QSPI3ErrorISR, 0, ISR_PRIORITY_QSPI3_ER);         /* SPI Slave Error Interrupt definition  */
#endif
IFX_INTERRUPT(DMAChn1ISR, 0, ISR_PRIORITY_DMA_CH1);             /* DMA Channel 1 Interrupt definition    */
IFX_INTERRUPT(DMAChn2ISR, 0, ISR_PRIORITY_DMA_CH2);             /* DMA Channel 2 Interrupt definition    */
IFX_INTERRUPT(DMAChn3ISR, 0, ISR_PRIORITY_DMA_CH3);             /* DMA Channel 3 Interrupt definition    */
IFX_INTERRUPT(DMAChn4ISR, 0, ISR_PRIORITY_DMA_CH4);             /* DMA Channel 4 Interrupt definition    */


/* Handle SPI0 Error interrupt */
#if(ADK_SPI_0 == 4)
void QSPI4ErrorISR(void)
{
    IfxQspi_SpiMaster_isrError(&spiPort[0].spiMaster);
}
#else
void QSPI1ErrorISR(void)
{
    IfxQspi_SpiMaster_isrError(&spiPort[0].spiMaster);
}
#endif

/* Handle SPI1 Error interrupt */
#if(ADK_SPI_1 == 0)
void QSPI0ErrorISR(void)
{
    IfxQspi_SpiMaster_isrError(&spiPort[1].spiMaster);
}
#else
void QSPI3ErrorISR(void)
{
    IfxQspi_SpiMaster_isrError(&spiPort[1].spiMaster);
}
#endif

/* Handle DMA Ch 1 (SPI0 Tx) interrupt */
void DMAChn1ISR(void)
{
    IfxQspi_SpiMaster_isrDmaTransmit(&spiPort[0].spiMaster);
}


/* Handle DMA Ch 2 (SPI0 Rx) interrupt */
void DMAChn2ISR(void)
{
    IfxQspi_SpiMaster_isrDmaReceive(&spiPort[0].spiMaster);
    /* This ISR signifies the end of a SPI transaction (SPI 0), hence the callback func. is called */
    if (spiPort[0].cbPtr != (void *)0)
    {
        spiPort[0].cbPtr(PORT0_SPI_DEVICE, PORT0_CHIP_SELECT);
    }
    G_SPI_0_DMA_break = true;
}


/* Handle DMA Ch 3 (SPI1 Tx) interrupt */
void DMAChn3ISR(void)
{
    IfxQspi_SpiMaster_isrDmaTransmit(&spiPort[1].spiMaster);
}


/* Handle DMA Ch 4 (SPI1 Rx) interrupt */
void DMAChn4ISR(void)
{
    IfxQspi_SpiMaster_isrDmaReceive(&spiPort[1].spiMaster);
    /* This ISR signifies the end of a SPI transaction (SPI 1), hence the callback func. is called */
    if (spiPort[1].cbPtr != (void *)0)
    {
        spiPort[1].cbPtr(PORT1_SPI_DEVICE, PORT1_CHIP_SELECT);
    }
    G_SPI_1_DMA_break = true;
}


SpiIf_Status SpiInitHelper(Ifx_QSPI *qspi, spi_port_config_t const * pSpiPortConfig, spi_port_state_t * pSpiPort)
{
    IfxQspi_SpiMaster_Config spiMasterConfig;

    /* Use QSPI1 module for SPI Port 0 */
    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, qspi);

    /* Assign SPI pins */
    spiMasterConfig.pins = &pSpiPortConfig->portPins;

    /* Assign DMA channels */
    spiMasterConfig.dma.useDma = TRUE;
    spiMasterConfig.dma.txDmaChannelId = pSpiPortConfig->txDMACh;
    spiMasterConfig.dma.rxDmaChannelId = pSpiPortConfig->rxDMACh;

    /* Assign DMA interrupt priorities */
    spiMasterConfig.base.txPriority = pSpiPortConfig->txPrio;
    spiMasterConfig.base.rxPriority = pSpiPortConfig->rxPrio;
    spiMasterConfig.base.erPriority = pSpiPortConfig->erPrio;
    spiMasterConfig.base.isrProvider = ISR_PROVIDER;

    /* Initialize the QSPI Master module using the user configuration structure */
    IfxQspi_SpiMaster_initModule(&pSpiPort->spiMaster, &spiMasterConfig);

    /* Define the Master Channel Configuration Structure */
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig;

    /* Initialize structure with default values */
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, &pSpiPort->spiMaster);

    spiMasterChannelConfig.mode = IfxQspi_SpiMaster_Mode_shortContinuous;                               
    spiMasterChannelConfig.sls.output = pSpiPortConfig->csPin;                                          /* Assign CS pin for this channel                                  */
    spiMasterChannelConfig.base.baudrate = SPI_SCLK;                                                    /* Set SCLK frequency to 1 MHz                                     */
    spiMasterChannelConfig.base.mode.dataHeading = SpiIf_DataHeading_msbFirst;                          /* Transmit MSB first                                              */
    spiMasterChannelConfig.base.mode.shiftClock = SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;     /* Samples at SCLK leading edge (CPHA = 0)                         */
    spiMasterChannelConfig.base.mode.clockPolarity = SpiIf_ClockPolarity_idleLow;                       /* Clock idles low (CPOL = 0)                                      */
    spiMasterChannelConfig.base.mode.csLeadDelay = LEAD_DELAY;                                          /* Delay = SPI SCLK cycle                                          */
    spiMasterChannelConfig.base.mode.csTrailDelay = TRAIL_DELAY;                                        /* Delay = SPI SCLK cycle                                          */
    spiMasterChannelConfig.base.mode.csInactiveDelay = WAIT_DELAY;                                      /* Delay = 200us                                                   */
    spiMasterChannelConfig.channelBasedCs = IfxQspi_SpiMaster_ChannelBasedCs_disabled;                  /* CS signal remains low for entire frame                          */

    /* Initialize the QSPI Master channel using the user configuration */
    SpiIf_Status status = IfxQspi_SpiMaster_initChannel(&pSpiPort->spiMasterChannel, &spiMasterChannelConfig);

    return status;
}


adi_wil_hal_err_t adi_wil_hal_SpiInit(uint8 iSPIDevice, adi_wil_hal_spi_cb_t pfCb)
{
        SpiIf_Status status;

    adi_wil_hal_err_t rc = ADI_WIL_HAL_ERR_FAILURE;

        switch(iSPIDevice)
        {
            case PORT0_SPI_DEVICE:
                status = SpiInitHelper(SPI0_MASTER, &spiPortConfig[0], &spiPort[0]);
                rc = (status == SpiIf_Status_ok) ? ADI_WIL_HAL_ERR_SUCCESS : ADI_WIL_HAL_ERR_FAILURE;
                /* Save callback function pointer */
                spiPort[0].cbPtr = pfCb;
                break;

            case PORT1_SPI_DEVICE:
                status = SpiInitHelper(SPI1_MASTER, &spiPortConfig[1], &spiPort[1]);
                rc = (status == SpiIf_Status_ok) ? ADI_WIL_HAL_ERR_SUCCESS : ADI_WIL_HAL_ERR_FAILURE;
                /* Save callback function pointer */
                spiPort[1].cbPtr = pfCb;
                break;

            default:
                rc = ADI_WIL_HAL_ERR_FAILURE;
                break;
        }

    return rc;
}


adi_wil_hal_err_t adi_wil_hal_SpiTransmit(uint8 iSPIDevice,
                                          uint8 iChipSelect,
                                          uint8 * const pTx,
                                          uint8 * const pRx,
                                          uint16 iLength)
{
    SpiIf_Status status;
    adi_wil_hal_err_t rc = ADI_WIL_HAL_ERR_FAILURE;
    (void)iChipSelect; /* chip select not used as there is only one device on each port */

    switch(iSPIDevice)
    {
        case PORT0_SPI_DEVICE:
            status = IfxQspi_SpiMaster_exchange(&spiPort[0].spiMasterChannel, pTx, pRx, iLength);
            rc = (status == SpiIf_Status_ok) ? ADI_WIL_HAL_ERR_SUCCESS : ADI_WIL_HAL_ERR_FAILURE;

            break;

        case PORT1_SPI_DEVICE:
            status = IfxQspi_SpiMaster_exchange(&spiPort[1].spiMasterChannel, pTx, pRx, iLength);
            rc = (status == SpiIf_Status_ok) ? ADI_WIL_HAL_ERR_SUCCESS : ADI_WIL_HAL_ERR_FAILURE;
            break;

        default:
            rc = ADI_WIL_HAL_ERR_FAILURE;
            break;
    }

    return rc;
}

extern DISPLAYSTR ADK_DEMO;

adi_wil_hal_err_t adi_wil_hal_SpiClose(uint8 iSpiDevice)
{
    adi_wil_hal_err_t rc = ADI_WIL_HAL_ERR_FAILURE;

    switch(iSpiDevice)
    {
        case PORT0_SPI_DEVICE:
            if(ADK_DEMO.DEMO_MODE == KEY_OFF_EVENT){
                while((G_SPI_0_DMA_break == false)||(G_SPI_1_DMA_break == false));
            }
            // IfxQspi_setDisableModuleRequest(SPI0_MASTER);
            /* disable interrupts configured this SPI module as a precaution */
            IfxSrc_disable(IfxQspi_getErrorSrc(SPI0_MASTER));
            IfxSrc_disable(IfxQspi_getReceiveSrc(SPI0_MASTER));
            IfxSrc_disable(IfxQspi_getTransmitSrc(SPI0_MASTER));
            /**
             * The WIL no longer waits for an already in progress SPI operation
             * to complete before calling the HAL function adi_wil_hal_SpiClose().
             * Hence in adi_wil_hal_SpiClose() function, the user needs to make sure
             * that the SPI shutdown is correctly handled if a SPI transaction is 
             * already in progress - SPI needs to be halted and SPI transfer completion interrupt is disabled. 
             * In this case, iLLD resetChannel halts the current DMA transaction.
             */
            IfxDma_resetChannel(&MODULE_DMA, spiPortConfig[0].txDMACh);
            IfxDma_resetChannel(&MODULE_DMA, spiPortConfig[0].rxDMACh);
            /* clear the callback pointer */
            spiPort[0].cbPtr = (void *)0;
            rc = ADI_WIL_HAL_ERR_SUCCESS;
            break;

        case PORT1_SPI_DEVICE:
            if(ADK_DEMO.DEMO_MODE == KEY_OFF_EVENT){
                while((G_SPI_0_DMA_break == false)||(G_SPI_1_DMA_break == false));
            }
            // IfxQspi_setDisableModuleRequest(SPI1_MASTER);
            /* disable interrupts configured this SPI module as a precaution */
            IfxSrc_disable(IfxQspi_getErrorSrc(SPI1_MASTER));
            IfxSrc_disable(IfxQspi_getReceiveSrc(SPI1_MASTER));
            IfxSrc_disable(IfxQspi_getTransmitSrc(SPI1_MASTER));
            /* make sure to halts the current DMA transaction */
            IfxDma_resetChannel(&MODULE_DMA, spiPortConfig[1].txDMACh);
            IfxDma_resetChannel(&MODULE_DMA, spiPortConfig[1].rxDMACh);
            /* clear the callback pointer */
            spiPort[1].cbPtr = (void *)0;
            rc = ADI_WIL_HAL_ERR_SUCCESS;
            break;

        default:
            rc = ADI_WIL_HAL_ERR_FAILURE;
            break;
    }

    return rc;
}


#if 0
#define TOTAL_ITER_TO_RUN               10   // Number of times to run the SPI test
 
uint8_t Spi_TxData_0[256];
static uint8_t Spi_RxData_0[10][256];
uint8_t Spi_TxData_1[256];
static uint8_t Spi_RxData_1[10][256];
// static struct  wbms_cmd_resp_query_device_t QueryDeviceResp_0;
// static struct  wbms_cmd_resp_query_device_t QueryDeviceResp_1;
 
void Spi_TxCompleteCbk_0(uint8_t iSPIDevice, uint8_t iChipSelect)
{
    // if(Spi_RxData_0[4] == 0x7F)
    // {
    //     memcpy(&QueryDeviceResp_0, &Spi_RxData_0[6], sizeof(struct wbms_cmd_resp_query_device_t));
    // }
}
 
void Spi_TxCompleteCbk_1(uint8_t iSPIDevice, uint8_t iChipSelect)
{
    // if(Spi_RxData_1[4] == 0x7F)
    // {
    //     memcpy(&QueryDeviceResp_1, &Spi_RxData_1[6], sizeof(struct wbms_cmd_resp_query_device_t));
    // }
}
 
void Test_Spi_Hal_0(void)
{
    uint32_t delay = 0;
    int i = 0;

    adi_wil_hal_SpiInit(0, &Spi_TxCompleteCbk_0);
     
    memset(&Spi_TxData_0[0], 0, 256);
     
    Spi_TxData_0[0]   = 0x04;
    Spi_TxData_0[1]   = 0xFF;
    Spi_TxData_0[2]   = 0x00;
    Spi_TxData_0[3]   = 0x00;
    Spi_TxData_0[4]   = 0x7F;
    Spi_TxData_0[5]   = 0x02;
     
    Spi_TxData_0[252] = 0xD6;
    Spi_TxData_0[253] = 0xEC;
    Spi_TxData_0[254] = 0x8B;
    Spi_TxData_0[255] = 0x01;
     
    for( i = 0; i < TOTAL_ITER_TO_RUN; i++)
    {
        adi_wil_hal_SpiTransmit(0, 0, &Spi_TxData_0[0], &Spi_RxData_0[i][0], 256);
        
        for(delay=0; delay<1250000; delay++){
            //5ms
        }
    }
}

int test_cnt = 0;

void Test_Spi_Hal_1(void)
{
    uint32_t delay = 0;
    int i = 0;

    adi_wil_hal_SpiInit(1, &Spi_TxCompleteCbk_1);
     
    memset(&Spi_TxData_1[0], 0, 256);
     
    Spi_TxData_1[0]   = 0x04;
    Spi_TxData_1[1]   = 0xFF;
    Spi_TxData_1[2]   = 0x00;
    Spi_TxData_1[3]   = 0x00;
    Spi_TxData_1[4]   = 0x7F;
    Spi_TxData_1[5]   = 0x02;
     
    Spi_TxData_1[252] = 0xD6;
    Spi_TxData_1[253] = 0xEC;
    Spi_TxData_1[254] = 0x8B;
    Spi_TxData_1[255] = 0x01;
     
    for( i = 0; i < TOTAL_ITER_TO_RUN; i++)
    {
        adi_wil_hal_SpiTransmit(1, 0, &Spi_TxData_1[0], &Spi_RxData_1[i][0], 256);
        test_cnt++;
        for(delay=0; delay<1250000; delay++){
            //5ms
        }
    }
}

#endif

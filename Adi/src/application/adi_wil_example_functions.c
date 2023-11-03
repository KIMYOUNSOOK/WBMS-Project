/*******************************************************************************
 * @file     adi_platform_app_wil_functions.c
 *
 * @brief    Platform Application WIL support functions
 *
 * @details  Include wrapping WIL functionality.
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "adi_wil.h"
#include "adi_wil_api.h"
#include "adi_wil_osal.h"
#include "adi_bms_container.h"
#include "adi_wil_example_cfg_profiles.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_functions.h"
#include "adi_wil_example_printf.h"
#include "adi_wil_example_version.h"
#include "adi_wil_example_w2can.h"
#include "adi_bms_defs.h"
#include "adi_wil_hal_task.h"
#include "adi_bms_types.h"
#include "adi_wil_example_acl.h"
#include "adi_wil_types.h"
#include "adi_wil_example_owd.h"
#include "adi_wil_hal_task_cb.h"
#include "adi_wil_example_cell_balance.h"
#include "adi_wil_example_debug_functions.h"
#include "wb_rsp_query_device.h"
#include "adi_wil_app_interface.h"
#include "adi_wil_hal_spi.h"

#include "otap_node_opfw_220.h"
#include "otap_mngr_opfw_220.h"

#include "adi_wil_example_PSFromLatency.h"



/*******************************************************************************/
/* Embedded WIL Function Declarations                                          */
/*******************************************************************************/

extern void adi_wil_example_hal_spi_configure_speed(uint8_t api_port, uint32_t speed);
extern void WaitForWilAPI(adi_wil_pack_t * const pPack);
extern void adi_task_bmsDataRetrieval(void);
extern uint32_t adk_debug_TickerBTGetTimestamp(void);
extern uint32_t adi_wil_hal_TickerGetTimestamp(void);

extern void Adbms683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt2(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Cell_OWD(adi_wil_device_t eNode);
extern void adk_debug_BootTimeLog(bool final, bool start, uint16_t step, ADK_LOG_FUNCTION api);
void adk_debug_log_GetDeviceVersion(ADK_OTAP_ARG_0 eDeviceID, ADK_OTAP_ARG_1 update, uint8_t idx);
void adk_debug_Report(ADK_FAIL_API_NAME api, adi_wil_err_t rc );
/*******************************************************************************/
/* Global Variable Declarations                                                */
/*******************************************************************************/
extern bool adi_gNotifyBmsData;
extern bool adi_gNotifyBms;
extern bool adi_gNotifyPms;
extern bool adi_gNotifyEms;
extern bool adi_gNotifyNetworkMeta;
extern adi_wil_network_status_t networkStatus;
extern bool bUpdateExpectedGenTime;
extern bool gQueryDeviceRetry;     /*  @remark : request by sure-soft */
extern volatile adi_wil_err_t adi_gProcessTaskErrorCode;

extern uint32_t adi_gW2canNodeCount;
extern const uint8_t bms_container_file;
extern const uint8_t pms_container_file;
extern adi_wil_pack_t packInstance;
extern uint8_t  iOWDPcktsRcvd[ADI_WIL_MAX_NODES];
extern adi_wil_device_removed_t LF_NG_LIST[12];
extern ADK_DEVICE_VER OTAP_DEVICE_VER;
extern uint16_t G_BOOT_TIME_CNT;

bool adi_gFaultDetected;
bool CB_EVEN_DCC_RECEIVED = false;
bool CB_ODD_DCC_RECEIVED = false;
bool CB_DEFAULT_DCC_RECEIVED = false;
bool BASE_PACKET_RECEIVED = false;
bool ACL_EMPTY = false;
bool KEY_ON = false;
bool KEY_OFF = false;
bool bFirstBMSdata = false;
bool gOTAPQueryDeviceRetry = false;
bool ADK_NodeState[64] = {false,};
bool FLG_pdr_initial = false;
bool iDisconn_node[ADK_MAX_node] = {false,};
/* Flag, set to true to indicate queryDevice callback has been called */
bool volatile queryDeviceCallbackCalled = false;

uint8_t iPacketLostData = 0;
uint32_t TEMP_BUFFER_ZERO[TEST_BUFFER_MAX] = {0,};
uint8_t recv_confirm[ADK_MAX_node] = {0,};

uint8_t pkt_valid_index[ADK_MAX_node];
uint8_t FLG_pdr_seq_initial[ADK_MAX_node];
uint8_t FLG_seq_num_rollback_period[ADK_MAX_node];
uint32_t pkt_seq_buffer[ADK_MAX_node][PDR_pktbuf_depth];
uint32_t pkt_valid_seq[ADK_MAX_node];
uint64_t accm_PDR_OK[ADK_MAX_node];
uint64_t accm_PDR_TOTAL[ADK_MAX_node];

int32_t accm_rssi_rx_0 = 0;
int32_t accm_rssi_rx_1 = 0;
int32_t accm_pkt_num_0 = 0;
int32_t accm_pkt_num_1 = 0;
int64_t temp_rssi_rx[ADK_MAX_node][2] = {{0,}, {0,}};
int64_t temp_count[ADK_MAX_node][2] = {{0,}, {0,}};

uint16_t Compare_depth = 0;
uint16_t Compare_cnt = 0;
uint32_t User_ACL[64] = {0,}; // @remark  max noode
uint32_t Read_ACL[64] = {0,}; // @remark  max noode
uint16_t User_ACL_cnt = 0;


/* Storage for the rc passed into queryDevice callback; allows application to read result of queryDevice call. */
adi_wil_err_t volatile queryDeviceRc = ADI_WIL_ERR_FAIL;
/* Pointer to storage for configuration information passed into queryDevice callback. */
adi_wil_configuration_t * pQueryDeviceConfig = (void*)0;

adi_wil_network_data_t   userNetworkBuffer[ADI_WIL_MAX_NODES * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
adi_wil_err_t gNotifRc;
unsigned int temp_BMS=0;
uint8_t EventCallback_pkt_count = 0;
uint32_t NetworkStatusTmr = 0;      /* Support Network Status Timer */

/* the wil *does* write to the internals element of the port_t so this needs to be static/global/long-lived */
adi_wil_port_t PortManager0 = {
    .iSPIDevice = 0,  /* we expect manager 0 to be available via SPI0 */
    .iChipSelect = 0,
    // .Internals = { 0 },
};

adi_wil_port_t PortManager1 = {
    .iSPIDevice = 1, /* we expect manager 1 to be available via SPI1 */
    .iChipSelect = 0,
    // .Internals = { 0 },
};

adi_wil_api_t  logAPIInProgress;
volatile uint8_t iMgrConnectCount = 0, iMgrDisconnectCount = 0;
uint16_t  nTotalPcktsRcvd = 0u;     /*  @remark : Variable to store total no. of bms packets received */

adi_wil_sensor_data_t           userBMSBuffer[BMS_DATA_PACKET_COUNT];
adi_wil_health_report0_t        userHR0Buffer[ADI_WIL_MAX_NODES];
adi_wil_health_report1_t        userHR1Buffer[ADI_WIL_MAX_NODES];
adi_wil_health_report2_t        userHR2Buffer[ADI_WIL_MAX_NODES];
adi_wil_health_report10_t       userHR10Buffer[NUM_OF_MANAGERS];
adi_wil_health_report11_t       userHR11Buffer[NUM_OF_MANAGERS];
adi_wil_health_report12_t       userHR12Buffer[NUM_OF_MANAGERS];
adi_wil_health_report14_t       userHR14Buffer[NUM_OF_MANAGERS];
adi_wil_health_report14_t       userHR15Buffer[NUM_OF_MANAGERS];
adi_wil_health_report14_t       userHR16Buffer[NUM_OF_MANAGERS];
adi_wil_health_report14_t       userHR17Buffer[NUM_OF_MANAGERS];
adi_wil_health_report14_t       userHR18Buffer[NUM_OF_MANAGERS];
adi_wil_health_report80_t       userHR80Buffer[ADI_WIL_MAX_NODES];
adi_wil_device_t                faultSources;
adi_wil_fault_report_t          faultReport;
adi_wil_file_crc_list_t         crclist;
adi_wil_sensor_data_t SensorDataArray[BMS_DATA_PACKET_COUNT];
adi_wil_pack_internals_t Internals;
adi_wil_safety_internals_t SafetyInternals;
adi_wil_connection_details_t tConnectionDetails;
client_data_t ClientData;
adi_wil_device_removed_t LF_NG_LIST[MAX_LF_RETRY];

#ifdef DBG_INIT_SCRIPT_TEST
uint8_t iInitBuffer0index = 0;
uint8_t iInitBuffer1index = 0;
uint8_t iInitBuffer2index = 0;
uint8_t iLostPacketCnt = 0;                             /* case of 0 iLength (packet lost case) */
adi_bms_init_pkt_0_t initBuffer0[ADK_MAX_node];
adi_bms_init_pkt_1_t initBuffer1[ADK_MAX_node];
adi_bms_init_pkt_2_t initBuffer2[ADK_MAX_node];
#endif

#ifdef DBG_GET_FILE_TEST
adi_wil_file_t getFileInformation;
uint8_t userGetFileContainer[GET_FILE_BUFFER_COUNT];
#endif

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
int16_t temp_buffer[20] = {0,};
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
int16_t temp_buffer[22] = {0,};
#else   /* Not supported */
#endif

#ifdef DBG_TEMP_TEST
uint32_t TEMP_ZERO[4] = {0,};
float TEMP_V2_MAX = -100;
float TEMP_V2_MIN = 100;
float TEMP_V3_MAX = -100;
float TEMP_V3_MIN = 100;
#endif 

#ifdef DBG_PACKET_ID_TEST
static uint8_t index = 0;
uint32_t TEMP_PACKET_ID[50] = {0,};
uint32_t TEMP_DEVICE_ID[50] = {0,};
#endif

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
extern bool adi_gNotifyLatent0;
extern bool adi_gNotifyLatent1;
// extern bool adi_gSelectLatent0;
// extern bool adi_gSelectLatent1;

adi_bms_latent0_pkt_0_t Latency0_Packet_0;
adi_bms_latent0_pkt_1_t Latency0_Packet_1;
adi_bms_latent0_pkt_2_t Latency0_Packet_2;
adi_bms_latent1_pkt_0_t Latency1_Packet_0;
adi_bms_latent1_pkt_1_t Latency1_Packet_1;

uint8_t latent0_recv_confirm[ADK_MAX_node] = {0,};
uint8_t latent1_recv_confirm[ADK_MAX_node] = {0,};
#else
#endif

/******************************************************************************
 * Static variable declarations
 *****************************************************************************/
static adi_wil_configuration_t      *pConfiguration;
static adi_wil_acl_t                systemAcl;


static adi_wil_port_t              portArray[PORT_COUNT] = {
    [0]= {
        .iSPIDevice = PORT0_SPI_DEVICE,
        .iChipSelect = PORT0_CHIP_SELECT,
        // .Internals = { 0 }, /* for zero warning */
    },
    [1] = {
        .iSPIDevice = PORT1_SPI_DEVICE,
        .iChipSelect = PORT1_CHIP_SELECT,
        // .Internals = { 0 }, /* for zero warning */
    }
};

#ifdef _ADI_ONLY
static adi_wil_contextual_data_t    returnedContextualData;
static adi_wil_gpio_value_t         returnedGpioValue;
static uint8_t                      returnedSoHValue;

static volatile uint32_t      iPMSNotificationCount = 0u;
static volatile uint32_t      iEMSNotificationCount = 0u;
static volatile uint32_t      iQueueOverflowNotificationCount = 0u;
static volatile uint32_t      iNodeConnectedCount = 0u;
static volatile uint32_t      iNodeDisconnectedCount = 0u;
static volatile uint32_t      iMgrToMgrErrCount = 0u;
static volatile uint32_t      iSecurityEvtCount = 0u;


static adi_wil_sensor_data_t    userPMSBuffer[PMS_DATA_PACKET_COUNT];
static adi_wil_sensor_data_t    userEMSBuffer[EMS_DATA_PACKET_COUNT];
#else
#endif

static volatile uint32_t      iBMSNotificationCount = 0u;
static volatile bool          bConnectApiCalled = false;
static adi_wil_port_t PortA, PortB;

#ifdef DBG_GET_FILE_TEST
static uint16_t     getfile_index = 0;
#endif

/******************************************************************************
 * Custom structure
 *****************************************************************************/
DISPLAYSTR ADK_DEMO;
extern BOOTTIMESTR BOOT_TIME;

/******************************************************************************
 * Static function declaration
 *****************************************************************************/

/******************************************************************************/
/* Local Funtion Declarations                                                 */
/******************************************************************************/
void adi_wil_example_ADK_readBms(void);
static void adi_wil_example_ADK_PDR_calc(void);
static void adi_wil_example_ADK_RSSI_calc(void);

// static void adi_wil_example_ADK_BGRSSI_calc(void);

adi_wil_err_t adi_wil_example_RetrySetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode);
static void adi_task_serviceProcessTask(void);
bool adi_wil_example_PeriodicallyCallProcessTask(void);
bool adi_wil_example_PeriodicallyCallProcessTaskCB(void);
static void adi_task_serviceProcessTaskCB(void);

/*******************************************************************************/
/* Local Variable Declarations                                                 */
/*******************************************************************************/

/******************************************************************************
 * Functions exercising the WIL APIs
 *****************************************************************************/

/******************************************************************************
 * Example function using adi_wil_Initiialize.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteInitialize(void)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    /* Initialize the WIL */
    if ((errorCode = adi_wil_Initialize()) != ADI_WIL_ERR_SUCCESS)
    {
        adk_debug_Report(DBG_wil_Initialize, errorCode);
    }
    else
    {
        /* Success */
    }
    return errorCode;
}

adi_wil_err_t Cmic_ExecuteInitialize(void)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
	
    /* Initialize the WIL */
    errorCode = adi_wil_Initialize();

	return errorCode;	
}


/******************************************************************************
 * Example function using adi_wil_Terminate.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteTerminate(void)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    if ((errorCode = adi_wil_Terminate()) != ADI_WIL_ERR_SUCCESS)
    {
        adk_debug_Report(DBG_wil_Terminate, errorCode);
    }
    else
    {
        //adi_wil_ex_info("WIL termination successful!");
    }

    return errorCode;
}

/******************************************************************************
 * Example function using adi_wil_QueryDevice.
 * This example shows the API being called on both SPI ports and using the
 * results from the call to determine whether the 2 managers on the 2 ports
 * are in dual manager mode.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteQueryDevice(adi_wil_configuration_t* pPortConfig)
{
    /* Dummy pack instance */
    adi_wil_pack_t *pPackLock;
    (void)memset(&pPackLock, 0xFF, sizeof(void*));

    for(int i = 0; i < PORT_COUNT; i++)
    {
        pConfiguration = &pPortConfig[i];

        returnOnWilError(adi_wil_QueryDevice(&portArray[i]));
        
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPackLock);

        if (gNotifRc != ADI_WIL_ERR_SUCCESS)
        {
            // adk_debug_Report(DBG_wil_QueryDevice, gNotifRc); /* Comment due to retry */
            return gNotifRc;
        }
        else
        {
            gQueryDeviceRetry = true;
            /* Success */
        }
    }
    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_RequestQueryDevice(uint8 portCnt)
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;


	errorCode = adi_wil_QueryDevice(&portArray[portCnt]);
        
	return errorCode;
}


/******************************************************************************
 * API callback function definition for QueryDevice.
 *****************************************************************************/
#ifdef _ADI_ONLY

void adi_wil_HandlePortCallback (adi_wil_port_t const * const pPort,
                                 adi_wil_api_t eAPI,
                                 adi_wil_err_t rc,
                                 void const * const pData)
{
    queryDeviceCallbackCalled = true;
    queryDeviceRc = rc;

    // if(ADK_DEMO.OTAP_STAT == Demo_OTAP_2_Confirm_Manager_FPA_______)
    // {

    //     if (rc == ADI_WIL_ERR_SUCCESS)
    //     {
    //         adi_wil_configuration_t const * const pConfig = (adi_wil_configuration_t const *)pData;
    //         /* If somehow destination pointer is still null, don't do the memcpy */
    //         if (pQueryDeviceConfig != (void *)0)
    //         {
    //             memcpy(pQueryDeviceConfig, pConfig, sizeof(adi_wil_configuration_t));
    //         }
    //     }

    // }
    // else
    {
        if ((rc == ADI_WIL_ERR_SUCCESS) && (pData != (void*)0) && (pConfiguration != (void*)0))
        {
            memcpy(pConfiguration, pData, sizeof(adi_wil_configuration_t));
        }
    }

    gNotifRc = rc;
}
#endif
/******************************************************************************
 * Example function using adi_wil_Connect.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteConnect(adi_wil_pack_t * const pPack,
                                             adi_wil_sensor_data_t *SensorData, uint8_t sysSensorPktCnt)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    // static adi_wil_port_t *pMgr1, *pMgr2;
    uint8_t mgrConnectionCount = 0;
    iMgrConnectCount = 0; 

    PortA.iSPIDevice = 0;
    PortA.iChipSelect = 0;
    PortB.iSPIDevice = 1;
    PortB.iChipSelect = 0;

    pPack->pInternals = &Internals;
    pPack->pSafetyInternals = &SafetyInternals;
    pPack->pDataBuffer = SensorData;
    pPack->pManager0Port = &PortA;
    pPack->pManager1Port = &PortB;
    pPack->iDataBufferCount = sysSensorPktCnt;
    pPack->pClientData = &ClientData;

    mgrConnectionCount = 2;
    // pMgr1 = &PortA;
    // pMgr2 = &PortB;

    /* Save the API function pointer */
    logAPIInProgress = ADI_WIL_API_CONNECT;

    /* Connect to the system */
    errorCode = adi_wil_Connect(pPack);

    if (errorCode != ADI_WIL_ERR_SUCCESS) {
        adk_debug_Report(DBG_wil_Connect, errorCode);

        return errorCode;
    } else {
                
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);
        
        errorCode = gNotifRc;        
    }
    return errorCode;
}

adi_wil_err_t Cmic_RequestConnect(adi_wil_pack_t * const pPack,
                                          adi_wil_sensor_data_t *SensorData, uint8_t sysSensorPktCnt)
{
 	
    PortA.iSPIDevice = 0;
    PortA.iChipSelect = 0;
    PortB.iSPIDevice = 1;
    PortB.iChipSelect = 0;

    pPack->pInternals = &Internals;
    pPack->pSafetyInternals = &SafetyInternals;
    pPack->pDataBuffer = SensorData;
    pPack->pManager0Port = &PortA;
    pPack->pManager1Port = &PortB;
    pPack->iDataBufferCount = sysSensorPktCnt;
    pPack->pClientData = &ClientData;

    return (adi_wil_Connect(pPack));
}


/******************************************************************************
 * Example function using adi_wil_Disconnect.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteDisconnect(adi_wil_pack_t * const pPack)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    //adi_wil_ex_info("Intiating disconnect from pack...");

    /* Disconnect the pack from the system. This is a blocking call so no API
       callback is generated. */
    errorCode = adi_wil_Disconnect(pPack);

    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        adk_debug_Report(DBG_wil_Disconnect, errorCode);
    }
    else
    {
        //adi_wil_ex_info("Disconnect from pack successful!");
    }

    return errorCode;
}
adi_wil_err_t Cmic_ExecuteDisconnect(adi_wil_pack_t * const pPack)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    errorCode = adi_wil_Disconnect(pPack);

    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        adk_debug_Report(DBG_wil_Disconnect, errorCode);
    }

    return errorCode;
}


/******************************************************************************
 * Helper function to adi_wil_SetMode.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_RetrySetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode){
    uint8_t setModeRetries = 0;
    while((setModeRetries < ADI_WIL_SET_MODE_MAX_RETRIES) && (gNotifRc == ADI_WIL_ERR_PARTIAL_SUCCESS)){
        returnOnWilError(adi_wil_SetMode(pPack, mode));
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);
    }
    return gNotifRc;
}

/******************************************************************************
 * Example function using adi_wil_SetMode.
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteSetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode) {

    adi_wil_mode_t currentMode;

    /*****************************Get Network Mode*****************************/
    //adi_wil_ex_info("Retrieving network mode...");
    returnOnWilError(adi_wil_example_ExecuteGetMode(pPack, &currentMode));

    if(currentMode == mode) {
        //adi_wil_ex_info("Network Mode already in desired state = %s", adi_wil_SystemModeToString(currentMode));
        return ADI_WIL_ERR_SUCCESS;
    }
    else {
        if(currentMode == ADI_WIL_MODE_OTAP) {
            ADK_DEMO.OTAP_RE_UPDATE_FORCE = true;
        } else {
            ADK_DEMO.OTAP_RE_UPDATE_FORCE = false;
        }
        switch (currentMode)
        {
            case ADI_WIL_MODE_STANDBY:
                break;
            case ADI_WIL_MODE_SLEEP:
            case ADI_WIL_MODE_MONITORING:
                /* Direct transition from Sleep/Monitoring to Active is allowed so no need to go to Standby */
                if(mode == ADI_WIL_MODE_ACTIVE)
                {
                    break;
                }

                /* For rest of modes transitions first go to standby */
            default:
                logAPIInProgress = ADI_WIL_API_SET_MODE;

                /* Set to Standby mode */
                //adi_wil_ex_info("Transitioning to Standby Mode");
                returnOnWilError(adi_wil_SetMode(pPack, ADI_WIL_MODE_STANDBY));
                
                /* Wait for non-blocking API to complete */
                WaitForWilAPI(pPack);
                
                if(gNotifRc == ADI_WIL_ERR_PARTIAL_SUCCESS) {
                    returnOnWilError(adi_wil_example_RetrySetMode(pPack, ADI_WIL_MODE_STANDBY));
                }

                //adi_wil_ex_info("Transitioning to Standby Mode successful!");
                returnOnWilError(adi_wil_example_ExecuteGetMode(pPack, &currentMode));
                break;
        }
        if(mode != ADI_WIL_MODE_STANDBY) {
            logAPIInProgress = ADI_WIL_API_SET_MODE;

            /* Set to Desired mode */
            //adi_wil_ex_info("Setting Network Mode to %s", adi_wil_SystemModeToString(mode));
            returnOnWilError(adi_wil_SetMode(pPack, mode));
            
            /* Wait for non-blocking API to complete */
            WaitForWilAPI(pPack);

            if(gNotifRc == ADI_WIL_ERR_PARTIAL_SUCCESS) {
                returnOnWilError(adi_wil_example_RetrySetMode(pPack, mode));
            }
            
            /* Set bUpdateExpectedGenTime as true for PerNodePSInfo */
            if(mode == ADI_WIL_MODE_ACTIVE)
            {
                bUpdateExpectedGenTime = true;
            }

            //adi_wil_ex_info("Network Mode set to %s",adi_wil_SystemModeToString(mode));
        }
    }
    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_RequestSetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode) 
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    errorCode = adi_wil_SetMode(pPack, mode);
            
    return errorCode;
}


/******************************************************************************
 * Example function using adi_wil_GetMode
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteGetMode(adi_wil_pack_t * const pPack,
                                             adi_wil_mode_t *pWilGetMode)
{
    returnOnWilError(adi_wil_GetMode(pPack, pWilGetMode));

    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_ExecuteGetMode(adi_wil_pack_t * const pPack,
											adi_wil_mode_t *pWilGetMode)
{
	adi_wil_err_t errorCode= ADI_WIL_ERR_SUCCESS;
	
	errorCode = adi_wil_GetMode(pPack, pWilGetMode);

    return errorCode;
}


/******************************************************************************
 * Example function using adi_wil_SetACL
 *****************************************************************************/
/**
 * @remark : inherit from TC375 project
 */
adi_wil_err_t adi_wil_example_ExecuteSetACL(uint8_t const * const pData, uint8_t iCount)
{
    returnOnWilError(adi_wil_SetACL(&packInstance, pData, iCount));

    /* Wait for non-blocking API to complete */
    WaitForWilAPI(&packInstance);

    /* If for some reason the set ACL did not succeed, display the failure code */
    returnOnWilError(gNotifRc);

    //adi_wil_ex_info("%s", "Set ACL successful!");
    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_RequestSetACL(uint8_t const * const pData, uint8_t iCount)
{
	adi_wil_err_t errorCode ; 
	
	errorCode = adi_wil_SetACL(&packInstance, pData, iCount);
 
    return errorCode;
}

/******************************************************************************
 * Example function using adi_wil_GetACL
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteGetACL(adi_wil_pack_t * const pPack)
{
    /* Set the Network Mode to STANDBY.                                     */
    /* Note GetACL can be called from STANDBY, Monitoring and ACTIVE mode   */
    // returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY)); /* @remark : Already STANDBY mode at this moment */

    logAPIInProgress = ADI_WIL_API_GET_ACL;
    returnOnWilError(adi_wil_GetACL(pPack));
    
    /* Wait for non-blocking API to complete */
    WaitForWilAPI(pPack);
   
    //adi_wil_ex_info("ACL retrieval successful!");
    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_RequestGetACL(adi_wil_pack_t * const pPack)
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
	
	errorCode = adi_wil_GetACL(pPack);
    
    return errorCode;
}

/******************************************************************************
 * Example function using adi_wil_SelectScript
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteSelectScript(adi_wil_pack_t * const pPack,
                                                   adi_wil_device_t eDeviceId,
                                                   adi_wil_sensor_id_t eSensorId,
                                                   uint8_t iScriptId,
                                                   bool no_set_mode)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    if(no_set_mode == false){
        /* Set the Network Mode to ACTIVE */
        returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_ACTIVE));
    }

    logAPIInProgress = ADI_WIL_API_SELECT_SCRIPT;
    
    errorCode = adi_wil_SelectScript(pPack,
                                     eDeviceId,
                                     eSensorId,
                                     iScriptId);
    if(errorCode != ADI_WIL_ERR_SUCCESS) {
        adk_debug_Report(DBG_wil_SelectScript, errorCode);
    }
    else {
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);

        errorCode = gNotifRc;
        if (gNotifRc == ADI_WIL_ERR_SUCCESS) {
                /* Success */
            }
        else {
                adk_debug_Report(DBG_wil_SelectScript_wait, errorCode);
            }
        }
    return errorCode;
}

adi_wil_err_t Cmic_RequestSelectScript(adi_wil_pack_t * const pPack,
                                       adi_wil_device_t eDeviceId,
                                       adi_wil_sensor_id_t eSensorId,
                                        uint8_t iScriptId)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

   
    errorCode = adi_wil_SelectScript(pPack,
                                     eDeviceId,
                                     eSensorId,
                                     iScriptId);
 
    return errorCode;
}


/******************************************************************************
 * Example function using adi_wil_GetNetworkStatus.
 * Return whether all nodes on the network has joined.
 *****************************************************************************/
bool adi_wil_example_ExecuteGetNetworkStatus(adi_wil_pack_t * const pPack,
                                             adi_wil_network_status_t *pNetworkStatus, bool no_set_mode)
{
    bool bAllNodesJoined = false;
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    uint32_t NetworkStatusTmr_thr = 0;
    uint32_t idle_count = 0;

    if(no_set_mode == false){
        /* Set the Network Mode to COMMISSIONING */
        returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_COMMISSIONING));
    }
    NetworkStatusTmr_thr = NetworkStatusTmr;

    //adi_wil_ex_info("Waiting for nodes in the ACL list to join the network...");
    while (!bAllNodesJoined)
    {
        /* keep trying to execute command if there's another operation in progress */
        
        do{
            idle_count++;
            errorCode = adi_wil_GetNetworkStatus(pPack, pNetworkStatus);
        }
        while(errorCode == ADI_WIL_ERR_API_IN_PROGRESS);

        if(errorCode != ADI_WIL_ERR_SUCCESS)
        {
            //adi_wil_ex_error("%s returned with error: %s", __func__, adi_wil_ErrorToString(errorCode));
            return bAllNodesJoined;
        }
        if(pNetworkStatus->iCount == 0)
        {
            //adi_wil_ex_info("No nodes in the ACL list!");
             return bAllNodesJoined;
        }
        else
        {
            bAllNodesJoined = true;
            for(uint32_t i = 0; i < pNetworkStatus->iCount; i++)
            {
                ADK_NodeState[i] = pNetworkStatus->iConnectState & (1ULL<<i);
                if(!(ADK_NodeState[i]))
                {
                    bAllNodesJoined = false;
                }
            }
            if(!bAllNodesJoined)
            {
                // Some nodes are connected, not all
                // Compare to threshold
                if(ADK_DEMO.OTAP_UPDATE_FORCE)
                {
                    if((NetworkStatusTmr - NetworkStatusTmr_thr) > (NETWORK_STATUS_THRESHOLD/2))
                    {
                        break;
                    }
                }
                else
                {
                    if((NetworkStatusTmr - NetworkStatusTmr_thr) > NETWORK_STATUS_THRESHOLD)
                    {
                        break;
                    }
                }                
            }
        }
    }

    return bAllNodesJoined;
}


/******************************************************************************
 * Example function using adi_wil_GetNetworkStatus.
 * Return whether all nodes on the network has joined.
 *****************************************************************************/
bool Cmic_RequestGetNetworkStatus(adi_wil_pack_t * const pPack,
                                             adi_wil_network_status_t *pNetworkStatus)
{
    bool bAllNodesJoined = false;
    bool bNodeState[64] = {false,};
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    uint32_t idle_count = 0;
    
    while (!bAllNodesJoined) //adi_wil_ex_info("Waiting for nodes in the ACL list to join the network...");
    {
        do{
            idle_count++;
            errorCode = adi_wil_GetNetworkStatus(pPack, pNetworkStatus);

		} while(errorCode == ADI_WIL_ERR_API_IN_PROGRESS);

        if((errorCode != ADI_WIL_ERR_SUCCESS) ||
                (pNetworkStatus->iCount == 0))
        {            
             return bAllNodesJoined;
        }
        else
        {
            bAllNodesJoined = true;
            for(uint8_t i = 0; i < pNetworkStatus->iCount; i++)
            {
                bNodeState[i] = pNetworkStatus->iConnectState & (1ULL<<i);
                if(!(bNodeState[i]))
                {
                    bAllNodesJoined = false;
                }
            }
        }
    }

    return bAllNodesJoined;
}

adi_wil_err_t adi_wil_example_otap_GetNetworkStatus(adi_wil_network_status_t * const pStatus)
{
    /* adi_wil_GetNetworkStatus() is a blocking call */
    adi_wil_err_t rc = adi_wil_GetNetworkStatus(&packInstance, pStatus);

    return rc;
}
uint8_t adi_wil_example_countUnconnectedNodes(void)
{
    adi_wil_network_status_t NetworkStatus;
    fatalOnWilError(adi_wil_example_otap_GetNetworkStatus(&NetworkStatus));

    uint8_t iUnconnectedNodeCount = 0;
    for(uint8_t iCount = 0; iCount < NetworkStatus.iCount; iCount++)
    {
        if (!((NetworkStatus.iConnectState) >> iCount)&0x1) {  iUnconnectedNodeCount++;  }
    }

    return iUnconnectedNodeCount;
}

/* Waits for all nodes to join the network before returning */
void adi_wil_example_waitForNodesToJoin(void)
{
    uint8_t unconnected_nodes = adi_wil_example_countUnconnectedNodes();

    if (unconnected_nodes == 0)
    {
        // info("All nodes have joined, no need to wait.");
    }
    else
    {
        adi_wil_mode_t eCurrentMode;
        adi_wil_example_ExecuteGetMode(&packInstance, &eCurrentMode);
        adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
        adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_COMMISSIONING);

        // info("Waiting for %u nodes to join the network....", unconnected_nodes);

        while (adi_wil_example_countUnconnectedNodes() != 0);

        /* Restore system mode we were in before going to commissioning */
        adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
        adi_wil_example_ExecuteSetMode(&packInstance, eCurrentMode);
    }
}

/******************************************************************************
 * Example function using adi_wil_example_processMac
 *****************************************************************************/
bool adi_wil_example_processMac(const volatile uint8_t *buf,
                 size_t iBufRemaining,
                 size_t *used,
                 uint8_t *dst)
{
    const size_t buf_sz = iBufRemaining;
    bool bHexIncr = true; /* Most significant nibble first */
    uint8_t iHexVal = 0;
    int bufIndex = 0;
    int iDstUsed = 0;

    while (bufIndex < buf_sz && iDstUsed < ADI_WIL_MAC_ADDR_SIZE) { /* a number */
        const uint8_t c = buf[bufIndex++];

        if (c >= 48 && c <= 57) { /* number */
            iHexVal |= (c - 48) << ((bHexIncr) ? 4 : 0);
        } else if (c >= 65 && c <= 70) { /* upper case hex */
            /* not 97 because we're adding 10 as it's hex [10..15] */
            iHexVal |= (c - 55) << ((bHexIncr) ? 4 : 0);
        } else if (c >= 97 && c <= 102) { /* lower case hex */
            /* not 97 because we're adding 10 as it's hex [10..15] */
            iHexVal |= (c - 87) << ((bHexIncr) ? 4 : 0);
        } else if (c == ',' && iDstUsed) {
            //adi_wil_ex_error("Malformed mac entry. ',' encountered early");
            break;
        } else if (c == ' ') { /* skip spaces */
            continue;
        } else {
            //adi_wil_ex_error("Unexpected character in mac: %c", c);
            break;
        }

        if (!bHexIncr) {
            dst[iDstUsed++] = iHexVal;
            iHexVal = 0;
        }
        bHexIncr = !bHexIncr;
    }

    *used = bufIndex;

    return iDstUsed == ADI_WIL_MAC_ADDR_SIZE;
}

/******************************************************************************
 * Example function using adi_wil_example_ExecuteLoadFile
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteLoadFile(adi_wil_pack_t * const pPack, adi_wil_file_type_t eFileType, adi_wil_device_t eDevice, bool no_set_mode)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    static uint8_t *pOtapImage;
    uint32_t iOtapImageLen;
    ClientData.LoadFileStatus.iOffset = 0;    
    uint32_t iImageLength = 0;
    uint8_t opfw_cnt = 0;
    float OTAP_Node_Prev[ADK_MAX_node] = {0,};
    float OTAP_Mng_Prev[2] = {0,};

    ADK_DEMO.LF_NG_CNT = 0; /* device removed counter clear */
    memset(&LF_NG_LIST, 0, sizeof(LF_NG_LIST)); /* device removed list clear */

    if(no_set_mode == false){
        /* Set the Network Mode to STANDBY */
        returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY));
    }

    if(eFileType == ADI_WIL_FILE_TYPE_BMS_CONTAINER)
    {
        if ((eDevice == ADI_WIL_DEV_MANAGER_0) || (eDevice == ADI_WIL_DEV_MANAGER_1) || (eDevice == ADI_WIL_DEV_ALL_MANAGERS ))
        {
            if(eDevice == ADI_WIL_DEV_MANAGER_0) {
                /* @remark  : Not used this time */
            }
            if(eDevice == ADI_WIL_DEV_MANAGER_1) {
                /* @remark  : Not used this time */
            }
        }
        if((eDevice <= ADI_WIL_DEV_NODE_61) || ( eDevice == ADI_WIL_DEV_ALL_NODES ))
        {
            adi_bms_GetContainerPtr(&pOtapImage, &iOtapImageLen);
        }
    }
    else if(eFileType == ADI_WIL_FILE_TYPE_CONFIGURATION)
    {
        pOtapImage = (uint8_t *)&configuration_file_configuration;
        iOtapImageLen =  configuration_file_configuration_length;
    }
    else if(eFileType == ADI_WIL_FILE_TYPE_FIRMWARE)
    {
        if ((eDevice == ADI_WIL_DEV_MANAGER_0) || (eDevice == ADI_WIL_DEV_MANAGER_1) || (eDevice == ADI_WIL_DEV_ALL_MANAGERS ))
        {
            if(eDevice == ADI_WIL_DEV_MANAGER_0) {
                /* @remark  : Not used this time */
            }
            if(eDevice == ADI_WIL_DEV_MANAGER_1) {
                /* @remark  : Not used this time */
            }

            if((ADK_DEMO.OTAP_STAT >= Demo_OTAP_LoadFile_OPFW_AllMngrs______) && (ADK_DEMO.OTAP_STAT <= Demo_OTAP_GetDeviceVersion_Node_Next__))
            {
                adi_bms_GetMngrOPFW220Ptr(&pOtapImage, &iOtapImageLen);
            }
        }
        if((eDevice <= ADI_WIL_DEV_NODE_61) || ( eDevice == ADI_WIL_DEV_ALL_NODES ))
        {
            if((ADK_DEMO.OTAP_STAT >= Demo_OTAP_SetMode_STANDBY_0___________) && (ADK_DEMO.OTAP_STAT <= Demo_OTAP_LoadFile_OPFW_AllNodes______))
            {
                adi_bms_GetNodeOPFW220Ptr(&pOtapImage, &iOtapImageLen);
            }
        }

    }

    /* File transfer loop */
    iImageLength = iOtapImageLen;
    do
    {
        if(iOtapImageLen > ADI_WIL_LOADFILE_DATA_SIZE)
        {
            iOtapImageLen = iImageLength - ClientData.LoadFileStatus.iOffset;
        }

        /* WIL API Call */
        logAPIInProgress = ADI_WIL_API_LOAD_FILE;
        do {
            do {
                errorCode = adi_gProcessTaskErrorCode;
            } while(errorCode != ADI_WIL_ERR_SUCCESS);
            errorCode = adi_wil_LoadFile(pPack, eDevice, eFileType, &pOtapImage[ClientData.LoadFileStatus.iOffset]);
        } while(errorCode == ADI_WIL_ERR_API_IN_PROGRESS);

        if (errorCode != ADI_WIL_ERR_SUCCESS)
        {
            /* Handle error */
            adk_debug_Report(DBG_wil_LoadFile, errorCode);
            if(errorCode == ADI_WIL_ERR_INVALID_PARAMETER)
                return errorCode;
            else
                break;
        }
        else
        {
            /* Wait for non-blocking API to complete */
            WaitForWilAPI(pPack);

            if(gNotifRc == ADI_WIL_ERR_FAIL || gNotifRc == ADI_WIL_ERR_TIMEOUT)
            {
                /* An error has occurred. The loading process has aborted. */
                /* No further API callbacks will occur. Re-start the OTAP. */
                ClientData.LoadFileStatus.iOffset = 0;
                iOtapImageLen = iImageLength;
                gNotifRc = ADI_WIL_ERR_IN_PROGRESS;
            }
            else if (gNotifRc == ADI_WIL_ERR_PARTIAL_SUCCESS){
                return ADI_WIL_ERR_SUCCESS;
            }
            else if ((gNotifRc != ADI_WIL_ERR_SUCCESS) && (gNotifRc != ADI_WIL_ERR_IN_PROGRESS)) {
                adk_debug_Report(DBG_wil_LoadFile_wait, errorCode);
                return gNotifRc;
            }
        }

        /* @remark : calculate progress for OTAP */
        if((eFileType == ADI_WIL_FILE_TYPE_FIRMWARE) && (ADK_DEMO.OTAP_STAT > Demo_OTAP_Update_Triggered____________) && (ADK_DEMO.OTAP_STAT < Demo_OTAP_Complete____________________)){
            if(eDevice == ADI_WIL_DEV_ALL_NODES){
                for(opfw_cnt = 0; opfw_cnt < realAcl.iCount; opfw_cnt++){
                    if(OTAP_Node_Prev[opfw_cnt] < (((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f))
                    {
                        ADK_DEMO.OTAP_NODE[opfw_cnt].OTAP____PROGRESS = ((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f;
                        OTAP_Node_Prev[opfw_cnt] = ((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f;
                    }
                }
            }
            else if(eDevice <= ADI_WIL_DEV_NODE_61){
                /* retry case */

            }

            if(eDevice == ADI_WIL_DEV_ALL_MANAGERS){
                if(OTAP_Mng_Prev[0] < (((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f))
                {
                    ADK_DEMO.OTAP_MNGR[0].OTAP____PROGRESS = ((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f;
                    OTAP_Mng_Prev[0] = ADK_DEMO.OTAP_MNGR[0].OTAP____PROGRESS;
                }
                if(OTAP_Mng_Prev[1] < (((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f))
                {
                    ADK_DEMO.OTAP_MNGR[1].OTAP____PROGRESS = ((float)(iImageLength - iOtapImageLen) / (float)iImageLength) * 100.0f;
                    OTAP_Mng_Prev[1] = ADK_DEMO.OTAP_MNGR[1].OTAP____PROGRESS;
                }
            }
            else if(eDevice <= ADI_WIL_DEV_NODE_61){
                /* retry case */

            }
        }
    } while (ADI_WIL_ERR_IN_PROGRESS == gNotifRc);

    /* @remark : calculate progress for OTAP */
    if((errorCode == ADI_WIL_ERR_SUCCESS) && (eFileType == ADI_WIL_FILE_TYPE_FIRMWARE) && (ADK_DEMO.OTAP_STAT > Demo_OTAP_Update_Triggered____________) && (ADK_DEMO.OTAP_STAT < Demo_OTAP_Complete____________________)){
        if(eDevice == ADI_WIL_DEV_ALL_NODES){
            for(opfw_cnt = 0; opfw_cnt < realAcl.iCount; opfw_cnt++){                    
                ADK_DEMO.OTAP_NODE[opfw_cnt].OTAP____PROGRESS = 100.0f;
            }
        }
        if(eDevice == ADI_WIL_DEV_ALL_MANAGERS){             
            ADK_DEMO.OTAP_MNGR[0].OTAP____PROGRESS = 100.0f;
            ADK_DEMO.OTAP_MNGR[1].OTAP____PROGRESS = 100.0f;
        }
    }

    return errorCode;
}



void adi_wil_example_LoadFileRetry(adi_wil_file_type_t eFileType, adi_wil_device_t eDevice)
{
    adi_wil_device_t iTargetDevices;
    bool retry_bAllNodesJoined = false;
    uint8_t LF_NG_CNT = 0;

    if(ADK_DEMO.LF_NG_CNT > 0)
    {   
        /* Commissioning again */
        do{
            retry_bAllNodesJoined = adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &networkStatus, SET_MODE);
            if(!retry_bAllNodesJoined){
                adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
                adi_wil_example_ExecuteResetDevice(&packInstance, eDevice);
            }
        }while(!retry_bAllNodesJoined);
        adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
        
        /* retry logic */
        do{
            iTargetDevices = 0;
            for(LF_NG_CNT = 0; LF_NG_CNT < ADK_DEMO.LF_NG_CNT; LF_NG_CNT++)
            {
                iTargetDevices |= LF_NG_LIST[LF_NG_CNT].eDeviceId;
            }
            fatalOnWilError(adi_wil_example_ExecuteLoadFile(&packInstance, eFileType, iTargetDevices, NO_SET_MODE));
        }while(ADK_DEMO.LF_NG_CNT != 0);
    }
}

/* Waits for event eEventCode to occur iCount times before returning */
void adi_wil_example_waitForEvent(adi_wil_event_id_t eEventCode, uint32_t iCount)
{
    // info("Waiting for %u %s event(s)", iCount, adi_wil_EventToString(eEventCode));

    /* Set the event to wait for and reset event counter */
    ClientData.eEventWaitingFor = eEventCode;
    ClientData.iEventWaitingForCount = 0;

    for(uint32_t i = 0; i < iCount; i++)
    {
        uint32_t startTime = adi_wil_hal_TickerGetTimestamp();
        uint32_t promptTime = WAIT_FOR_EVENT_PROMPT_INITIAL_MSEC;
        /* use a volatile pointer, this prevents compiler from optimizing away the wait loop */
        uint32_t volatile * pEventCount = &ClientData.iEventWaitingForCount;
        while (*pEventCount == i)
        {
            uint32_t currentTime = adi_wil_hal_TickerGetTimestamp();
            if ((currentTime - startTime) >= promptTime)
            {
                // info("Example has been waiting for %u seconds. This may indicate an external problem e.g. a node or a manager has lost power.", (promptTime / 1000));
                // info("If this wait is not expected, please check your system. You may then need to restart the example.");
                promptTime += WAIT_FOR_EVENT_PROMPT_REPEAT_MSEC;
            }
        }

        // info("  Count %u", i);
    }
}

/******************************************************************************
 * adi_wil_example_PrintACL
 *****************************************************************************/
void adi_wil_example_PrintACL(adi_wil_pack_t * const pPack)
{
    for (int i=0; i < systemAcl.iCount; i++) {
        //adi_wil_ex_info("MAC entry %d : %02X%02X%02X",
//             i,
//             systemAcl.Data[(i*ADI_WIL_MAC_ADDR_SIZE)+5],
//             systemAcl.Data[(i*ADI_WIL_MAC_ADDR_SIZE)+6],
//             systemAcl.Data[(i*ADI_WIL_MAC_ADDR_SIZE)+7]);
    }
}

/******************************************************************************
 * Example function using adi_wil_example_ProcessTask
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ProcessTask(adi_wil_pack_t * const pPack)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    /* Wait till event is processed */
    errorCode = adi_wil_ProcessTask(pPack);

    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        if(!bConnectApiCalled)
        {
            /**
             * If adi_wil_ProcessTask is called with an uninitialized pack object 
             * (as it may happen if adi_wil_ProcessTask is in a separate thread or 
             * adi_wil_QueryDevice is being executed), the API may return ADI_WIL_ERR_INVALID_PARAMETER
             * which is expected and it should be ignored by the host application. 
             * The pack is initialzed after calling teh Conenct API
             * adi_wil_ProcessTask now also returns ADI_WIL_ERR_API_IN_PROGRESS
             * since it now acquires a lock before proceeding with pack processing.
             * The processTask now just reuses the global lock, which QueryDevice 
             * uses when it占쎌뀼 running, hence the return code ADI_WIL_ERR_API_IN_PROGRESS
             * which is also expected.
             */
            if (!((errorCode == ADI_WIL_ERR_API_IN_PROGRESS) ||
                  (errorCode == ADI_WIL_ERR_INVALID_PARAMETER)))
            {
                adk_debug_Report(DBG_wil_ProcessTask_0, errorCode);
            }
        }
        else
        {
            adk_debug_Report(DBG_wil_ProcessTask_1, errorCode);
        }
    }
    return errorCode;
}

/******************************************************************************
 * print_welcome message
 *****************************************************************************/
void adi_wil_example_print_welcome(void)
{    
    adi_wil_version_t version;
    adi_wil_GetWILSoftwareVersion(&version);
}

/******************************************************************************
 * Example function using adi_wil_GetDeviceVersion
 *****************************************************************************/
adi_wil_err_t adi_wil_example_GetDeviceVersion(adi_wil_pack_t * const pPack, adi_wil_device_t eDevice, bool no_set_mode)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    if(no_set_mode == false){
        /* Set the Network Mode to STANDBY */
        returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY));
    }

    /* Save the API function pointer */
    logAPIInProgress = ADI_WIL_API_GET_DEVICE_VERSION;

    /* Call the adi_wil_GetDeviceVersion API */
    errorCode = adi_wil_GetDeviceVersion(pPack, eDevice);
    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        /* Handle error */
        adk_debug_Report(DBG_wil_GetDeviceVersion, errorCode);
    }
    else
    {
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);
    }

    /* Check for rc from the API callback and print out the main processor device information and silicon version */
    if (gNotifRc == ADI_WIL_ERR_SUCCESS){
        // adi_wil_ex_info("Device S/W Version Information: %d.%d.%d.%d",
        // deviceVersion.MainProcSWVersion.iVersionMajor,
        // deviceVersion.MainProcSWVersion.iVersionMinor,
        // deviceVersion.MainProcSWVersion.iVersionPatch,
        // deviceVersion.MainProcSWVersion.iVersionBuild);
        // adi_wil_ex_info("Device Silicon Version Information: %d.%d", deviceVersion.iMainProcSiVersion, deviceVersion.iCoProcSiVersion);
    }
    return errorCode;
}

/******************************************************************************
 * Example function using adi_wil_GetFileCRC
 *****************************************************************************/
adi_wil_err_t adi_wil_example_GetFileCRC(adi_wil_pack_t * const pPack,
                                         adi_wil_device_t eDevice,
                                         adi_wil_file_type_t eFileType, 
                                         bool no_set_mode)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    if(no_set_mode == false){
        /* Set the Network Mode to STANDBY */
        returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY));
    }

    /* Save the API function pointer */
    logAPIInProgress = ADI_WIL_API_GET_FILE_CRC;

    /* Call the adi_wil_GetFileCRC API */
    errorCode = adi_wil_GetFileCRC(pPack, eDevice, eFileType);
    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        /* Handle error */
        adk_debug_Report(DBG_wil_GetFileCRC, errorCode);
    }
    else
    {
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);
    }

    /* Check for rc from the API callback and print out the container details */
     errorCode = gNotifRc;
    if (gNotifRc == ADI_WIL_ERR_SUCCESS){
        // adi_wil_ex_info("Container details on device %s corresponding to sensorID type %s as follows:-",adi_wil_DeviceToString(eDevice), adi_wil_SensorIdTypeToString(eSensorId));   /* @remark : Not used this time */
        //adi_wil_ex_info("Container ID : %d", containerDetails.iContainerId);
        //adi_wil_ex_info("Container Version : %d", containerDetails.iVersion);
        //adi_wil_ex_info("Container Hash : 0x%08X", containerDetails.iHash);
    }
    else {
        adk_debug_Report(DBG_wil_GetFileCRC_wait, errorCode);
    }
    return errorCode;
}

#ifdef DBG_GET_FILE_TEST
/******************************************************************************
 * Example function using adi_wil_example_ExecuteGetFile
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteGetFile(adi_wil_pack_t * const pPack, 
                                            adi_wil_file_type_t eFileType, 
                                            adi_wil_device_t eDevice)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
    uint32_t iTotalFileSizeRcvd = 0u;

    /* Set the Network Mode to STANDBY */
    returnOnWilError(adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY));

    do
    {
        /* Call the API to get the next block in file */
        errorCode = adi_wil_GetFile(pPack, eDevice, eFileType);
 
        if (ADI_WIL_ERR_SUCCESS != errorCode)
        {
            /* Handle error */
            adk_debug_Report(DBG_wil_GetFile, errorCode);
        }
        else
        {
            /* Wait for non-blocking API to complete */
            WaitForWilAPI(pPack);

            errorCode = gNotifRc;

            if ((errorCode == ADI_WIL_ERR_SUCCESS) || (errorCode == ADI_WIL_ERR_IN_PROGRESS)) {
                if (((getFileInformation.iByteCount + getFileInformation.iRemainingBytes)) > GET_FILE_BUFFER_COUNT) {
                    // error("Not enough memory allocated for requested file.");
                    errorCode = ADI_WIL_ERR_FAIL; // end the get api function
                }
                iTotalFileSizeRcvd += getFileInformation.iByteCount;
                if (errorCode == ADI_WIL_ERR_SUCCESS) {
                    // adi_wil_ex_info("Get File Success!");
                    // adi_wil_ex_info("File size received (bytes): %d", iTotalFileSizeRcvd);
                }
            }
            else {
                adk_debug_Report(DBG_wil_GetFile_wait, errorCode);
            }
        }
    } while (errorCode == ADI_WIL_ERR_IN_PROGRESS);

    return errorCode;
}
#endif

/******************************************************************************
 * Example function using adi_wil_EraseFile
 *****************************************************************************/
adi_wil_err_t adi_wil_example_EraseFile(adi_wil_pack_t * const pPack,
                                        adi_wil_device_t eDeviceId,
                                        adi_wil_file_type_t eFileType)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    /* Set the Network Mode to STANDBY */
    adi_wil_example_ExecuteSetMode(pPack, ADI_WIL_MODE_STANDBY);

    /* Save the API function pointer */
    logAPIInProgress = ADI_WIL_API_ERASE_FILE;

    /* Call the adi_wil_EraseFile API */
    errorCode = adi_wil_EraseFile(pPack, eDeviceId, eFileType);
    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        /* Handle error */
        adk_debug_Report(DBG_wil_EraseFile, errorCode);
    }
    else
    {
        /* Wait for non-blocking API to complete */
        WaitForWilAPI(pPack);

        /* Check for rc from the API callback and print out error if it was not a success*/
        errorCode = gNotifRc;
        if (gNotifRc == ADI_WIL_ERR_SUCCESS) {
            // adi_wil_ex_info("%s file erased from %s", adi_wil_FileTypeToString(eFileType), adi_wil_DeviceToString(eDeviceId));   /* @remark : Not used this time */
        }
        else {
            adk_debug_Report(DBG_wil_EraseFile_wait, errorCode);
        }
    }
    return errorCode;
}

/******************************************************************************
 * Example function that returns true if manager pair is configured in dual
 * manager configuration.
 *****************************************************************************/
bool adi_wil_example_isDualConfig(adi_wil_configuration_t * C1, adi_wil_configuration_t * C2, adi_wil_port_t* pPort)
{
    bool rc = false;
    uint8_t BlankMac[ADI_WIL_MAC_ADDR_SIZE] = {0};
    if (C1->bDualConfiguration && C2->bDualConfiguration && /* Compare bDualConfiguration */
        (!memcmp(C1->PeerMAC, C2->MAC, ADI_WIL_MAC_ADDR_SIZE)) && (!memcmp(C2->PeerMAC, C1->MAC, ADI_WIL_MAC_ADDR_SIZE)) && /* Compare MAC and PeerMAC */
        (C1->iConfigurationHash == C2->iConfigurationHash))
    {
        //adi_wil_ex_info("Manager connected to SPI device %d and Manager connected to SPI device %d are configured in Dual manager mode!",pPort[0].iSPIDevice,pPort[1].iSPIDevice);
        rc = true;
    }
    if (!memcmp(C1->PeerMAC, BlankMac, ADI_WIL_MAC_ADDR_SIZE))
    {
        //adi_wil_ex_info("Manager connected to SPI device %d is in single manager mode.",pPort[0].iSPIDevice); 
    }
    if (!memcmp(C2->PeerMAC, BlankMac, ADI_WIL_MAC_ADDR_SIZE))
    {
        //adi_wil_ex_info("Manager connected to SPI device %d is in single manager mode.",pPort[1].iSPIDevice);
    }
    return rc;
}




/**
 * @remark : custome function for ADK
 */

void adi_wil_example_ADK_ExecuteProcessBMSBuffer(void)
{

#if BMS_PACKETS_PRINTING_ON
    //adi_wil_ex_info("BMS Received from Node %d\t\tBMS Packet count: %d", userBMSBuffer->eDeviceId, iBMSNotificationCount);
#endif

    adi_wil_example_ADK_readBms();            /* @remark : Read BMS data */

}


void adi_wil_example_ADK_readBms(void)
{
    uint8_t pkt_count = 0;
    uint8_t BMS_packetId = 0;
    uint8_t BMS_eNode = 100;
  //  adi_bms_base_pkt_0_t* BMS_Packet_0_Ptr;
  //  adi_bms_base_pkt_1_t* BMS_Packet_1_Ptr;
  //  adi_bms_base_pkt_2_t* BMS_Packet_2_Ptr;
   uint8_t temp_recv_confirm[ADK_MAX_node] = {0,};
    int i;

    for(pkt_count = 0; pkt_count < nTotalPcktsRcvd; pkt_count++){
        if (userBMSBuffer[pkt_count].iLength == 0) continue; // Adele: skip processing of empty packet

        BMS_eNode = ADK_ConvertDeviceId(userBMSBuffer[pkt_count].eDeviceId);
        BMS_packetId = userBMSBuffer[pkt_count].Data[0];
                
        if(BMS_packetId == ADI_BMS_BASE_PKT_0_ID ){
            temp_recv_confirm[BMS_eNode] |= 0x01;
            ADK_DEMO.BMS_Packet_0_Ptr = (adi_bms_base_pkt_0_t*) &userBMSBuffer[pkt_count].iLength;
            
            /* @remark : Read raw data (float) */
            temp_buffer[0]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdaca.iAc2v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdaca.iAc2v[0]);
            temp_buffer[1]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdaca.iAc3v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdaca.iAc3v[0]);
            temp_buffer[2]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc4v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc4v[0]);
            temp_buffer[3]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc5v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc5v[0]);
            temp_buffer[4]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc6v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacb.iAc6v[0]);
            temp_buffer[5]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc7v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc7v[0]);
            temp_buffer[6]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc8v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc8v[0]);
            temp_buffer[7]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc9v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacc.iAc9v[0]);
            temp_buffer[8]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacd.iAc11v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacd.iAc11v[0]);
            temp_buffer[9]  = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacd.iAc12v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacd.iAc12v[0]);
            temp_buffer[10] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc13v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc13v[0]);
            temp_buffer[11] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc14v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc14v[0]);
            temp_buffer[12] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc15v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdace.iAc15v[0]);
            temp_buffer[13] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc16v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc16v[0]);
            temp_buffer[14] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc17v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc17v[0]);
            temp_buffer[15] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc18v[1]  << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdacf.iAc18v[0]);
            temp_buffer[16] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iG1v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iG1v[0]);
            temp_buffer[17] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iG2v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iG2v[0]);
            temp_buffer[18] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iGa11v[1] << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdauxa.iGa11v[0]);
            temp_buffer[19] = (signed short int)((ADK_DEMO.BMS_Packet_0_Ptr->Rdauxe.iG3v[1]   << 8) + ADK_DEMO.BMS_Packet_0_Ptr->Rdauxe.iG3v[0]);
           

            for(i = 0; i < TEST_BUFFER_MAX; i++){
                if(temp_buffer[i] == 0)
                {
                    TEMP_BUFFER_ZERO[i]++;
                }
            }            
            for(i = 0; i < 16; i++){
                /* @remark : Cell voltage convert (float) */
                ADK_DEMO.NODE[BMS_eNode].CELL_V[i] = (float)(temp_buffer[i]  * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : Cell voltage convert (int16) */
                ADK_DEMO.NODE[BMS_eNode].CELL_Vi[i] = temp_buffer[i] + 10000;
            }

            for(i = 0; i < 4; i++){
                /* @remark : AUX voltage convert (float) */
                ADK_DEMO.NODE[BMS_eNode].TEMP_V[i]  = (temp_buffer[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;

                /* @remark : AUX voltage convert (int16) */
                ADK_DEMO.NODE[BMS_eNode].TEMP_Vi[i]  = temp_buffer[i + 16] + 10000;  

            }

            #ifdef DBG_TEMP_TEST
            for(i =0 ;i < 3; i++)
            {
                if(temp_buffer[16+i] == 0)
                {
                    TEMP_ZERO[i]++;
                }
            }

            if(TEMP_V2_MAX < ADK_DEMO.NODE[BMS_eNode].TEMP_V[2])
            {
                TEMP_V2_MAX = ADK_DEMO.NODE[BMS_eNode].TEMP_V[2];
            }
            if(TEMP_V2_MIN > ADK_DEMO.NODE[BMS_eNode].TEMP_V[2])
            {
                TEMP_V2_MIN = ADK_DEMO.NODE[BMS_eNode].TEMP_V[2];
            }
            #endif

            #ifdef DBG_TEMP_TEST
            if(temp_buffer[19] == 0)
            {
                TEMP_ZERO[3]++;
            }
            
            if(TEMP_V3_MAX < ADK_DEMO.NODE[BMS_eNode].TEMP_V[3])
            {
                TEMP_V3_MAX = ADK_DEMO.NODE[BMS_eNode].TEMP_V[3];
            }
            if(TEMP_V3_MIN > ADK_DEMO.NODE[BMS_eNode].TEMP_V[3])
            {
                TEMP_V3_MIN = ADK_DEMO.NODE[BMS_eNode].TEMP_V[3];
            }
            #endif

            Adbms683x_Monitor_Base_Pkt0(&userBMSBuffer[pkt_count]);     
        }
        else if (BMS_packetId == ADI_BMS_BASE_PKT_1_ID){
            temp_recv_confirm[BMS_eNode] |= 0x02;
            ADK_DEMO.BMS_Packet_1_Ptr = (adi_bms_base_pkt_1_t*) &userBMSBuffer[pkt_count].iLength;

            #if  (ADK_ADBMS683x == 0) /* ADBMS6830 */
            ADK_DEMO.NODE[BMS_eNode].CB_STAT = (ADK_DEMO.BMS_Packet_1_Ptr->Rdcfgb.iCfgbr5 << 8) + ADK_DEMO.BMS_Packet_1_Ptr->Rdcfgb.iCfgbr4 ;
            #elif(ADK_ADBMS683x == 3) /* ADBMS6833 */
            ADK_DEMO.NODE[BMS_eNode].CB_STAT = ((ADK_DEMO.BMS_Packet_1_Ptr->Rdcfga.iCfgar2 & BMS_CELLS_1_TO_2_MASK) << 16) + (ADK_DEMO.BMS_Packet_1_Ptr->Rdcfgb.iCfgbr5 << 8) + ADK_DEMO.BMS_Packet_1_Ptr->Rdcfgb.iCfgbr4;
            #else   /* Not supported */
            #endif
            Adbms683x_Monitor_Base_Pkt1(&userBMSBuffer[pkt_count]);       
        }
        else if (BMS_packetId == ADI_BMS_BASE_PKT_2_ID){
            temp_recv_confirm[BMS_eNode] |= 0x04;
            ADK_DEMO.BMS_Packet_2_Ptr = (adi_bms_base_pkt_2_t*) &userBMSBuffer[pkt_count].iLength;

            #if  (ADK_ADBMS683x == 0) /* ADBMS6830 */
            ADK_DEMO.NODE[BMS_eNode].OWD_CS_STAT = (ADK_DEMO.BMS_Packet_2_Ptr->Rdstatc.iCsflt[1] << 8) + ADK_DEMO.BMS_Packet_2_Ptr->Rdstatc.iCsflt[0];
            #elif(ADK_ADBMS683x == 3) /* ADBMS6833 */
            ADK_DEMO.NODE[BMS_eNode].OWD_CS_STAT = ((ADK_DEMO.BMS_Packet_2_Ptr->Rdstatc.iStcr2 & 0xC0) << 10) + (ADK_DEMO.BMS_Packet_2_Ptr->Rdstatc.iStcr1 << 8) + ADK_DEMO.BMS_Packet_2_Ptr->Rdstatc.iStcr0;
            #else   /* Not supported */
            #endif
        }
        else{
            /* DO NOTHING */
			#ifdef DBG_PACKET_ID_TEST
            if(index < 50){
                TEMP_PACKET_ID[index] = BMS_packetId;
                TEMP_DEVICE_ID[index++] = BMS_eNode;
            }
			#endif
        }

        #if(ADK_ADBMS683x == 0) /* ADBMS6830 */
        if(iOWDPcktsRcvd[BMS_eNode] == SM_ADBMS6830_ALL_PCKTS_FOR_OWD_RCVD)
        
        #elif(ADK_ADBMS683x == 3) /* ADBMS6833 */
        if(iOWDPcktsRcvd[BMS_eNode] == SM_ADBMS6833_ALL_PCKTS_FOR_OWD_RCVD)

        #else   /* Not supported */
        #endif
        {
            Adbms683x_Monitor_Cell_OWD(BMS_eNode);
            iOWDPcktsRcvd[BMS_eNode] = 0U;
        }
    }
    for(pkt_count = 0; pkt_count < NODE_NUM; pkt_count++)
    {
        // Adele
        uint8_t* bms_pkt_map = ADK_DEMO.NODE[pkt_count].BMS_PKT_MAP;
        if(iBMSNotificationCount < 5)
            bms_pkt_map[1] = 0xFF;
        bms_pkt_map[0] = temp_recv_confirm[pkt_count];
        if (adi_cnt1_8bit(bms_pkt_map[0]) < adi_cnt1_8bit(bms_pkt_map[1]))
            bms_pkt_map[1] = bms_pkt_map[0];
    }
}


/**
 * @remark: This is example, exceptional case must be handled for PDR accuracy
 */
static void adi_wil_example_ADK_PDR_calc(void)
{
    uint8_t i_node = 0;
    uint8_t i_pkt = 0;
    uint32_t temp_buffer;

    uint16_t i,j,k;   

    uint8_t  temp_drvd_pkt[ADK_MAX_node]={0,};

    uint16_t temp_PDR_OK[ADK_MAX_node]={0,};
    uint16_t temp_PDR_TOTAL[ADK_MAX_node]={0,};
    uint8_t  pdr_DUP_buffer[ADK_MAX_node]={0,};

    /* Step 1 : Init global PDR data */
    if(FLG_pdr_initial == false)
    {
        memset(&accm_PDR_OK[0],                 0, sizeof(uint64_t) * ADK_MAX_node);
        memset(&accm_PDR_TOTAL[0],             0, sizeof(uint64_t) * ADK_MAX_node);

        memset(&pkt_seq_buffer[0][0],           0, sizeof(uint32_t) * PDR_pktbuf_depth * ADK_MAX_node);
        memset(&pkt_valid_index[0],             0, sizeof(uint8_t) * ADK_MAX_node);
        memset(&pkt_valid_seq[0],               0, sizeof(uint32_t) * ADK_MAX_node);
        memset(&FLG_pdr_seq_initial[0],         0, sizeof(uint8_t) * ADK_MAX_node);
        memset(&FLG_seq_num_rollback_period[0], 0, sizeof(uint8_t) * ADK_MAX_node);
        FLG_pdr_initial = true;
    }

    for(i_node=0 ; i_node<NODE_NUM ; i_node++)
    {
        // Step 2 : Store sequence number of all NW packet
            //Store sequence number into "pkt_seq_buffer"
            //Inclease valid index number of "pkt_seq_buffer" to "pkt_valid_index(15)"
            //Assume initial number of derivered packet for instant PDR during (buffer<15)
        for(i_pkt=0 ; i_pkt<(NODE_NUM*BMS_PACKETS_PER_INTERVAL) ; i_pkt++)
        {
            if(ADK_ConvertDeviceId(userNetworkBuffer[i_pkt].eSrcDeviceId) == i_node)
            {
                if(FLG_seq_num_rollback_period[i_node]) //if rollback status
                {
                    pkt_seq_buffer[i_node][pkt_valid_index[i_node]++] = userNetworkBuffer[i_pkt].iSequenceNumber + 0x10000;
                    temp_drvd_pkt[i_node]++;
                }
                else                                    //not rollback status
                {
                    pkt_seq_buffer[i_node][pkt_valid_index[i_node]++] = userNetworkBuffer[i_pkt].iSequenceNumber;
                    temp_drvd_pkt[i_node]++;
                }
                
            }
        }

        // Step 3 : SEQ_NUM into the rollback action
        if((FLG_seq_num_rollback_period[i_node]==0) && (pkt_seq_buffer[i_node][0]>0xFFFFF000))
        {
            for(i=0; i<pkt_valid_index[i_node]; i++)
            {
                pkt_seq_buffer[i_node][i] += 0x10000;
            }
            if (pkt_valid_seq[i_node])  pkt_valid_seq[i_node] += 0x10000;
            FLG_seq_num_rollback_period[i_node]=1;
        }

        // Step 4 : SEQ_NUM out from the rollback action
        if((FLG_seq_num_rollback_period[i_node]==1) && (pkt_seq_buffer[i_node][0]>0x10100))
        {
            for(i=0; i<pkt_valid_index[i_node]; i++)
            {
                pkt_seq_buffer[i_node][i] -= 0x10000;
            }
            if (pkt_valid_seq[i_node])  pkt_valid_seq[i_node] -= 0x10000;
            FLG_seq_num_rollback_period[i_node]=0;
        }

        // Step 5 : Calculate instant start time PDR
        if(FLG_pdr_seq_initial[i_node]==0)
        {
            //Current PDR calculation
            ADK_DEMO.NODE[i_node].PDR[0] = ((float)temp_drvd_pkt[i_node]) / ((float)BMS_PACKETS_PER_INTERVAL) * 100;
            if(ADK_DEMO.NODE[i_node].PDR[0]>100.0)  ADK_DEMO.NODE[i_node].PDR[0] = 100.0;
            //Minimum PDR init
            ADK_DEMO.NODE[i_node].PDR[1] = 100.0;
            //Maximum PDR init
            ADK_DEMO.NODE[i_node].PDR[2] = 0.0;
            //DUP init
            ADK_DEMO.NODE[i_node].DUPLICATE[1] = 0;
        }

        // Step 6 : When buffer index exceeds pkt_valid_index_threshold,
        if(pkt_valid_index[i_node]>pkt_valid_index_threshold)
        {
            // Step 6-1 : duplicated packet count and descending sort
            k=pkt_valid_index[i_node];
            for(i=0;i<(k-1);i++)
            {
                for(j=i+1;j<k;j++)
                {
                    if(pkt_seq_buffer[i_node][i]==pkt_seq_buffer[i_node][j])        //same value
                    {
                        // Step 6-1-a : Duplication detect
                        if(pkt_seq_buffer[i_node][i])                               //same value && none zero
                        {
                            pkt_seq_buffer[i_node][j]=0;                            //delete duplicated
                            pkt_valid_index[i_node]--;                              //declease index
                            pdr_DUP_buffer[i_node]++;
                        }
                    }
                    else if(pkt_seq_buffer[i_node][i]<pkt_seq_buffer[i_node][j])
                    {
                        // Step 6-1-b : Sorting
                        temp_buffer = pkt_seq_buffer[i_node][i];
                        pkt_seq_buffer[i_node][i] = pkt_seq_buffer[i_node][j];
                        pkt_seq_buffer[i_node][j] = temp_buffer;
                    }
                }
            }
            
            // Step 6-2 : Assign sequence start number
            if(FLG_pdr_seq_initial[i_node]==0)
            {
                pkt_valid_seq[i_node] = pkt_seq_buffer[i_node][pkt_valid_index[i_node]-1] - 1;
                FLG_pdr_seq_initial[i_node]=1;
            }
        
            // Step 6-3 : Loop until buffer < 4 , count OK/NG cases
            while(pkt_valid_index[i_node]>pkt_valid_index_threshold)
            {
                //Rollback moment
                if((pkt_valid_seq[i_node]==0xFFFF) && (FLG_seq_num_rollback_period[i_node]==1))
                {
                    pkt_valid_seq[i_node]++;
                }
                
                //OK case   [expected value] == [acquired smalest value]
                if((pkt_valid_seq[i_node] + 1) == pkt_seq_buffer[i_node][pkt_valid_index[i_node]-1])
                {
                    pkt_seq_buffer[i_node][pkt_valid_index[i_node]-1]=0;
                    pkt_valid_index[i_node]--;
                    temp_PDR_OK[i_node]++;
                    temp_PDR_TOTAL[i_node]++;

                    pkt_valid_seq[i_node]++;
                }

                //Upset case    [expected value] > [acquired smallest value]    <- "NEVER happen case"
                else if((pkt_valid_seq[i_node] + 1) > pkt_seq_buffer[i_node][pkt_valid_index[i_node]-1])
                {
                    pkt_seq_buffer[i_node][pkt_valid_index[i_node]-1]=0;
                    pkt_valid_index[i_node]--;
                    temp_PDR_TOTAL[i_node]++;
                }
                //NG(=packet lost) case
                else    
                {
                    pkt_valid_seq[i_node]++;
                    temp_PDR_TOTAL[i_node]++;
                }
            }
            accm_PDR_OK[i_node] += temp_PDR_OK[i_node];
            accm_PDR_TOTAL[i_node] += temp_PDR_TOTAL[i_node];

            if((accm_PDR_OK[i_node]&0x8000000000000000)&&((accm_PDR_TOTAL[i_node]&0x8000000000000000)))
            {
                accm_PDR_OK[i_node]/=2;
                accm_PDR_TOTAL[i_node]/=2;
            }
            
            // Step 6-4 : Current PDR = OK / (OK + NG) -> Display
            ADK_DEMO.NODE[i_node].PDR[0] = ((float)temp_PDR_OK[i_node]) / ((float)temp_PDR_TOTAL[i_node]) * 100.0;
            ADK_DEMO.NODE[i_node].PDR[3] = ((float)accm_PDR_OK[i_node]) / ((float)accm_PDR_TOTAL[i_node]) * 100.0;

            if(ADK_DEMO.NODE[i_node].PDR[0] < 10)
            {
                memset(pkt_seq_buffer[i_node], 0, sizeof(uint32_t) * PDR_pktbuf_depth);
            }
            //Minimum PDR of current PDR calculation
            if(ADK_DEMO.NODE[i_node].PDR[0]<ADK_DEMO.NODE[i_node].PDR[1])   ADK_DEMO.NODE[i_node].PDR[1] = ADK_DEMO.NODE[i_node].PDR[0];
            //Maximum PDR of current PDR calculation
            if(ADK_DEMO.NODE[i_node].PDR[0]>ADK_DEMO.NODE[i_node].PDR[2])   ADK_DEMO.NODE[i_node].PDR[2] = ADK_DEMO.NODE[i_node].PDR[0];

            // Step 6-5 : Current DUP -> Display
            ADK_DEMO.NODE[i_node].DUPLICATE[0] = pdr_DUP_buffer[i_node];
            if(ADK_DEMO.NODE[i_node].DUPLICATE[0]>ADK_DEMO.NODE[i_node].DUPLICATE[1]) ADK_DEMO.NODE[i_node].DUPLICATE[1]=ADK_DEMO.NODE[i_node].DUPLICATE[0];
        }   
        // Step 7 : PDR=0 when Node_disconnected
        if(ADK_DEMO.NODE[i_node].BMS_PKT_MAP[0]==0)
        {
            ADK_DEMO.NODE[i_node].PDR[0]=0;
            ADK_DEMO.NODE[i_node].PDR[1]=0;
            ADK_DEMO.NODE[i_node].PDR[2]=0;
            ADK_DEMO.NODE[i_node].PDR[3]=0;
            FLG_pdr_seq_initial[i_node]=0;
            memset(pkt_seq_buffer[i_node], 0, sizeof(uint32_t) * PDR_pktbuf_depth);
            pkt_valid_index[i_node] = 0;
        }
    }
}

static void adi_wil_example_ADK_RSSI_calc(void){

    bool FLG_rssi_initial = false;
    uint8_t temp_cnt = 0;
    // uint8_t temp_cnt1 = 0;
    uint8_t temp_mngr[ADK_MAX_node * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
    uint8_t temp_node[ADK_MAX_node * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
    int8_t  temp_rssi[ADK_MAX_node * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];

    if(FLG_rssi_initial == false){
        /* Step 0 : Init temp */
        memset(&temp_mngr[temp_cnt], 0, sizeof(uint8_t) * ADK_MAX_node * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL);
        memset(&temp_rssi[temp_cnt], 0, sizeof(uint8_t) * ADK_MAX_node * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL);
        FLG_rssi_initial = true;
    }
    else{   /* This code goes after 1st entry */    }

    /* Step 1 : Read data from "userNetworkBuffer" */
    for(temp_cnt = 0; temp_cnt < NODE_NUM * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL; temp_cnt++){
        temp_mngr[temp_cnt] = ADK_ConvertDeviceId(userNetworkBuffer[temp_cnt].eSrcManagerId);
        temp_node[temp_cnt] = ADK_ConvertDeviceId(userNetworkBuffer[temp_cnt].eSrcDeviceId);
        temp_rssi[temp_cnt] = userNetworkBuffer[temp_cnt].iRSSI;
    }

    /* Step 2 : Data relocation */
    for(temp_cnt = 0; temp_cnt < NODE_NUM * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL; temp_cnt++)
    {
        if(temp_mngr[temp_cnt] == 240){                 /* ADI_WIL_DEV_MANAGER_0 */
            //adi_wil_example_ADK_RSSI_save(temp_node[temp_cnt], ADI_WIL_DEV_MANAGER_0, temp_rssi[temp_cnt]);
            ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][0] = temp_rssi[temp_cnt];
            temp_rssi_rx[temp_node[temp_cnt]][0] += temp_rssi[temp_cnt];
            if(temp_count[temp_node[temp_cnt]][0] != 0)
            {
                if(ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][1] > temp_rssi[temp_cnt])
                {
                    ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][1] = temp_rssi[temp_cnt];
                }
                if(ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][2] < temp_rssi[temp_cnt])
                {
                    ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][2] = temp_rssi[temp_cnt];
                }
            }
            else if (temp_count[temp_node[temp_cnt]][0] == 0)
            {
                ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][2] = temp_rssi[temp_cnt];
            }
            temp_count[temp_node[temp_cnt]][0]++;
            ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[0][3] = (int8_t)(temp_rssi_rx[temp_node[temp_cnt]][0] / temp_count[temp_node[temp_cnt]][0]);
        }
        else if(temp_mngr[temp_cnt] == 241) {           /* ADI_WIL_DEV_MANAGER_1 */
            //adi_wil_example_ADK_RSSI_save(temp_node[temp_cnt], ADI_WIL_DEV_MANAGER_1, temp_rssi[temp_cnt]);
            ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][0] = temp_rssi[temp_cnt];
            temp_rssi_rx[temp_node[temp_cnt]][1] += temp_rssi[temp_cnt];
            if(temp_count[temp_node[temp_cnt]][1] != 0)
            {
                if(ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][1] > temp_rssi[temp_cnt])
                {
                    ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][1] = temp_rssi[temp_cnt];
                }
                if(ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][2] < temp_rssi[temp_cnt])
                {
                    ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][2] = temp_rssi[temp_cnt];
                }
            }
            else if (temp_count[temp_node[temp_cnt]][1] == 0)
            {
                ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][2] = temp_rssi[temp_cnt];
            }
            temp_count[temp_node[temp_cnt]][1]++;
            ADK_DEMO.NODE[temp_node[temp_cnt]].RSSI[1][3] = (int8_t)(temp_rssi_rx[temp_node[temp_cnt]][1] / temp_count[temp_node[temp_cnt]][1]);
        }
        else{/* DO NOTHING */}
    }

    for(temp_cnt = 0; temp_cnt < NODE_NUM ; temp_cnt++)
    {
        //RSSI=0 when Node_disconnected
        if(ADK_DEMO.NODE[temp_cnt].BMS_PKT_MAP[0]==0)
        {
            ADK_DEMO.NODE[temp_cnt].RSSI[0][0]=0x80;
            ADK_DEMO.NODE[temp_cnt].RSSI[0][1]=0;
            ADK_DEMO.NODE[temp_cnt].RSSI[0][2]=0x80;
            ADK_DEMO.NODE[temp_cnt].RSSI[0][3]=0x80;

            ADK_DEMO.NODE[temp_cnt].RSSI[1][0]=0x80;
            ADK_DEMO.NODE[temp_cnt].RSSI[1][1]=0;
            ADK_DEMO.NODE[temp_cnt].RSSI[1][2]=0x80;
            ADK_DEMO.NODE[temp_cnt].RSSI[1][3]=0x80;
        }
    }
}



/******************************************************************************
 * API notification callback
 *****************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteResetDevice(adi_wil_pack_t * const pPack, adi_wil_device_t eDeviceId)
{
    //adi_wil_ex_info("Resetting Device = %s", adi_wil_DeviceToString(eDeviceId));

    logAPIInProgress = ADI_WIL_API_RESET_DEVICE;
    returnOnWilError(adi_wil_ResetDevice(pPack, eDeviceId));
    
    /* Wait for non-blocking API to complete */
    WaitForWilAPI(pPack);

    //adi_wil_ex_info("Reset device %s successful!", adi_wil_DeviceToString(eDeviceId));

    return ADI_WIL_ERR_SUCCESS;
}

adi_wil_err_t Cmic_RequestResetDevice(adi_wil_pack_t * const pPack, adi_wil_device_t eDeviceId)
{
	
	adi_wil_err_t errorCode;
		
	errorCode = adi_wil_ResetDevice(pPack, eDeviceId);
    
    return errorCode;
}


#ifdef _ADI_ONLY
/******************************************************************************
 * API notification callback
 *****************************************************************************/
void adi_wil_HandleCallback (adi_wil_pack_t const * const pPack,
                             void const * const pClientData,
                             adi_wil_api_t eAPI,
                             adi_wil_err_t rc,
                             void const * const pData)
{
    /* For readability, cast the void pointer to app's client data type */
    client_data_t * appClientData = (client_data_t *)pClientData;
    
    /* Handle API generated data and flags here */
    if (pData != NULL)
    {
        /* Handle API generated data and flags here */
        if (eAPI == ADI_WIL_API_LOAD_FILE)
        {
            if (rc == ADI_WIL_ERR_IN_PROGRESS)
            {
                (void)memcpy(&appClientData->LoadFileStatus, pData, sizeof(adi_wil_loadfile_status_t));
            }
        }
        #ifdef DBG_GET_FILE_TEST
        else if (eAPI == ADI_WIL_API_GET_FILE)
        {
            if ( (rc == ADI_WIL_ERR_IN_PROGRESS) || (rc == ADI_WIL_ERR_SUCCESS) )
            {
                (void)memcpy(&getFileInformation, pData, sizeof(getFileInformation));
                (void)memcpy(&userGetFileContainer[getfile_index], getFileInformation.pData, getFileInformation.iByteCount);
                getfile_index += getFileInformation.iByteCount;
            }
        }
        #endif
        else if (eAPI == ADI_WIL_API_GET_DEVICE_VERSION)
        {
            (void)memcpy(&appClientData->DeviceVersion, pData, sizeof(adi_wil_dev_version_t));
        }
        else if (eAPI == ADI_WIL_API_GET_FILE_CRC)
        {
            (void)memcpy(&crclist, pData, sizeof(adi_wil_file_crc_list_t));
        }
        else if (eAPI == ADI_WIL_API_GET_CONTEXTUAL_DATA)
        {
            (void)memcpy(&returnedContextualData, pData, sizeof(returnedContextualData));
        }
        else if (eAPI == ADI_WIL_API_GET_GPIO)
        {
            (void)memcpy(&returnedGpioValue, pData, sizeof(returnedGpioValue));
        }
        else if (eAPI == ADI_WIL_API_GET_STATE_OF_HEALTH)
        {
            (void)memcpy(&returnedSoHValue, pData, sizeof(returnedSoHValue));
        }
        else if (eAPI == ADI_WIL_API_GET_ACL)
        {
            (void)memcpy(&systemAcl, pData, sizeof(adi_wil_acl_t));
        }
        else if (eAPI == ADI_WIL_API_CONNECT)
        {
            (void)memcpy(&appClientData->ConnectionDetails, pData, sizeof(adi_wil_connection_details_t));
            if(*((uint32_t*)pData) == pPack->pInternals->ConnectState.iManager0ConfigurationHash)
            {
                bConnectApiCalled = true;
            }
        }
    }
    gNotifRc = rc;
}
#endif

uint8_t init_pkt_received [48u];
void adi_wil_example_readInitPacket(void)
{
    uint8_t pkt_count = 0;
    uint8_t BMS_packetId = 0;
    uint8_t BMS_eNode = 100;
    adi_bms_init_pkt_0_t* BMS_Packet_0_Ptr;
    int i;
    memset(init_pkt_received, 0, sizeof(init_pkt_received));

    for(pkt_count = 0; pkt_count < nTotalPcktsRcvd; pkt_count++){
        if (userBMSBuffer[pkt_count].iLength == 0) continue; // Adele: skip processing of empty packet
        BMS_eNode = ADK_ConvertDeviceId(userBMSBuffer[pkt_count].eDeviceId);
        BMS_packetId = userBMSBuffer[pkt_count].Data[0];
        
        #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
        if(BMS_packetId == ADI_BMS_INIT_PKT_0_ID)
        #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
        if((BMS_packetId == ADI_BMS_INIT_PKT_0_ID) || (BMS_packetId == ADI_BMS_INIT_PKT_1_ID) || (BMS_packetId == ADI_BMS_INIT_PKT_2_ID))
        #else
        #endif
        {
            init_pkt_received[BMS_eNode]++;
            BMS_Packet_0_Ptr = (adi_bms_init_pkt_0_t*) &userBMSBuffer[pkt_count].iLength;

            #if   (ADK_ADBMS683x == 0) /* ADBMS6830 */
            /* Not Supported */

            #elif(ADK_ADBMS683x == 3) /* ADBMS6833 */

            if(BMS_Packet_0_Ptr->Rdaca.iAc2v[1] == 0) {continue;}
            temp_buffer[0]  = (signed short int)((BMS_Packet_0_Ptr->Rdaca.iAc2v[1]   << 8) + BMS_Packet_0_Ptr->Rdaca.iAc2v[0]);
            temp_buffer[1]  = (signed short int)((BMS_Packet_0_Ptr->Rdaca.iAc3v[1]   << 8) + BMS_Packet_0_Ptr->Rdaca.iAc3v[0]);
            temp_buffer[2]  = (signed short int)((BMS_Packet_0_Ptr->Rdacb.iAc4v[1]   << 8) + BMS_Packet_0_Ptr->Rdacb.iAc4v[0]);
            temp_buffer[3]  = (signed short int)((BMS_Packet_0_Ptr->Rdacb.iAc5v[1]   << 8) + BMS_Packet_0_Ptr->Rdacb.iAc5v[0]);
            temp_buffer[4]  = (signed short int)((BMS_Packet_0_Ptr->Rdacb.iAc6v[1]   << 8) + BMS_Packet_0_Ptr->Rdacb.iAc6v[0]);
            temp_buffer[5]  = (signed short int)((BMS_Packet_0_Ptr->Rdacc.iAc7v[1]   << 8) + BMS_Packet_0_Ptr->Rdacc.iAc7v[0]);
            temp_buffer[6]  = (signed short int)((BMS_Packet_0_Ptr->Rdacc.iAc8v[1]   << 8) + BMS_Packet_0_Ptr->Rdacc.iAc8v[0]);
            temp_buffer[7]  = (signed short int)((BMS_Packet_0_Ptr->Rdacc.iAc9v[1]   << 8) + BMS_Packet_0_Ptr->Rdacc.iAc9v[0]);
            temp_buffer[8]  = (signed short int)((BMS_Packet_0_Ptr->Rdacd.iAc11v[1]  << 8) + BMS_Packet_0_Ptr->Rdacd.iAc11v[0]);
            temp_buffer[9]  = (signed short int)((BMS_Packet_0_Ptr->Rdacd.iAc12v[1]  << 8) + BMS_Packet_0_Ptr->Rdacd.iAc12v[0]);
            temp_buffer[10] = (signed short int)((BMS_Packet_0_Ptr->Rdace.iAc13v[1]  << 8) + BMS_Packet_0_Ptr->Rdace.iAc13v[0]);
            temp_buffer[11] = (signed short int)((BMS_Packet_0_Ptr->Rdace.iAc14v[1]  << 8) + BMS_Packet_0_Ptr->Rdace.iAc14v[0]);
            temp_buffer[12] = (signed short int)((BMS_Packet_0_Ptr->Rdace.iAc15v[1]  << 8) + BMS_Packet_0_Ptr->Rdace.iAc15v[0]);
            temp_buffer[13] = (signed short int)((BMS_Packet_0_Ptr->Rdacf.iAc16v[1]  << 8) + BMS_Packet_0_Ptr->Rdacf.iAc16v[0]);
            temp_buffer[14] = (signed short int)((BMS_Packet_0_Ptr->Rdacf.iAc17v[1]  << 8) + BMS_Packet_0_Ptr->Rdacf.iAc17v[0]);
            temp_buffer[15] = (signed short int)((BMS_Packet_0_Ptr->Rdacf.iAc18v[1]  << 8) + BMS_Packet_0_Ptr->Rdacf.iAc18v[0]);
            temp_buffer[16] = (signed short int)((BMS_Packet_0_Ptr->Rdauxa.iG1v[1]   << 8) + BMS_Packet_0_Ptr->Rdauxa.iG1v[0]);
            temp_buffer[17] = (signed short int)((BMS_Packet_0_Ptr->Rdauxa.iG2v[1]   << 8) + BMS_Packet_0_Ptr->Rdauxa.iG2v[0]);
            temp_buffer[18] = (signed short int)((BMS_Packet_0_Ptr->Rdauxa.iGa11v[1] << 8) + BMS_Packet_0_Ptr->Rdauxa.iGa11v[0]);
            temp_buffer[19] = (signed short int)((BMS_Packet_0_Ptr->Rdauxe.iG3v[1]   << 8) + BMS_Packet_0_Ptr->Rdauxe.iG3v[0]);
            #else   /* Not supported */
            #endif
            for(i = 0; i<16; i++){
                /* @remark : Cell voltage convert (float) */
                ADK_DEMO.NODE[BMS_eNode].CELL_V[i]  = (float)(temp_buffer[i]  * CELL_UNIT / 1000000.0f) + CELL_OFFSET;

                /* @remark : Cell voltage convert (int16) */
                ADK_DEMO.NODE[BMS_eNode].CELL_Vi[i]  = temp_buffer[i]   + 10000;
            }

            for(i = 0; i<4; i++){
                /* @remark : AUX voltage convert (float) */
                ADK_DEMO.NODE[BMS_eNode].TEMP_V[i]  = (float)(temp_buffer[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : AUX voltage convert (int16) */
                ADK_DEMO.NODE[BMS_eNode].TEMP_Vi[i]  = temp_buffer[i + 16] + 10000;  

            }

        }
    }
}




#ifdef _ADI_ONLY
/******************************************************************************
 * Event notification callback
 *****************************************************************************/
void adi_wil_HandleEvent (adi_wil_pack_t const * const pPack,
                          void const * const pClientData,
                          adi_wil_event_id_t EventCode,
                          void const * const pData)
{
    uint16_t iCount;
    uint8_t HR_index=0;
    #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    bool bLatent0 = false;
    bool bLatent1 = false;
    uint8_t BMS_eNode = 100;
    #else
    #endif

    /* For readability, cast the void pointer to app's client data type */
    client_data_t * appClientData = (client_data_t *)pClientData;

    if (appClientData->eEventWaitingFor == EventCode)
    {
        appClientData->iEventWaitingForCount++;
    }
    else
    {
        /* code flow should never enter here */
    }

    switch (EventCode)
    {
        case ADI_WIL_EVENT_COMM_MGR_CONNECTED:
            iMgrConnectCount++;
            break;

        case ADI_WIL_EVENT_COMM_MGR_DISCONNECTED:
            iMgrDisconnectCount++;
            break;

        case ADI_WIL_EVENT_DATA_READY_PMS:
            /* Handle PMS data here in the event CB from pData */
            /* The application should signal here to let the PMS data processing routine know that there is new PMS data. */
            iPMSNotificationCount++;
            /* If somehow WIL passes more sensor data packets than example code can store, store as much data as fits. */
            iCount = ((adi_wil_sensor_data_buffer_t *)pData)->iCount;
            if (iCount > PMS_DATA_PACKET_COUNT)
            {
                iCount = PMS_DATA_PACKET_COUNT;
            }
            /* copy the PMS data out here in the event CB from pData */
            memcpy(userPMSBuffer, ((adi_wil_sensor_data_buffer_t *)pData)->pData, iCount*(sizeof(adi_wil_sensor_data_t)));
            adi_gNotifyPms = true;
            break;

        case ADI_WIL_EVENT_DATA_READY_BMS:
            iBMSNotificationCount++;
            /* copy the BMS data out here in the event CB from pData */
            memcpy(userBMSBuffer, ((adi_wil_sensor_data_buffer_t *)pData)->pData, (((adi_wil_sensor_data_buffer_t *)pData)->iCount)*(sizeof(adi_wil_sensor_data_t)));
            /* The application should signal here to let the BMS data processing thread know that there is new BMS data. */
            adi_gNotifyBms = true;
            adi_gNotifyBmsData = true;
            /* @remark : Read BMS data */
            adi_wil_example_ADK_ExecuteProcessBMSBuffer();
            nTotalPcktsRcvd = ((adi_wil_sensor_data_buffer_t *)pData)->iCount;  /*  @remark Akash : Variable to store total no. of bms packets received */

            /* @remark  : check BASE packet header */
            for(EventCallback_pkt_count = 0; EventCallback_pkt_count < nTotalPcktsRcvd; EventCallback_pkt_count++){
                #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
                #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
                BMS_eNode = ADK_ConvertDeviceId(userBMSBuffer[EventCallback_pkt_count].eDeviceId);
                if (userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_LATENT0_PKT_0_ID){
                    bLatent0 = true;
                    latent0_recv_confirm[BMS_eNode] |= 0x01;
                    memcpy(&Latency0_Packet_0, (adi_bms_latent0_pkt_0_t*)userBMSBuffer[EventCallback_pkt_count].Data, sizeof(adi_bms_packetheader_t));
                    memcpy(&Latency0_Packet_0.Rdstatc, (adi_bms_latent0_pkt_0_t*)&userBMSBuffer[EventCallback_pkt_count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_0_t) - 8);
                }
                else if (userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_LATENT0_PKT_1_ID){
                    bLatent0 = true;
                    latent0_recv_confirm[BMS_eNode] |= 0x02;
                    memcpy(&Latency0_Packet_1, (adi_bms_latent0_pkt_1_t*)userBMSBuffer[EventCallback_pkt_count].Data, sizeof(adi_bms_packetheader_t));
                    memcpy(&Latency0_Packet_1.Rdpwma_2, (adi_bms_latent0_pkt_1_t*)&userBMSBuffer[EventCallback_pkt_count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_1_t) - 8);
                }
                else if (userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_LATENT0_PKT_2_ID){
                    bLatent0 = true;
                    latent0_recv_confirm[BMS_eNode] |= 0x04;
                    memcpy(&Latency0_Packet_2, (adi_bms_latent0_pkt_2_t*)userBMSBuffer[EventCallback_pkt_count].Data, sizeof(adi_bms_packetheader_t));
                    memcpy(&Latency0_Packet_2.Rdace, (adi_bms_latent0_pkt_2_t*)&userBMSBuffer[EventCallback_pkt_count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_2_t) - 8);
                }
                else if (userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_LATENT1_PKT_0_ID){
                    bLatent1 = true;
                    latent1_recv_confirm[BMS_eNode] |= 0x01;
                    memcpy(&Latency1_Packet_0, (adi_bms_latent1_pkt_0_t*)userBMSBuffer[EventCallback_pkt_count].Data, sizeof(adi_bms_packetheader_t));
                    memcpy(&Latency1_Packet_0.Rdaca, (adi_bms_latent1_pkt_0_t*)&userBMSBuffer[EventCallback_pkt_count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_0_t) - 8);
                }
                else if (userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_LATENT1_PKT_1_ID){
                    bLatent1 = true;
                    latent1_recv_confirm[BMS_eNode] |= 0x02;
                    memcpy(&Latency1_Packet_1, (adi_bms_latent1_pkt_1_t*)userBMSBuffer[EventCallback_pkt_count].Data, sizeof(adi_bms_packetheader_t));
                    memcpy(&Latency1_Packet_1.Rdsvd, (adi_bms_latent1_pkt_1_t*)&userBMSBuffer[EventCallback_pkt_count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_1_t) - 8);
                }
                #else   /* Not supported */
                #endif

                /* capture Init script data */
                #ifdef DBG_INIT_SCRIPT_TEST
                if(userBMSBuffer[EventCallback_pkt_count].iLength == 0)
                {
                    iLostPacketCnt++;
                }
                else if((userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_INIT_PKT_0_ID) && (iInitBuffer0index < userAcl.iCount))
                {
                    iBMSNotificationCount = 0; // Adele: to init BMS_PKT_MAP [1]
                    memcpy(&initBuffer0[iInitBuffer0index++], &userBMSBuffer[EventCallback_pkt_count].Data, userBMSBuffer[EventCallback_pkt_count].iLength);
                    adi_wil_example_readInitPacket();
                    if (!bFirstBMSdata)  adk_debug_BootTimeLog(Overall_, LogEnd__, 730, Demo_key_on_event_____________________);
                    bFirstBMSdata=true;
                }
                else if((userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_INIT_PKT_1_ID) && (iInitBuffer1index < ADK_MAX_node))
                {
                    memcpy(&initBuffer1[iInitBuffer1index++], &userBMSBuffer[EventCallback_pkt_count].Data, userBMSBuffer[EventCallback_pkt_count].iLength);
                    if (!bFirstBMSdata)  adk_debug_BootTimeLog(Overall_, LogEnd__, 730, Demo_key_on_event_____________________);
                    bFirstBMSdata=true;
                }
                else if((userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_INIT_PKT_2_ID) && (iInitBuffer2index < ADK_MAX_node))
                {
                    memcpy(&initBuffer2[iInitBuffer2index++], &userBMSBuffer[EventCallback_pkt_count].Data, userBMSBuffer[EventCallback_pkt_count].iLength);
                    if (!bFirstBMSdata)  adk_debug_BootTimeLog(Overall_, LogEnd__, 730, Demo_key_on_event_____________________);
                    bFirstBMSdata=true;
                }
                #endif
                if(userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_BASE_PKT_0_ID || userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_BASE_PKT_1_ID || userBMSBuffer[EventCallback_pkt_count].Data[0] == ADI_BMS_BASE_PKT_2_ID){
                       BASE_PACKET_RECEIVED = true;
                       if (!bFirstBMSdata)  adk_debug_BootTimeLog(Overall_, LogEnd__, 730, Demo_key_on_event_____________________);
                       bFirstBMSdata=true;
                       break;
                   }
                else{
                    BASE_PACKET_RECEIVED = false;
                }
            }
            
            #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
            #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
            for(EventCallback_pkt_count = 0; EventCallback_pkt_count < NODE_NUM; EventCallback_pkt_count++)
            {
                if(latent0_recv_confirm[EventCallback_pkt_count] != 0x07)
                {
                    bLatent0 = false;
                }
                if(latent1_recv_confirm[EventCallback_pkt_count] != 0x03)
                {
                    bLatent1 = false;
                }
            }
            if(bLatent0)
            {
                adi_gNotifyLatent0 = true;
            }
            if(bLatent1)
            {
                adi_gNotifyLatent1 = true;
            }
            #else   /* Not supported */
            #endif

            break;

        case ADI_WIL_EVENT_DATA_READY_EMS:
            iEMSNotificationCount++;
            /* copy the EMS data out here in the event CB from pData */
            memcpy(userEMSBuffer, ((adi_wil_sensor_data_buffer_t *)pData)->pData, (((adi_wil_sensor_data_buffer_t *)pData)->iCount)*(sizeof(adi_wil_sensor_data_t)));
            /* The application should signal here to let the BMS data processing thread know that there is new BMS data. */
            adi_gNotifyEms = true;
            break;

        case ADI_WIL_EVENT_COMM_NODE_CONNECTED:
            /* A node joined the WBMS network. Connected node device ID (adi_wil_device_t) is returned  */
            /* Counter to track connected nodes in the network */
            iNodeConnectedCount++;
            iDisconn_node[ADK_ConvertDeviceId(*((uint8_t*)pData))] = false;
            break;

        case ADI_WIL_EVENT_COMM_NODE_DISCONNECTED:
            /* A node dropped from the WBMS network. Disconnected node device ID (adi_wil_device_t) is returned */
            /* Counter to track disconnected nodes in the network */
            iNodeDisconnectedCount++;
            uint8_t Node_index = ADK_ConvertDeviceId(*((uint8_t*)pData));
            iDisconn_node[Node_index] = true;
            ADK_DEMO.NODE[Node_index].BMS_PKT_MAP[0] = 0;
            for(uint8_t i = 0; i < 4; i++)
            {
                ADK_DEMO.NODE[Node_index].PDR[i]=0;
            }
            FLG_pdr_seq_initial[Node_index]=0;
            pkt_valid_index[Node_index] = 0;
            accm_PDR_OK[Node_index] = 0;
            accm_PDR_TOTAL[Node_index] = 0;
            memset(pkt_seq_buffer[Node_index], 0, sizeof(uint32_t) * PDR_pktbuf_depth);
            break;

        case ADI_WIL_EVENT_COMM_MGR_TO_MGR_ERROR:
            /* Communication between the two network managers failed (dual manager mode only), no data is returned */
            /* Counter to track MGR to MGR errors */
            iMgrToMgrErrCount++;
            break;
        
        case ADI_WIL_EVENT_MGR_QUEUE_OVERFLOW:
            /* Event used to identified SPI transmission queue issues in the managers */
            iQueueOverflowNotificationCount++;
            break;
        
        case ADI_WIL_EVENT_DATA_READY_HEALTH_REPORT:
            /* copy the HR data out here in the event CB from pData */
            if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_DEVICE)
            {
                /* HR Data Packet1 : contains signal rssi, reset counter
                twohop counter, join attempts etc */
                memcpy(&userHR0Buffer[ADK_ConvertDeviceId(((adi_wil_health_report_t *)pData)->eDeviceId)], 
                       &((adi_wil_health_report_t*)pData)->Data[0], 
                       sizeof(adi_wil_health_report0_t));
                // adi_w2can_NotifyHR_availability();   /* @remark: Not used this time */
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_AVERAGE_RSSI)
            {
                /* HR Data Packet2 : contains node average rssi */
                memcpy(&userHR1Buffer[ADK_ConvertDeviceId(((adi_wil_health_report_t *)pData)->eDeviceId)], 
                       &((adi_wil_health_report_t*)pData)->Data[0], 
                       sizeof(adi_wil_health_report1_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_BACKGROUND_RSSI)
            {
                /* HR Data Packet2 : contains background rssi */
                memcpy(&userHR2Buffer[ADK_ConvertDeviceId(((adi_wil_health_report_t *)pData)->eDeviceId)], 
                       &((adi_wil_health_report_t*)pData)->Data[0], 
                       sizeof(adi_wil_health_report2_t));
                // adi_wil_example_ADK_BGRSSI_calc();   /* @remark: Not used this time */
                // adi_w2can_NotifyHR_availability();   /* @remark: Not used this time */
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_MANAGER_APPLICATION)
            {
                /* HR Data Packet 0x10 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR10Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report10_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_MANAGER_BACKGROUND_RSSI)
            {
                /* HR Data Packet 0x11 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR11Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report11_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_MANAGER_DEVICE)
            {
                /* HR Data Packet 0x12 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR12Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report12_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_MANAGER_0_4_RSSI)
            {
                /* HR Data Packet 0x14 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR14Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report14_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_MANAGER_5_9_RSSI)
            {
                /* HR Data Packet 0x15 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR15Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report14_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_MANAGER_10_14_RSSI)
            {
                /* HR Data Packet 0x16 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR16Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report14_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_MANAGER_15_19_RSSI)
            {
                /* HR Data Packet 0x17 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR17Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report14_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_MANAGER_20_23_RSSI)
            {
                /* HR Data Packet 0x18 :  */
                if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_0)
                {
                    HR_index = 0;
                }
                else if(((adi_wil_health_report_t *)pData)->eDeviceId == ADI_WIL_DEV_MANAGER_1)
                {
                    HR_index = 1;
                }
                else
                {
                    break;
                }
                memcpy(&userHR18Buffer[HR_index],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report14_t));
            }
            else if (((adi_wil_health_report_t *)pData)->Data[0] == PACKET_ID_NODE_APPLICATION)
            {
                /* HR Data Packet 0x80 : contains data about node firmware */
                memcpy(&userHR80Buffer[ADK_ConvertDeviceId(((adi_wil_health_report_t *)pData)->eDeviceId)],
                       &((adi_wil_health_report_t*)pData)->Data[0],
                       sizeof(adi_wil_health_report80_t));
                // adi_w2can_NotifyHR_availability();   /* @remark: Not used this time */
            }
            else
            {
                // invalid Data Packet ID - ignore
            }
            break;

        case ADI_WIL_EVENT_DATA_READY_NETWORK_DATA:
            /* Copy out the network data if enabled */
            memcpy(userNetworkBuffer, ((adi_wil_network_data_buffer_t *)pData)->pData, (((adi_wil_network_data_buffer_t *)pData)->iCount)*(sizeof(adi_wil_network_data_t)));
            adi_gNotifyNetworkMeta = true;
            adi_wil_example_ADK_PDR_calc();
            adi_wil_example_ADK_RSSI_calc();
            adi_wil_example_StoreNWDataForPS(userNetworkBuffer);
            break;

        case ADI_WIL_EVENT_FAULT_SOURCES:
            /* A fault in monitor mode has occured and a system summary has been received. */
            /* Copy out the fault sources summary */
            memcpy(&faultSources, pData, sizeof(adi_wil_device_t));
            break;

        case ADI_WIL_EVENT_FAULT_REPORT:
            /* Environmental monitoring fault report has been received. */
            /* Copy out the fault report */
            memcpy(&faultReport, pData, sizeof(adi_wil_fault_report_t));
            adi_gFaultDetected = true;
            break;

        case ADI_WIL_EVENT_SEC_NODE_NOT_IN_ACL:
            /* Node that tried to join the WBMS is not in the ACL list, the MAC
               address of the unidentified node (uint8_t *) is returned*/
        case ADI_WIL_EVENT_SEC_CERTIFICATE_CALCULATION_ERROR:
            /* A certificate calculation error occurred, the MAC address of the
               device (uint8_t *) that triggered the event is returned*/
        case ADI_WIL_EVENT_SEC_JOIN_NO_KEY:
            /* A node does not have a valid join key, the MAC address of the
               device (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_JOIN_DUPLICATE_JOIN_COUNTER:
            /* A node has a duplicate join counter, the MAC address of the
               device (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_JOIN_MIC_FAILED:
            /* MIC verification failed for a join packet, the MAC address of the
               device (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_SESSION_MIC_FAILED:
            /* Session MIC Failure for a session packet, the MAC address of the
               device (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_M2M_JOIN_CNTR_ERROR:
            /* M2M Join Counter Security Error, the MAC address of the peer
               manager (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_M2M_SESSION_CNTR_ERROR:
            /* M2M Session Counter Security Error, the MAC address of the peer
               manager (uint8_t *) that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_UNKNOWN_ERROR:
            /* Unknown security error, the MAC address of the device (uint8_t *)
               that triggered the event is returned */
        case ADI_WIL_EVENT_SEC_CERTIFICATE_EXCHANGE_LOCK_ERROR:
            /* A certificate exchange error occurred, the MAC address of the
               device (uint8_t *) that triggered the event is returned */
            iSecurityEvtCount++;
            break;
        case ADI_WIL_EVENT_XFER_DEVICE_REMOVED:
            memcpy(&LF_NG_LIST[ADK_DEMO.LF_NG_CNT], pData, sizeof(adi_wil_device_removed_t));
            ADK_DEMO.LF_NG_CNT++;
            if( ADK_DEMO.LF_NG_CNT >= MAX_LF_RETRY ){
                /* @warning Load file command failed a lot */
                adk_debug_Report(DBG_need_investigation, ADI_WIL_ERR_FAIL);
            }
            /* This event applies only during the LoadFile API */
            /* pData points to a adi_wil_device_removed_t structure. This data
               can be copied out so that the thread that is executing the
               LoadFile API can determine which device has dropped off. */
            break;
        case ADI_WIL_EVENT_INSUFFICIENT_BUFFER:
            /* The user has provided a adi_wil_sensor_data_t (adi_wil_sensor_data_t)
               buffer that is insufficiently sized for this network */
            /* The correct number of buffers the user should provide is returned
               (uint16_t *). */
            break;
        default: ;
    }
}
#endif

/******************************************************************************
 * MAC <-> index retrieval function
 * This is a helper function that will return to the user either the MAC address or
 * the device ID of a node in the network
 *****************************************************************************/
void adi_wil_mac_deviceID_return(adi_wil_pack_t *pPack, bool bMacReturn, uint8_t *pMacPtr, uint8_t *pDeviceID)
{
    uint8_t iDeviceID = *pDeviceID;
    /* Check MAC return flag to ascertain if MAC address of the node is to be returned or the device ID */
    if(bMacReturn) {
        for(uint8_t i=0;i<ADI_WIL_MAC_ADDR_SIZE;i++) {
            /*Retrieving the MAC address */
            *(pMacPtr+i) = systemAcl.Data[(ADI_WIL_MAC_ADDR_SIZE*iDeviceID)+i];
        }
    }
    else {
        for(uint8_t i=0;i<ADI_WIL_MAX_NODES;i++) {
            bool bMacFound = true;
            for(uint8_t j=0;j<ADI_WIL_MAC_ADDR_SIZE && bMacFound;j++) {
                bMacFound = (systemAcl.Data[(ADI_WIL_MAC_ADDR_SIZE*i)+j] == *(pMacPtr+j));
            }
            if(bMacFound == true) {
                /*Retrieving the Device ID */
                *pDeviceID = i;
                break;
            }
        }
    }
}

/******************************************************************************
 * Read sensor packet count function
 *****************************************************************************/
uint8_t readSensorPacketCount(adi_wil_configuration_t* pPortConfig) {
    uint8_t sensorPacketCount;

    if(pPortConfig[0].iSensorPacketCount != pPortConfig[1].iSensorPacketCount) {
        //adi_wil_ex_info("Network Managers have different sensor packet count!");
        if(pPortConfig[0].iSensorPacketCount > pPortConfig[1].iSensorPacketCount) {
            sensorPacketCount = (uint8_t)pPortConfig[0].iSensorPacketCount;
        }
        else {
            sensorPacketCount = (uint8_t)pPortConfig[1].iSensorPacketCount;
        }
    }
    else {
        sensorPacketCount = (uint8_t)pPortConfig[0].iSensorPacketCount;
    }
    return sensorPacketCount;
}

/******************************************************************************
 * Print Event Statistics
 *****************************************************************************/
void adi_task_EventStatistics(void)
{
    //adi_wil_ex_info("Event Statistics...");
    //printf("ADI_WIL_EVENT_COMM_MGR_CONNECTED = %u\r\n", iMgrConnectCount);
    //printf("ADI_WIL_EVENT_COMM_MGR_DISCONNECTED = %u\r\n", iMgrDisconnectCount);
    //printf("ADI_WIL_EVENT_DATA_READY_BMS = %u\r\n", iBMSNotificationCount);
    //printf("ADI_WIL_EVENT_DATA_READY_PMS = %u\r\n", iPMSNotificationCount);
    //printf("ADI_WIL_EVENT_DATA_READY_EMS = %u\r\n", iEMSNotificationCount);
    //printf("ADI_WIL_EVENT_MGR_QUEUE_OVERFLOW = %u\r\n", iQueueOverflowNotificationCount);
}

/******************************************************************************
 * @remark : Custom functions for development
 *****************************************************************************/
bool adi_wil_example_ADK_CompareACL(void){

    bool ACL_rc = false; /* false = ACL update needed, true = no need to update */

    if(userAcl.iCount == systemAcl.iCount){
        /* iCount is same */
        while(1){ /* Parse user MAC */
            if(userAcl.Data[Compare_cnt * 8] == 0){
                /* @remark : There are no MAC exist in user setting */
                break;
            }
            else{
                /* @remark : Parse node specific MAC */
                User_ACL[User_ACL_cnt] |= userAcl.Data[Compare_cnt * 8 + 5] << 16; //6th MAC
                User_ACL[User_ACL_cnt] |= userAcl.Data[Compare_cnt * 8 + 6] << 8; //7th MAC
                User_ACL[User_ACL_cnt] |= userAcl.Data[Compare_cnt * 8 + 7]; //8th MAC
                User_ACL_cnt ++;
            }
            Compare_cnt++;
        }

        Compare_cnt = 0; // counter re-init
        User_ACL_cnt = 0; // counter re-init

        while(1){ /* Parse read MAC */
            if(systemAcl.Data[Compare_cnt * 8] == 0){
                /* @remark : There are no MAC exist in get ACL setting */
                break;
            }
            else{
                /* @remark : Parse node specific MAC */
                Read_ACL[User_ACL_cnt] |= systemAcl.Data[Compare_cnt * 8 + 5] << 16; //6th MAC
                Read_ACL[User_ACL_cnt] |= systemAcl.Data[Compare_cnt * 8 + 6] << 8; //7th MAC
                Read_ACL[User_ACL_cnt] |= systemAcl.Data[Compare_cnt * 8 + 7]; //8th MAC
                User_ACL_cnt ++;
            }
            Compare_cnt++;
        }

        Compare_cnt = 0; // counter re-init
        User_ACL_cnt = 0; // counter re-init

        /* @remark : Compare MAC */
        for(Compare_cnt = 0; Compare_cnt < 64; Compare_cnt++){
            ACL_rc = false;
            for(User_ACL_cnt = 0; User_ACL_cnt < 64; User_ACL_cnt++){
                if(User_ACL[Compare_cnt] == Read_ACL[User_ACL_cnt]){
                    ACL_rc = true; /* found same MAC */
                    break;
                }
            }
            if(ACL_rc == false) break;
        }
    }
    else{
        /* iCount is different */
        ACL_rc = false;
        if (systemAcl.iCount == 0) ACL_EMPTY = true; //Empty managers case
    }

    return ACL_rc;    
}




/******************************************************************************
 * Inherit from TC275 project
 * @remark : These functions are only for TC387
 *****************************************************************************/
bool Cmic_CallProcessTask(void)
{
	return adi_wil_hal_TaskStart(PROCESS_TASK_INTERVAL_USEC, adi_task_serviceProcessTask);
}


bool adi_wil_example_PeriodicallyCallProcessTask(void)
{
    return adi_wil_hal_TaskStart(PROCESS_TASK_INTERVAL_USEC, adi_task_serviceProcessTask);
}

/* service the adi_wil_ProcessTask() call and record result */
static void adi_task_serviceProcessTask(void)
{
    adi_gProcessTaskErrorCode = adi_wil_ProcessTask(&packInstance);
}

bool adi_wil_example_PeriodicallyCallProcessTaskCB(void)
{
    return adi_wil_hal_TaskCBStart(PROCESS_TASK_CB_INTERVAL_USEC, adi_task_serviceProcessTaskCB);
}

/* service the adi_wil_ProcessTask() call and record result */
static void adi_task_serviceProcessTaskCB(void)
{
    /* Read BMS data goes here */
    adi_task_bmsDataRetrieval();
}

void adk_debug_Report(ADK_FAIL_API_NAME api, adi_wil_err_t rc ){
    ADK_DEMO.DBG_FAIL_API = api;
    ADK_DEMO.DBG_FAIL__RC = rc;
    while(1);
}

/**
 * @brief   Timing measurement for WIL example initialization, saved in BOOT_TIME structure [1ms tick]
 *
 * @param  bool                 Overall_, Interval
 *
 * @param  bool                 LogStart, LogEnd__
 * 
 * @param  uint16_t             ADK_DEMO.BOOT
 * 
 * @param  ADK_LOG_FUNCTION     function name
 *
 * @return None
 */
void adk_debug_BootTimeLog(bool final, bool start, uint16_t step, ADK_LOG_FUNCTION api ){
    if(final == true){
        if(start == true){
            BOOT_TIME[0].timestamp[0] = adk_debug_TickerBTGetTimestamp();
            BOOT_TIME[0].STEP = step;
        }
        else{
            BOOT_TIME[0].timestamp[1] = adk_debug_TickerBTGetTimestamp();
            BOOT_TIME[0].timestamp[2] = BOOT_TIME[0].timestamp[1] - BOOT_TIME[0].timestamp[0];  //result in msec
            BOOT_TIME[0].DURATION = ((float)BOOT_TIME[0].timestamp[2]) / 1000;                  //convert to sec
            BOOT_TIME[0].API = api;
        }
    }
    else{
        if(start == true){
            BOOT_TIME[G_BOOT_TIME_CNT].timestamp[0] = adk_debug_TickerBTGetTimestamp();
            BOOT_TIME[G_BOOT_TIME_CNT].STEP = step;
        }
        else{
            BOOT_TIME[G_BOOT_TIME_CNT].timestamp[1] = adk_debug_TickerBTGetTimestamp();
            BOOT_TIME[G_BOOT_TIME_CNT].timestamp[2] = BOOT_TIME[G_BOOT_TIME_CNT].timestamp[1] - BOOT_TIME[G_BOOT_TIME_CNT].timestamp[0];
            BOOT_TIME[G_BOOT_TIME_CNT].DURATION = ((float)BOOT_TIME[G_BOOT_TIME_CNT].timestamp[2]) / 1000;
            BOOT_TIME[G_BOOT_TIME_CNT].API = api;
            G_BOOT_TIME_CNT++;
        }
    }
}

/**
 * @brief  Covert WIL Device Id to uint8_t
 *
 * @param  adi_wil_device_t[in]      WilDeviceId
 *
 * @return uint8_t[out]              DeviceId
 */

uint8_t ADK_ConvertDeviceId (adi_wil_device_t WilDeviceId)
{
    uint8_t DeviceId = 0;
    
    if(WilDeviceId != 0)
    {
        if (WilDeviceId == ADI_WIL_DEV_MANAGER_0) return 240;
        else if (WilDeviceId == ADI_WIL_DEV_MANAGER_1) return 241;
        else if (WilDeviceId == ADI_WIL_DEV_ALL_MANAGERS) return 254;
        else if (WilDeviceId == ADI_WIL_DEV_ALL_NODES) return 255;
    
        while ((WilDeviceId & (ADI_WIL_DEV_NODE_0 << DeviceId)) == 0)
        {
            if(DeviceId < 255)
            {
                DeviceId++;
            }
            else
            {
                DeviceId = 200;
                break;
            }
        }
    }
    else
    {
        DeviceId = 200;
    }
    return DeviceId;
}



void adk_debug_log_GetDeviceVersion(ADK_OTAP_ARG_0 eDeviceID, ADK_OTAP_ARG_1 update, uint8_t idx)
{
    if(eDeviceID == OTAP_Mngr)
    {
        if(update == OTAP_P0)
        {
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P0_PREV_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P0_PREV_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P0_PREV_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P0_PREV_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
        else if(update == OTAP_P1)
        {
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P1_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P1_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P1_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_MNGR[idx].OTAP_P1_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
        else if(update == OTAP_NOW)
        {
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_MNGR[idx].CURRENT_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
    }
    else if(eDeviceID == OTAP_Node)
    {
        if(update == OTAP_P0)
        {
            ADK_DEMO.OTAP_NODE[idx].OTAP_P0_PREV_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P0_PREV_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P0_PREV_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P0_PREV_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
        else if(update == OTAP_P1)
        {
            ADK_DEMO.OTAP_NODE[idx].OTAP_P1_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P1_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P1_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_NODE[idx].OTAP_P1_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
        else if(update == OTAP_NOW)
        {
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionMajor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMajor;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionMinor = ClientData.DeviceVersion.MainProcSWVersion.iVersionMinor;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionPatch = ClientData.DeviceVersion.MainProcSWVersion.iVersionPatch;
            ADK_DEMO.OTAP_NODE[idx].CURRENT_OPFW_VER.iVersionBuild = ClientData.DeviceVersion.MainProcSWVersion.iVersionBuild;
        }
    }
}

adi_wil_err_t adi_wil_example_otap_QueryDevicePort0(adi_wil_configuration_t * const pConfig)
{
    return adi_wil_example_otap_QueryDeviceCommon(&PortManager0, pConfig, "Port 0");
}

adi_wil_err_t adi_wil_example_otap_QueryDevicePort1(adi_wil_configuration_t * const pConfig)
{
    return adi_wil_example_otap_QueryDeviceCommon(&PortManager1, pConfig, "Port 1");
}

adi_wil_err_t adi_wil_example_otap_QueryDeviceCommon(adi_wil_port_t * const pPort, adi_wil_configuration_t * const pConfig, char const pPortName[])
{
    // char macBuffer[ADI_WIL_MAC_ADDR_SIZE * 3];
    bool callback_success = false;

    adi_wil_err_t rc = adi_wil_QueryDevice(pPort);

    if (rc == ADI_WIL_ERR_SUCCESS)
    {
        /* Wait for non-blocking API to complete */
        pQueryDeviceConfig = pConfig;
        queryDeviceCallbackCalled = false;
        while (!queryDeviceCallbackCalled)  {  /* spin wait */ }

        rc = queryDeviceRc;
        callback_success = (rc == ADI_WIL_ERR_SUCCESS);
    }

    // info("adi_wil_QueryDevice returned result code = %s", adi_wil_ErrorToString(rc));
    if (callback_success)
    {
        gOTAPQueryDeviceRetry = true;
        // adi_wil_SprintMAC(macBuffer, pConfig->MAC);
        // info("  %s MAC = %s", pPortName, macBuffer);
        // adi_wil_SprintMAC(macBuffer, pConfig->PeerMAC);
        // info("  %s PeerMAC = %s", pPortName, macBuffer);
        // info("  %s sensor packet count = %u", pPortName, pConfig->iSensorPacketCount);
        // info("  %s configuraton hash = %u", pPortName, pConfig->iConfigurationHash);
        // info("  %s dual configuration = %s", pPortName, pConfig->bDualConfiguration ? "TRUE" : "FALSE");
    }

    return rc;
}

/******************************************************************************
 * Example function using adi_wil_CheckDeviceVersion
 *****************************************************************************/
void adi_wil_example_GetOTAPVersion(bool bAllNodesJoined)
{
    adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_MANAGER_0, NO_SET_MODE);
    adk_debug_log_GetDeviceVersion(OTAP_Mngr, OTAP_P0, 0);
    adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_NODE_0, NO_SET_MODE);
    adk_debug_log_GetDeviceVersion(OTAP_Node, OTAP_P0, 0);
    
    if(ADK_DEMO.OTAP_MNGR[0].OTAP_P0_PREV_VER.iVersionMajor == 2)
    {
        if(ADK_DEMO.OTAP_MNGR[0].OTAP_P0_PREV_VER.iVersionMinor == 1)
        {
            if((ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMajor == 2) &&
                (ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMinor == 1))
            {
                OTAP_DEVICE_VER = VER_2_1_0_NODE;
            }
            else if((ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMajor == 2) &&
                (ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMinor == 2))
            {
                OTAP_DEVICE_VER = VER_2_1_0_MNG;
            }
            else
            {
                OTAP_DEVICE_VER = NONE;
            }
        }
        else if(ADK_DEMO.OTAP_MNGR[0].OTAP_P0_PREV_VER.iVersionMinor == 2)
        {
            if((ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMajor == 2) &&
                (ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMinor == 2))
            {
                OTAP_DEVICE_VER = VER_2_2_0;
            }
            else if((ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMajor == 2) &&
                (ADK_DEMO.OTAP_NODE[0].OTAP_P0_PREV_VER.iVersionMinor == 1))
            {
                OTAP_DEVICE_VER = VER_2_1_0_NODE;
            }
            else
            {
                OTAP_DEVICE_VER = NONE;
            }
        }
    }
    
}

#if 0
#define TOTAL_ITER_TO_RUN               10   // Number of times to run the SPI test
 
uint8_t Spi_TxData_0[256];
static uint8_t Spi_RxData_0[10][256];
uint8_t Spi_TxData_1[256];
static uint8_t Spi_RxData_1[10][256];
static struct  wbms_cmd_resp_query_device_t QueryDeviceResp_0[10];
static struct  wbms_cmd_resp_query_device_t QueryDeviceResp_1[10];
uint8_t Cbk_0_cnt = 0;
uint8_t Cbk_1_cnt = 0;
 
void Spi_TxCompleteCbk_0(uint8_t iSPIDevice, uint8_t iChipSelect)
{
    if(Spi_RxData_0[Cbk_0_cnt][4] == 0x7F)
    {
        memcpy(&QueryDeviceResp_0[Cbk_0_cnt], &Spi_RxData_0[Cbk_0_cnt][6], sizeof(struct wbms_cmd_resp_query_device_t));
    }
    Cbk_0_cnt++;
}
 
void Spi_TxCompleteCbk_1(uint8_t iSPIDevice, uint8_t iChipSelect)
{
    if(Spi_RxData_1[Cbk_1_cnt][4] == 0x7F)
    {
        memcpy(&QueryDeviceResp_1[Cbk_1_cnt], &Spi_RxData_1[Cbk_1_cnt][6], sizeof(struct wbms_cmd_resp_query_device_t));
    }
    Cbk_1_cnt++;
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

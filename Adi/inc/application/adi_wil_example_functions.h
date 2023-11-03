/*******************************************************************************
 * @file adi_platform_app_wil_functions.h
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef _ADI_PLATFORM_APP_WIL_FUNCTIONS_H
#define _ADI_PLATFORM_APP_WIL_FUNCTIONS_H

#include <stdint.h>
#include <stdbool.h>

#include "adi_wil_types.h"
#include "adi_wil_example_debug_functions.h"
#include "adi_wil_hal.h"
#include "adi_wil_version.h"
#include "adi_wil.h"
#include "adi_bms_types.h"
#include "CmicMConfig.h"
#include "CmicM.h"

/*******************************************************************************
 * #defines                                                                    *
 *******************************************************************************/
/**
 * @remark : structure for ADK
 */

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#define ADK_MAX_cell            16
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
#define ADK_MAX_cell            18
#else   /* Not supported */
#endif

#define ADK_MAX_node            12
#define ADK_MAX_mngr            2
#define PDR_depth               100
#define PDR_AVG_depth           200
#define PDR_pktbuf_depth        (pkt_valid_index_threshold+(ADK_MAX_node*BMS_PACKETS_PER_INTERVAL))
#define NO_SET_MODE             true
#define SET_MODE                false
#define ADK_SINGLE              true
#define ADK_DUAL                false
#define NUM_OF_CHANNELS         16              /* For Health report0 - PathStability */
#define NUM_OF_MANAGERS         ADK_MAX_mngr    /* For Health report0 - PathStability */
#define MAX_NODE_TO_NODE_RSSI   24
#define MAX_MNG_TO_NODE_RSSI    2
#define MAX_DEV_FOR_RSSI        MAX_NODE_TO_NODE_RSSI + MAX_MNG_TO_NODE_RSSI
#define pkt_valid_index_threshold               4
#define NETWORK_STATUS_THRESHOLD                60000

/* the waitForEvent() function issues a prompt if it is still waiting after this amount of time. */
#define WAIT_FOR_EVENT_PROMPT_INITIAL_MSEC      (120 * 1000)
/* the waitForEvent() function re-issues the prompt at this rate if is still waiting. */
#define WAIT_FOR_EVENT_PROMPT_REPEAT_MSEC       (60 * 1000)

/* interval in microseconds between calls to adi_wil_ProcessTask() */
#define PROCESS_TASK_INTERVAL_USEC (2500)
#define PROCESS_TASK_CB_INTERVAL_USEC (50000) /* 50 ms */

/* @remark : Custom global variables */
#define CELL_UNIT   150.0f
#define CELL_OFFSET   1.5f
#define NODE_NUM    realAcl.iCount 
#define TEST_BUFFER_MAX     20

#define moving_AVG 0
#define MAX_LF_RETRY    12

/*******************************************************************************
 * Enumerations
 *******************************************************************************/
/**
 * @brief   File types provided by example application
 */
typedef enum
{
    ADI_WIL_EXAMPLE_GENERIC_BMS_OTAP_CONTAINER = 0,
    ADI_WIL_EXAMPLE_GENERIC_PMS_OTAP_CONTAINER,
    ADI_WIL_NWK_CFG_2_6
} adi_wil_example_file_t;

/**
 * @brief   Manager connections
 */
typedef enum
{
    ADI_WIL_EXAMPLE_STANDALONE_MGR_SPI_PORTA = 0,
    ADI_WIL_EXAMPLE_STANDALONE_MGR_SPI_PORTB = 1,
    ADI_WIL_EXAMPLE_DUAL_MGR = 2
} adi_wil_example_mgr_spi_connection_t;

/**
 * @remark : custom structure by ADK
 */

typedef enum
{
    NORMAL_BMS_SENSING = 0,
    CELL_BALANCING_EVEN = 1,
    CELL_BALANCING_ODD = 2,
    OPEN_WIRE_DETECTION = 3,
    KEY_ON_EVENT = 4,
    KEY_OFF_EVENT = 5
} ADK_DEMO_TYPE;

typedef enum
{
    NO_fail_api_so_far               = 0,
    DBG_wil_Initialize               = 1,
    DBG_wil_Terminate                = 2,
    DBG_wil_QueryDevice              = 3,
    DBG_wil_Connect                  = 4,
    DBG_wil_Disconnect               = 5,
    DBG_wil_SetContextualData        = 6,
    DBG_wil_SetContextualData_wait   = 7,
    DBG_wil_GetContextualData        = 8,
    DBG_wil_GetContextualData_wait   = 9,
    DBG_wil_SetGPIO                  = 10,
    DBG_wil_SetGPIO_wait             = 11,
    DBG_wil_GetGPIO                  = 10,
    DBG_wil_GetGPIO_wait             = 11,
    DBG_wil_SetStateOfHealth         = 12,
    DBG_wil_SetStateOfHealth_wait    = 13,
    DBG_wil_GetStateOfHealth         = 14,
    DBG_wil_GetStateOfHealth_wait    = 15,
    DBG_wil_SelectScript             = 16,
    DBG_wil_SelectScript_wait        = 17,
    DBG_wil_LoadFile                 = 18,
    DBG_wil_LoadFile_wait            = 19,
    DBG_wil_ProcessTask_0            = 20,
    DBG_wil_ProcessTask_1            = 21,
    DBG_wil_GetDeviceVersion         = 22,
    DBG_wil_GetFileCRC               = 23,
    DBG_wil_GetFileCRC_wait          = 24,
    DBG_wil_GetFile                  = 25,
    DBG_wil_GetFile_wait             = 26,
    DBG_wil_EraseFile                = 27,
    DBG_wil_EraseFile_wait           = 28,
    DBG_need_investigation           = 98,
    DBG_fatal_error                  = 99
} ADK_FAIL_API_NAME;

typedef enum
{
    ______________________________________ = 0,
    Demo_ExecuteInitialize________________ = 100,
    Demo_PeriodicallyCallProcessTask______ = 101,
    Demo_ExecuteQueryDevice_______________ = 102,
    Demo_isDualConfig_____________________ = 110,
    Demo_ExecuteConnect_0_________________ = 120,
    Demo_ExecuteResetDevice_single_mngr_0_ = 121,
    Demo_ExecuteLoadFile_singlecon_allmngr = 122,
    Demo_ExecuteResetDevice_single_mngr_1_ = 123,
    Demo_ExecuteDisconnect_0______________ = 124,
    Demo_ExecuteConnect_1_________________ = 130,
    Demo_ExecuteSetMode_0_________________ = 140,
    Demo_ExecuteGetACL_0__________________ = 150,
    Demo_ADK_CompareACL___________________ = 155,
    Demo_ACL_UPDATE_FORCE_________________ = 200,
    Demo_ExecuteResetDevice_______________ = 209,
    Demo_ExecuteSetACL____________________ = 210,
    Demo_ExecuteGetACL_1__________________ = 211,
    Demo_ExecuteGetNetworkStatus_0________ = 212,
    Demo_bAllNodesJoined__________________ = 215,
    Demo_realAcl_generation_______________ = 218,
    Demo_ExecuteSetMode_STANDBY___________ = 220,
    /************** OTAP area start ************/
    Demo_OTAP_No_Operation________________ = 500,
    Demo_OTAP_Update_Triggered____________ = 501,    
    /***************** Update ******************/
    Demo_OTAP_SetMode_STANDBY_0___________ = 502,
    Demo_OTAP_SetMode_OTAP________________ = 503,
    Demo_OTAP_Wait_for_mngr_connect_0_____ = 504,
    Demo_OTAP_GetNetworkStatus____________ = 510,
    Demo_OTAP_LoadFile_OPFW_AllNodes______ = 520,
    Demo_OTAP_LoadFile_OPFW_AllMngrs______ = 530,
    Demo_OTAP_ResetDevice_AllNodes________ = 531,
    Demo_OTAP_SetMode_STANDBY_1___________ = 532,
    Demo_OTAP_Wait_for_mngr_connect_1_____ = 533,
    Demo_OTAP_Wait_for_Nodes_to_join______ = 534,
    Demo_OTAP_SetMode_STANDBY_2___________ = 540,
    Demo_OTAP_GetDeviceVersion_Mngr_Next__ = 541,
    Demo_OTAP_GetDeviceVersion_Node_Next__ = 542,
    Demo_OTAP_Complete____________________ = 550,
    /************** OTAP area end **************/
    Demo_GetDeviceVersion_Mngr_Now________ = 360,
    Demo_GetDeviceVersion_Node_Now________ = 361,
    Demo_ExecuteLoadFile_config_allnode___ = 233,
    Demo_ExecuteLoadFile_config_allmngr___ = 234,
    Demo_ExecuteLoadFile_cntain_allnode___ = 235,
    Demo_ExecuteLoadFile_cntain_node_retry = 236,
    Demo_ExecuteResetDevice_allnode_______ = 250,
    Demo_ExecuteResetDevice_allmngr_______ = 251,
    Demo_ExecuteDisconnect_1______________ = 260,
    Demo_ExecuteConnect_2_________________ = 261,
    Demo_ExecuteGetNetworkStatus_1________ = 270,
    Demo_GetFileCRC_Container_____________ = 272,
    Demo_GetNetworkStatus_________________ = 300,
    example_ExecuteSetMode_1______________ = 400,
    Demo_EnableNetworkDataCapture_________ = 420,
    Demo_PeriodicallyCallProcessTaskCB____ = 450,
    Demo_SchedulerInit____________________ = 998,
    Demo_key_on_event_____________________ = 730,
    Demo_TaskStart________________________ = 703,
    Demo_ExcuteConnect____________________ = 710,
    Demo_GetNetworkStatusACTmode__________ = 711,
    Demo_TaskStartCB______________________ = 720,
    Demo_Total_boot_time__________________ = 999
} ADK_LOG_FUNCTION;

typedef enum
{
    OTAP_Arg0Default = 0,
    OTAP_Mngr = 1,
    OTAP_Node = 2
} ADK_OTAP_ARG_0;

typedef enum
{
    OTAP_Arg1Default = 0,
    OTAP_P0 = 1,
    OTAP_P1 = 2,
    OTAP_NOW = 3
} ADK_OTAP_ARG_1;

typedef enum
{
    VER_2_1_0_NODE = 0,
    VER_2_1_0_MNG,
    VER_2_2_0,
    NONE
} ADK_DEVICE_VER;

/*******************************************************************************
 * Structures
 *******************************************************************************/
typedef struct client_data_struct
{
    adi_wil_err_t eApiCallbackRc;                       /* holds the last rc reported to the API callback */
    adi_wil_connection_details_t  ConnectionDetails;    /* stores the connection details returned by successful connect call */
    adi_wil_event_id_t eEventWaitingFor;                /* application is waiting for this event to occur (application blocks while waiting) */
    uint32_t iEventWaitingForCount;                     /* number of times the eEventWaitingFor event has occured */
    adi_wil_dev_version_t DeviceVersion;                /* stores device information from successful GetDeviceVersion call*/
    adi_wil_acl_t Acl;                                  /* stores the ACL list for app to use */
    adi_wil_loadfile_status_t LoadFileStatus;           /* used during the load file process, records the offset index which should be loaded in next loadFile call */
    adi_wil_file_crc_list_t crclist;
}client_data_t;

typedef struct
{
    uint8_t BMS_PKT_MAP[2];
    float   CELL_V[ADK_MAX_cell];
    int16_t CELL_Vi[ADK_MAX_cell];
    float   TEMP_V[4];      /* Temperature by GPIO1, GPIO2, GPIO3, GPIO4 */
    int16_t TEMP_Vi[4];
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
    uint16_t CB_STAT;       /* Represent DCC bit */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
    uint32_t CB_STAT;       /* Represent DCC bit */
#else   /* Not supported */
#endif    
    bool    OWD_STAT;
    uint16_t OWD_SUCCESS[ADK_MAX_cell];
    uint16_t OWD_FAIL[ADK_MAX_cell];
    uint32_t OWD_CS_STAT;
    float   PDR[4];         /* Current, Worst, Best, Accumulated */
	uint32_t DUPLICATE[2];
    int8_t  RSSI[2][4];     /* Current, Worst, Best, Accumulated */
} NODESTR[ADK_MAX_node];

typedef struct
{
    float OTAP____PROGRESS;
    adi_wil_version_t OTAP_P0_PREV_VER;
    adi_wil_version_t OTAP_P1_OPFW_VER;
    adi_wil_version_t CURRENT_OPFW_VER;
} MNGROTAPSTR[ADK_MAX_mngr];

typedef struct
{
    float OTAP____PROGRESS;
    adi_wil_version_t OTAP_P0_PREV_VER;
    adi_wil_version_t OTAP_P1_OPFW_VER;
    adi_wil_version_t CURRENT_OPFW_VER;
} NODEOTAPSTR[ADK_MAX_node];

typedef struct 
{
    uint16_t BOOT;
    float BOOT_TIME;
    ADK_DEMO_TYPE DEMO_MODE;
    NODESTR NODE;
    bool CB_CELL[12][ADK_MAX_cell];
    bool CB_NODE[12];
    uint8_t LF_NG_CNT; 
    bool ACL_UPDATE_NO_NEED;
    bool ACL_UPDATE_FORCE;
    bool OTAP_UPDATE_FORCE;
    bool OTAP_RE_UPDATE_FORCE;
    ADK_LOG_FUNCTION OTAP_STAT;
    MNGROTAPSTR OTAP_MNGR;
    NODEOTAPSTR OTAP_NODE;    
    ADK_FAIL_API_NAME DBG_FAIL_API;
    adi_wil_err_t DBG_FAIL__RC;   
    ADK_DEMO_TYPE PREV_MODE;
	adi_bms_base_pkt_0_t* BMS_Packet_0_Ptr; //SURE_DEBUG
    adi_bms_base_pkt_1_t* BMS_Packet_1_Ptr;	//SURE_DEBUG
    adi_bms_base_pkt_2_t* BMS_Packet_2_Ptr;	//SURE_DEBUG
} DISPLAYSTR;

typedef struct
{
    unsigned int SeqNum[PDR_depth];
    unsigned short int SeqNum_CrntDepth;
} SEQSTR[ADK_MAX_node];

typedef struct
{
    unsigned int RssiNum[PDR_depth];
    unsigned short int Rssi_CrntDepth;
} RSSISTR[2];

typedef struct
{
    uint16_t STEP;
    ADK_LOG_FUNCTION API;
    float DURATION;    
    uint32_t timestamp[3]; // START, END, TOTAL
} BOOTTIMESTR[150];

/* Node Device */
typedef struct  {
    uint8_t iPacketId;
    uint8_t iTotalPktsGenerated[2];
    uint8_t iPacketsNacked[2];
    uint8_t iNetMicFailures[2];
    uint8_t iMacMicFailures[2];
    uint8_t iJoinAttempts[2];
    uint8_t iParents[2];
    uint8_t pathStability[NUM_OF_MANAGERS][NUM_OF_CHANNELS];
    uint8_t iPacketQueueFailure;
    int8_t  iADCtemp;
}adi_wil_health_report0_t;

/* Node Average RSSI */
typedef struct  {
    uint8_t iPacketId;
    uint8_t signalRssi[MAX_DEV_FOR_RSSI];
}adi_wil_health_report1_t;

/* Node Background RSSI */
typedef struct  {
    uint8_t iPacketId;
    uint8_t backgroundRSSi[NUM_OF_CHANNELS-1][5];
}adi_wil_health_report2_t;

/* Manager Application */
typedef struct  {
    uint8_t iPacketId;
    uint8_t iResetStatus;
    uint8_t iResetCounter[2];
    uint8_t iFreeSpaceFS;
    uint8_t iFlashReadErr[2];
    uint8_t iFlashWriteErr[2];
    uint8_t iFlashEraseErr[2];
    uint8_t iReserved[6];
    uint8_t iAppDebugInfo[32];
}adi_wil_health_report10_t;

/* Manager Device */
typedef struct  {
    uint8_t iPacketId;
    uint8_t iPktSuccess[2];
    uint8_t iPktNacked[2];
    uint8_t iNetMicFailures;
    uint8_t iMasterCounter[2];
    uint8_t iMgrAllocFailCounter[4];
    int8_t  iADCtemp;
    uint8_t iMacMicFailures[24][2];
    uint8_t iReserved[9][2];
}adi_wil_health_report12_t;

/* Manager Background RSSI */
typedef struct  {
    uint8_t iPacketId;
    uint8_t backgroundRSSi[NUM_OF_CHANNELS-1][5];
}adi_wil_health_report11_t;

/* Manager Node-Manager RSSI */
typedef struct  {
    uint8_t iPacketId;
    int8_t  signalRssi[5][NUM_OF_CHANNELS-1];
}adi_wil_health_report14_t;

/* Node Application */
typedef struct  {
    uint8_t iPacketId;
    uint8_t iResetStatus;
    uint8_t iResetCounter[2];
    uint8_t iFreeSpaceFS;
    uint8_t iFlashReadErr[2];
    uint8_t iFlashWriteErr[2];
    uint8_t iFlashEraseErr[2];
    uint8_t iReserved[6];
    uint8_t iAppDebugInfo[32];
}adi_wil_health_report80_t;

/*******************************************************************************
 * Externs                                                                     *
 *******************************************************************************/
// extern bool AUX2A_PACKET_RECEIVED;
// extern bool AUX2B_PACKET_RECEIVED;
extern bool CB_EVEN_DCC_RECEIVED;
extern bool CB_ODD_DCC_RECEIVED;
extern bool CB_DEFAULT_DCC_RECEIVED;
extern bool BASE_PACKET_RECEIVED;
extern bool KEY_ON;
extern bool KEY_OFF;
extern bool bFirstBMSdata;

extern client_data_t ClientData;

#ifdef DBG_INIT_SCRIPT_TEST
// Init buffer indice

extern adi_bms_init_pkt_0_t initBuffer0[ADK_MAX_node];
extern adi_bms_init_pkt_1_t initBuffer1[ADK_MAX_node];
extern adi_bms_init_pkt_2_t initBuffer2[ADK_MAX_node];
extern uint8_t iInitBuffer0index;
extern uint8_t iInitBuffer1index;
extern uint8_t iInitBuffer2index;
#endif


/*******************************************************************************
 * Embedded WIL Wrapper Function Declarations                                  *
 *******************************************************************************/
/**
 * @brief    WIL process task wrapper function (non-blocking)
 *
 * @details  This function calls WIL process task, collect the status and returns.
 *
 * @param    pPack[in,out]           pointer to pack instance
 *
 * @return adi_wil_hal_err_t   Error code of the Process Task.
 */
adi_wil_err_t adi_wil_example_ProcessTask(adi_wil_pack_t * const pPack);

/**
 * @brief   WIL Initialize wrapper function
 *
 * @details Wrapper function for WIL initialization.
 *
 * @param   void               No arguments.
 *
 * @return  adi_wil_hal_err_t  Error code of the initialization.
 */
adi_wil_err_t adi_wil_example_ExecuteInitialize(void);

/**
 * @brief   WIL Terminate wrapper function
 *
 * @details Wrapper function will call WIL terminate function. When called, the WIL calls the
 *          adi_wil_Disconnect() function for any WBMS systems to which the WIL
 *          has been connected, and frees memory used within the WIL. After calling
 *          the adi_wil_Terminate() function, the adi_wil_Initialize() function must
 *          be called before any other WIL function calls can be called again. This
 *          function is a blocking API.
 *
 * @param   void                No arguments.
 *
 * @return  adi_wil_err_t       Error code of the Terminate function.
 */
adi_wil_err_t adi_wil_example_ExecuteTerminate(void);

/**
 * @brief   WIL Query Device wrapper function
 *
 * @details In the case that the host is connecting to a pack where the 
 *          configuration is not known, the user can invoke this API to
 *          interrogate the manager on each SPI port for its current 
 *          configuration, prior an attempt to connect to the managers. This 
 *          allows the user to allocate an appropriately sized packet buffer
 *          and connect in an appropriate manner using the adi_wil_Connect API.
 *          By reviewing the configuration data returned from each SPI port, 
 *          the user can determine if the connect function should be invoked in
 *          single or dual manager mode, the correct amount of packet buffer 
 *          memory to allocate to receive data from the WBMS.
 *          This API is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: Address of an {@link adi_wil_configuration_t} object that
 *                   holds the retrieved configuration. The data in the 
 *                   structure is only valid if the operation was successful.
 *
 * @param   pPortConfig   Pointer to data returned by the query reply
 *
 * @return adi_wil_err_t    Operation error code.
 */
adi_wil_err_t adi_wil_example_ExecuteQueryDevice(adi_wil_configuration_t* pPortConfig);

/**
 * @brief    WIL connect wrapper function
 *
 * @details  Wrapper function will call WIL connect function till network managers are in "Connected" state
 *
 * @param    pPack[in,out]           pointer to pack instance
 * 
 * @param    SensorData[in]          pointer to Sensor Data
 *
 * @param    sysSensorPktCnt[in]     Total sensor packets(BMS, PMS and EMS) that will be received by the system per measurement interval
 * 
 * @param    SingleDual[in]          Define Single / Dual connection
 *
 * @return   adi_wil_err_t       Error code of the connect function.
 */
adi_wil_err_t adi_wil_example_ExecuteConnect(adi_wil_pack_t * const pPack,
                                             adi_wil_sensor_data_t *SensorData,
                                             uint8_t sysSensorPktCnt);

/**
 * @brief    WIL disconnect wrapper function
 *
 * @details  Wrapper function will call WIL disconnect function.
 *           The API disconnects from the network managers. 
 *
 * @param    pPack[in,out]        pointer to pack instance
 *
 * @return   adi_wil_err_t        Error code of the Disconnect function.
 */
adi_wil_err_t adi_wil_example_ExecuteDisconnect(adi_wil_pack_t * const pPack);

/**
 * @brief    WIL set mode wrapper function
 *
 * @details  Wrapper function will call WIL set mode function and set the network mode corresponding to parameter passed
 *
 * @param    pPack[in,out]           pointer to pack instance
 * @param    wilSetMode            Network mode Selected scheme to iterate through API ports
 *
 * @return   adi_wil_err_t       Error code of the Set Mode function.
 */
adi_wil_err_t adi_wil_example_ExecuteSetMode(adi_wil_pack_t * const pPack,
                                             adi_wil_mode_t wilSetMode);

/**
 * @brief    WIL network info wrapper function
 *
 * @details  Wrapper function will call WIL networkinfo function and will populate a reply structure on return
 *
 * @param    pPack[in,out]          pointer to pack instance
 * @param    pNetworkStatus[out]    pointer to store network status
 *
 * @return   bool                   determines whether all nodes have joined the network.
 *
 */
bool adi_wil_example_ExecuteGetNetworkStatus(adi_wil_pack_t * const pPack,
                                             adi_wil_network_status_t *pNetworkStatus, bool no_set_mode);

/**
 * @brief    WIL get mode wrapper function
 *
 * @details  Wrapper function will call WIL get mode function and return the current mode of the network
 *
 * @param    pPack[in,out]          pointer to pack instance
 * @param    pWilGetMode[out]       Network mode Selected scheme to iterate through API ports
 *
 * @return   adi_wil_err_t       Error code of the Set Mode function.
 *
 */
adi_wil_err_t adi_wil_example_ExecuteGetMode(adi_wil_pack_t * const pPack,
                                             adi_wil_mode_t *pWilGetMode);

/**
 * @brief    WIL set ACL wrapper function
 *
 * @details  Wrapper function will call WIL set ACL function as set the ACL list as per parameter passed
 *
 * @param    pPack[in,out]                  pointer to pack instance
 * @param    pWriteDeviceList[in,out]       ACL list
 *
 * @return   adi_wil_err_t        Error code of the Set ACL function.
 */
adi_wil_err_t adi_wil_example_ExecuteSetACL(uint8_t const * const pData, uint8_t iCount);                   /*  @remark: Inherit from TC275 */

/**
 * @brief    WIL get ACL wrapper function
 *
 * @details  Wrapper function will call WIL get ACL function and will populate the network manager ACL list
 *           with MAC addresses passed through a parameter
 *
 * @param    pPack[in,out]              pointer to pack instance
 *
 * @return   adi_wil_err_t        Error code of the Get ACL function.
 */
adi_wil_err_t adi_wil_example_ExecuteGetACL(adi_wil_pack_t * const pPack);

/**
 * @brief    WIL download file to manager/node wrapper function
 *
 * @details  Wrapper function will call WIL download file to manager/node function, here file can be firmware, configuration or container.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_file_type_t[in]                file type to load
 *
 * @param    adi_wil_device_t[in]                   device onto which file pointer to pack instance
 * 
 * @param    bool[in]                               NO_SET_MODE(true), SET_MODE(false)
 *
 * @return   adi_wil_err_t                          Error code of the Download File To Node function.
 */
adi_wil_err_t adi_wil_example_ExecuteLoadFile(adi_wil_pack_t * const pPack, adi_wil_file_type_t eFileType, adi_wil_device_t eDevice, bool no_set_mode);
void adi_wil_example_LoadFileRetry(adi_wil_file_type_t eFileType, adi_wil_device_t eDevice);

/**
 * @brief    WIL set contextual data on to manager/node wrapper function
 *
 * @details  Wrapper function will call WIL set contextual data API and set contextual values on to a manager/node.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which set contextual data is targetted
 *
 * @param    adi_wil_contextual_id_t[in]            contextual ID, specifying the nature and location of the data.
 *
 * @param    adi_wil_contextual_data_t[in]          contextual data to be loaded.
 *
 * @return   adi_wil_err_t                          Error code of the set contextual to manager/node function.
 */
adi_wil_err_t adi_wil_example_ExecuteSetContextualData(adi_wil_pack_t * const pPack, 
                                                       adi_wil_device_t eDeviceId,
                                                       adi_wil_contextual_id_t eContextualId,
                                                       adi_wil_contextual_data_t * pContextualData);

/**
 * @brief    WIL get contextual data from a specified manager/node wrapper function
 *
 * @details  Wrapper function will call WIL get contextual data API and get contextual values from a manager/node.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @param    adi_wil_contextual_id_t[in]            contextual ID, specifying the nature and location of the data.
 *
 * @return   adi_wil_err_t                          Error code of the get contextual from a manager/node function.
 */
adi_wil_err_t adi_wil_example_ExecuteGetContextualData(adi_wil_pack_t * const pPack, 
                                                       adi_wil_device_t eDeviceId,
                                                       adi_wil_contextual_id_t eContextualId);

/**
 * @brief    WIL get GPIO value from a specified manager/node wrapper function
 *
 * @details  Wrapper function will call WIL get GPIO API and retreive GPIO value of a specific GPIO pin from a manager/node.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @param    adi_wil_gpio_id_t[in]                  gpio ID, specifying the gpio pin.
 *
 * @return   adi_wil_err_t                          error code of the get gpio from a manager/node function.
 */
adi_wil_err_t adi_wil_example_ExecuteGetGpio(adi_wil_pack_t * const pPack, 
                                             adi_wil_device_t eDeviceId,
                                             adi_wil_gpio_id_t eGPIOId);

/**
 * @brief    WIL Set GPIO value for a specified manager/node wrapper function
 *
 * @details  Wrapper function will call WIL set GPIO API and sets a specific value to a specified GPIO pin of a manager/nod.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @param    adi_wil_gpio_id_t[in]                  gpio ID, specifying the gpio pin.
 *
 * @param    adi_wil_gpio_value_t[in]               gpio value to be set.
 *
 * @return   adi_wil_err_t                          error code of the set gpio from a manager/node function.
 */
adi_wil_err_t adi_wil_example_ExecuteSetGpio(adi_wil_pack_t * const pPack, 
                                             adi_wil_device_t eDeviceId,
                                             adi_wil_gpio_id_t eGPIOId,
                                             adi_wil_gpio_value_t eGPIOValue);

/**
 * @brief    WIL Set State of Health value for a specified node wrapper function
 *
 * @details  Wrapper function will call WIL set State of Health API and sets a specific value of SoH to a specified node.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @param    uint8_t[in]                            percentage SoH to set.
 *
 * @return   adi_wil_err_t                          error code of the set State of Health API for a node function.
 */
adi_wil_err_t adi_wil_example_ExecuteSetStateOfHealth(adi_wil_pack_t * const pPack,
                                                      adi_wil_device_t eDeviceId,
                                                      uint8_t iPercentage);
/**
 * @brief    WIL Get State of Health value from a specified node wrapper function
 *
 * @details  Wrapper function will call WIL get State of Health API and gets SoH percentage for a specified node.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @return   adi_wil_err_t                          error code of the get State of Health API for a node function.
 */
adi_wil_err_t adi_wil_example_ExecuteGetStateOfHealth(adi_wil_pack_t * const pPack,
                                                      adi_wil_device_t eDeviceId);

/**
 * @brief    WIL select script for specified manager/node wrapper funtion
 *
 * @details  Wrapper function will call WIL select script API and sets a specficifed script ID on a specified device.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device to which get contextual data is targetted
 *
 * @param    adi_wil_sensor_id_t[in]                sensor ID.
 *
 * @param    uint8_t[in]                            script ID to be executed every measurement interval.
 * 
 * @param    bool[in]                               NO_SET_MODE(true), SET_MODE(false)
 *
 * @return   adi_wil_err_t                          error code of the select script API for a specified device.
 */
adi_wil_err_t adi_wil_example_ExecuteSelectScript(adi_wil_pack_t * const pPack,
                                                   adi_wil_device_t eDeviceId,
                                                   adi_wil_sensor_id_t eSensorId,
                                                   uint8_t iScriptId,
                                                   bool no_set_mode);


/**
 * @brief    Example function that processes the BMS buffer
 *
 * @details  Processes the BMS buffer
 *
 * @param    void               No arguments.
 *
 * @return   void     no return.
 */
void adi_wil_example_ExecuteProcessBMSBuffer(void);

/**
 * @remark : custome function for ADK
 */

void adi_wil_example_ADK_ExecuteProcessBMSBuffer(void);

/**
 * @brief    Example function that processes the PMS buffer
 *
 * @details  Processes the PMS buffer
 *
 * @param    void               No arguments.
 *
 * @return   bool     returns true if PMS Buffer ahs been processed
 */
bool adi_wil_example_ExecuteProcessPMSBuffer(void);

/**
 * @brief    Example function that processes the EMS buffer
 *
 * @details  Processes the EMS buffer
 *
 * @param    void       No arguments.
 *
 * @return   bool       returns true after EMS Buffer has been processed
 */
bool adi_wil_example_ExecuteProcessEMSBuffer(void);

/**
 * @brief    WIL connect Pack wrapper function
 *
 * @details  Wrapper function will call WIL connect
 *
 * @param    pPack[in,out]              pointer to pack instance
 * @param    mgrSpiConnection[in]       requested manager configuration
 *
 * @return   adi_wil_err_t        Error code of the Deintialize function.
 */
adi_wil_err_t adi_wil_example_connectPack(adi_wil_pack_t * const pPack,
                                          adi_wil_example_mgr_spi_connection_t mgrSpiConnection);

/************************************************************************************
 * adi_wil_example_WaitForProcessTask
 *
 * Implement a routine to command the WIL ProcessTask.
 *
 * In a multi-thread application, it is recommended to run the Process Task
 * on its own dedicated thread.
 * For an efficient bare metal implementation, the recommendation in terms of frequency
 * of calling the processTask function, it is enough to call it at least once
 * per SPI triggering period.
 *
 * @return adi_wil_err_t
 ***********************************************************************************/
adi_wil_err_t adi_wil_example_WaitForProcessTask(adi_wil_pack_t * const pPack);

/************************************************************************************
 * adi_wil_example_print_welcome
 *
 * Print Welcome message.
 *
 * @return None
 ***********************************************************************************/
void adi_wil_example_print_welcome(void);

/************************************************************************************
 * adi_wil_example_processMac
 *
 * This helper function processes the MAC address within the user input.
 *
 * @return bool
 ***********************************************************************************/
bool adi_wil_example_processMac(const volatile uint8_t *buf,
                                size_t iBufRemaining,
                                size_t *used,
                                uint8_t *dst);

/************************************************************************************
 * adi_wil_example_ExecuteResetDevice
 *
 * This device wrapper function to software reset a given device.
 *
 * @return adi_wil_err_t
 ***********************************************************************************/
adi_wil_err_t adi_wil_example_ExecuteResetDevice(adi_wil_pack_t * const pPack, adi_wil_device_t  eDeviceId);

/************************************************************************************
 * adi_wil_example_PrintACL
 *
 * @brief    Print the ACLs found in a MAC list
 *
 * @param    pPack   pointer to pack instance
 *
 * @return None
 ***********************************************************************************/
void adi_wil_example_PrintACL(adi_wil_pack_t * const pPack);

/**
 * @brief    Get Device version manager/node wrapper function
 *
 * @details  Wrapper function will call WIL get device version and print the information retrieved.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device onto which file pointer to pack instance
 * 
 * @param    bool[in]                               NO_SET_MODE(true), SET_MODE(false)
 *
 * @return adi_wil_err_t 
 */
adi_wil_err_t adi_wil_example_GetDeviceVersion(adi_wil_pack_t * const pPack, adi_wil_device_t eDevice, bool no_set_mode);

/**
 * @brief    Get container details from manager/node wrapper function
 *
 * @details  Wrapper function will call WIL get container details and print the information retrieved.
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device onto which file pointer to pack instance
 *
 * @param    adi_wil_file_type_t[in]                file type being loaded.
 *
 * @param    bool[in]                               NO_SET_MODE(true), SET_MODE(false)
 *
 * @return adi_wil_err_t                            error code of the function GetContainerDetails
 */
adi_wil_err_t adi_wil_example_GetFileCRC(adi_wil_pack_t * const pPack,
                                         adi_wil_device_t eDevice,
                                         adi_wil_file_type_t eFileType, 
                                         bool no_set_mode);

#ifdef DBG_GET_FILE_TEST
/**
 * @brief   WIL retrieves specified file from a manager or a node device.
 *
 * @details Wrapper function will call WIL Get File API from a specified device.
 *
 * @param   pPack[in,out]                          pointer to pack instance
 *
 * @param   adi_wil_file_type_t[in]                file type to retrieve the file from.
 *
 * @param   adi_wil_device_t[in]                   target device to get file from
 *
 * @return  adi_wil_err_t                          error code of the get file API from a specified device.
 */
adi_wil_err_t adi_wil_example_ExecuteGetFile(adi_wil_pack_t * const pPack, 
                                             adi_wil_file_type_t eFileType,
                                             adi_wil_device_t eDevice);
#endif

/**
 * @brief    Erases a specified file that resides on one or more devices wrapper function
 *
 * @details  Wrapper function that erases a file on the target device(s).
 *
 * @param    pPack[in,out]                          pointer to pack instance
 *
 * @param    adi_wil_device_t[in]                   device onto which file pointer to pack instance
 *
 * @param    adi_wil_file_type_t[in]                file type being loaded.
 *
 * @return adi_wil_err_t                            error code of the function GetContainerDetails
 */
adi_wil_err_t adi_wil_example_EraseFile(adi_wil_pack_t * const pPack,
                                        adi_wil_device_t eDevice,
                                        adi_wil_file_type_t eFileType);

/**
 * @brief    Checks if configurations are of dual manager configuration.
 *
 * @details  Wrapper function that checks if both input configurations are correctly 
 *           configured in dual manager configuration.
 *
 * @param    adi_wil_configuration_t[in]    QueryDevice response structure.
 * @param    adi_wil_port_t[in]    Port State structure
 *
 * @return   bool                     returns true if configuration is in dual manager mode
 */
bool adi_wil_example_isDualConfig(adi_wil_configuration_t * C1, adi_wil_configuration_t * C2, adi_wil_port_t* pPort);

extern void adi_wil_HandleCallback (adi_wil_pack_t const * const pPack,
                                    void const * const pClientData,
                                    adi_wil_api_t eAPI,
                                    adi_wil_err_t rc,
                                    void const * const pData);

extern void adi_wil_HandleEvent (adi_wil_pack_t const * const pPack,
                                 void const * const pClientData,
                                 adi_wil_event_id_t EventCode,
                                 void const * const pData);


void adi_wil_mac_deviceID_return(adi_wil_pack_t *pPack, bool bMacReturn, uint8_t *pMacPtr, uint8_t *pDeviceID);

uint8_t readSensorPacketCount(adi_wil_configuration_t* pPortConfig);
void adi_task_EventStatistics(void);

/******************************************************************************
 * @remark : Custom functions for development
 *****************************************************************************/
bool adi_wil_example_ADK_CompareACL(void);
void adi_wil_example_waitForEvent(adi_wil_event_id_t eEventCode, uint32_t iCount);
adi_wil_err_t adi_wil_example_otap_QueryDeviceCommon(adi_wil_port_t * const pPort, adi_wil_configuration_t * const pConfig, char const pPortName[]);
adi_wil_err_t adi_wil_example_otap_QueryDevicePort1(adi_wil_configuration_t * const pConfig);
adi_wil_err_t adi_wil_example_otap_QueryDevicePort0(adi_wil_configuration_t * const pConfig);
uint8_t adi_wil_example_countUnconnectedNodes(void);
void adi_wil_example_waitForNodesToJoin(void);

uint8_t ADK_ConvertDeviceId (adi_wil_device_t WilDeviceId);

adi_wil_hal_err_t adk_debug_TickerBTInit(void);
adi_wil_hal_err_t adk_debug_TickerBTStart(void);
uint32_t adk_debug_TickerBTGetTimestamp(void);
adi_wil_hal_err_t adk_debug_TickerBTStop(void);
void adi_wil_example_GetOTAPVersion(bool bAllNodesJoined);

adi_wil_err_t Cmic_ExecuteInitialize(void);
adi_wil_err_t Cmic_RequestQueryDevice(uint8 portCnt);
adi_wil_err_t Cmic_RequestConnect(adi_wil_pack_t * const pPack,
                                          adi_wil_sensor_data_t *SensorData, uint8_t sysSensorPktCnt);
adi_wil_err_t Cmic_RequestResetDevice(adi_wil_pack_t * const pPack, adi_wil_device_t eDeviceId);

bool Cmic_RequestGetNetworkStatus(adi_wil_pack_t * const pPack,
                                             adi_wil_network_status_t *pNetworkStatus);

adi_wil_err_t Cmic_RequestSelectScript(adi_wil_pack_t * const pPack,
                                                   adi_wil_device_t eDeviceId,
                                                   adi_wil_sensor_id_t eSensorId,
                                                   uint8_t iScriptId);   
adi_wil_err_t Cmic_RequestSetACL(uint8_t const * const pData, uint8_t iCount);

adi_wil_err_t Cmic_RequestGetACL(adi_wil_pack_t * const pPack);
adi_wil_err_t Cmic_RequestSetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode);
adi_wil_err_t Cmic_ExecuteGetMode(adi_wil_pack_t * const pPack,
											adi_wil_mode_t *pWilGetMode);

adi_wil_err_t Cmic_ExecuteDisconnect(adi_wil_pack_t * const pPack);

bool 		Cmic_CallProcessTask(void);


#endif /* _ADI_PLATFORM_APP_WIL_FUNCTIONS_H */

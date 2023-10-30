/**************************************************************************************************
 * @warning  This project is example code, user must ensure functionality before migration
 * 
 * @brief    wBMS basic example code for IFX TC387 + ADRF8850 + ADRF8800 + (ADBMS6833 or ADBMS6830)
 * @version  ADK v1.9.21
 *
 * @remark   [Preparation]
 *            1. adi_wil_example_debug_function.h : 
 *               1) ADK_SPI_0 = define QSPI_x channel for SPI0_MASTER
 *               2) ADK_SPI_1 = define QSPI_x channel for SPI1_MASTER
 *               3) ADK_SPI_SPEED = Reserved, Now SPI speed has been fixed to 1Mhz
 *               4) ADK_ADBMS683x = define BMIC
 *            2. adi_wil_example_config.h : 
 *               1) ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL = BMS packet number per interval
 *                  This parameter depends on ADRF88xx configuration and container
 *                  In this example project, it is 3
 *            3. Check HAL driver configuration
 *               1) User must check all HAL driver configurations before build except osal
 *               2) Basically all HAL drivers are working when this example runs only
 *            4. Check configuration_files
 *               1) User must check ADRF88xx configuration based on their wBMS system
 *               2) Example configuration is under Tools.zip
 *            5. Check container_files
 *               1) User must check ADRF88xx container based on their wBMS system
 *               2) Example container is under Tools.zip
 *            6. OP FW must be downloaded to all managers and nodes
 *               1) This step should be done by external JTAG for all ADRF88xx
 *               2) OP FW 1.1.5 is used for this project
 *            7. adi_wil_example_acl.c :
 *               1) userAcl.iCount = number of nodes
 *               2) userAcl.Data = array of MAC address for all nodes
 * 
 * @details  [Display]    : "ADK_DEMO" structure
 * 
 * @details  [User input] : ADK_DEMO.DEMO_MODE
 *                        = 0 (NORMAL_BMS_SENSING)
 *                        = 1 (CELL_BALANCING_EVEN)
 *                        = 2 (CELL_BALANCING_ODD)
 *                        = 3 (OPEN_WIRE_DETECTION) --> Reserved, DO NOT USE
 *                        = 4 (KEY_ON_EVENT)
 *                        = 5 (KEY_OFF_EVENT)
 * 
 * @details  [Available wBMS function list (Available DEMO_MODE)] 
 *            1. Cell voltage read (0, 1, 2, 3, 4)
 *            2. PDR calculation (0, 1, 2, 3, 4)
 *            3. RSSI calculation (0, 1, 2, 3, 4)
 *            4. Aux channel voltage read for temp. sensor (0, 1, 2, 3, 4)
 *            5. Cell balancing (1, 2)
 *            6. Open wire detection (0, 1, 2, 3, 4)
 * 
 * @date     2022.09.27
 * @author   Analog Devices Korea Software FAE Team
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
**************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "adi_wil.h"
#include "adi_wil_example_app_version.h"
#include "adi_wil_example_debug_functions.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_functions.h"
#include "adi_wil_example_printf.h"
#include "adi_wil_example_acl.h"
#include "adi_wil_example_version.h"
#include "adi_wil_example_scheduler.h"
#include "adi_wil_example_cell_balance.h"
#include "adi_wil_example_owd.h"
#include "adi_bms_defs.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_example_cfg_profiles.h"



/******************************************************************************************/
/* Global Function Declarations                                                           */
/******************************************************************************************/
extern bool adi_wil_example_PeriodicallyCallProcessTask(void);
extern void adk_debug_Report(ADK_FAIL_API_NAME api, adi_wil_err_t rc );
extern void adk_debug_BootTimeLog(bool final, bool start, uint16_t step, ADK_LOG_FUNCTION api);
bool adi_wil_example_PeriodicallyCallProcessTaskCB(void);
extern uint32_t adi_wil_hal_TickerGetTimestamp(void);
extern void WaitForWilAPI(adi_wil_pack_t * const pPack);
extern adi_wil_err_t adi_wil_example_RetrySetMode(adi_wil_pack_t * const pPack, adi_wil_mode_t mode);

extern void adk_debug_log_GetDeviceVersion(ADK_OTAP_ARG_0 eDeviceID, ADK_OTAP_ARG_1 update, uint8_t idx);
extern void adi_wil_hal_TaskStop(void);

void adi_wil_example_OTA_mode(void);
void adi_wil_example_OTA_Node(void);
void adi_wil_example_OTA_MNG(void);

/******************************************************************************************/
/* Global Structure Declarations                                                          */
/******************************************************************************************/
extern DISPLAYSTR ADK_DEMO;
BOOTTIMESTR BOOT_TIME;
extern adi_wil_err_t gNotifRc;

/******************************************************************************************/
/* Global Variable Declarations                                                           */
/******************************************************************************************/
extern volatile uint8_t iMgrConnectCount;
bool adi_gNotifyBms;
bool adi_gNotifyBmsData;
bool adi_gNotifyPms;
bool adi_gNotifyEms;
bool adi_gNotifyNetworkMeta;
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
bool adi_gNotifyLatent0 = false;
bool adi_gNotifyLatent1 = false;
extern uint8_t latent0_recv_confirm[ADK_MAX_node];
extern uint8_t latent1_recv_confirm[ADK_MAX_node];
#else   /* Not supported */
#endif
volatile adi_wil_err_t adi_gProcessTaskErrorCode = ADI_WIL_ERR_SUCCESS;
adi_wil_network_status_t networkStatus;
adi_wil_network_data_t   networkDataBuffer[ADI_WIL_MAX_NODES*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
adi_wil_pack_t packInstance; /* pack instance object */
adi_wil_acl_t realAcl;
#ifdef DBG_ACL_MAP_TEST
bool userAcl_map[ADK_MAX_node] = {false,};
bool realAcl_map[ADK_MAX_node] = {false,};
#endif

uint8_t gTotalNodes       = 0u;        /* @remark Variable to store total no. of Nodes in network */
bool gQueryDeviceRetry    = false;     /* @remark request by sure-soft */
uint8_t Test_RxData_0[256];
uint8_t Test_RxData_1[256];
uint16_t NWBufferSize = 0;
ADK_DEVICE_VER OTAP_DEVICE_VER;

extern bool ACL_EMPTY;
extern uint8_t Spi_TxData_0[256];
extern uint8_t Spi_TxData_1[256];
extern bool WAKEUP;
uint16_t G_BOOT_TIME_CNT = 1;

uint8_t G_SCR_STATE = 0;
uint8_t G_CB_INITIAL = 0;
uint8_t CB_EVEN = 0;
uint8_t CB_ODD = 1;
uint8_t CB_DEFAULT = 2;
uint8_t selectscript_node_count = 0;

#ifdef DBG_GET_FILE_TEST
extern uint8_t   userGetFileContainer[GET_FILE_BUFFER_COUNT];
#endif

/******************************************************************************************/
/* Local Variable Declarations                                                            */
/******************************************************************************************/
static adi_wil_sensor_data_t  wbmsSysSensorData[BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT];
static adi_wil_port_t portVar[PORT_COUNT];
static adi_wil_configuration_t portConfig[PORT_COUNT];

/******************************************************************************************/
/* Start of adi_wil_example_Main                                                          */
/******************************************************************************************/
int adi_wil_example_Main(void)
{
    uint8_t i, j;
    bool bAllNodesJoined = false;
    bool bLoadMNGConfig = false;

    /* remark : Ticker early init for boot time measurement */
    adk_debug_TickerBTInit();
    ADK_DEMO.BOOT = 90;
    adk_debug_BootTimeLog(Overall_, LogStart, 999, Demo_Total_boot_time__________________);
    
    #ifdef DBG_ACL_MAP_TEST
    /* @remark : userAcl_map initialize */
    for(i=0; i<userAcl.iCount; i++)
    {
        userAcl_map[i] = true;
    }
    #endif

    /* @remark : realAcl initialize */
    memset(&realAcl, 0, sizeof(adi_wil_acl_t));
    /* @remark : Container update indicator (false = update needed, true = update no need) */
    ADK_DEMO.ACL_UPDATE_NO_NEED = false;
    ADK_DEMO.OTAP_STAT = Demo_OTAP_No_Operation________________;
    
    
    /* STEP 1  : WBMS System Initialization **********************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 100, Demo_ExecuteInitialize________________);
    adi_wil_example_ExecuteInitialize();
    ADK_DEMO.BOOT = 100;
    adk_debug_BootTimeLog(Interval, LogEnd__, 100, Demo_ExecuteInitialize________________);

    /* STEP 2  : Start calling processTask periodically **********************************/
    adk_debug_BootTimeLog(Interval, LogStart, 101, Demo_PeriodicallyCallProcessTask______);
    adi_wil_example_PeriodicallyCallProcessTask();
    ADK_DEMO.BOOT = 101;
    adk_debug_BootTimeLog(Interval, LogEnd__, 101, Demo_PeriodicallyCallProcessTask______);

    /* STEP 3  : Query device ************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 102, Demo_ExecuteQueryDevice_______________);
    /* Determine whether the two managers on the two SPI ports are in dual manager mode */
    memset(portConfig, 0, sizeof(portConfig));
    while(gQueryDeviceRetry != true){       /*  @remark : Query device retry */
        adi_wil_example_ExecuteQueryDevice(portConfig);
    }        
    ADK_DEMO.BOOT = 102;
    adk_debug_BootTimeLog(Interval, LogEnd__, 102, Demo_ExecuteQueryDevice_______________);

    /* CONDITION 1 : Check Dual Manager configuration ************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 110, Demo_isDualConfig_____________________);
    bool bDualMgrConfiguration = adi_wil_example_isDualConfig(&portConfig[0], &portConfig[1], portVar);
    ADK_DEMO.BOOT = 110;
    adk_debug_BootTimeLog(Interval, LogEnd__, 110, Demo_isDualConfig_____________________);


    if (!bDualMgrConfiguration)
    {
        /* Dual Manager mode config download for empty device */
        /* STEP 4  : Connect *************************************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 120, Demo_ExecuteConnect_0_________________);
        adi_wil_err_t rc;
        rc = (adi_wil_example_ExecuteConnect(&packInstance,
                                             wbmsSysSensorData,
                                             (BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT)));
        ADK_DEMO.BOOT = 120;
        adk_debug_BootTimeLog(Interval, LogEnd__, 120, Demo_ExecuteConnect_0_________________);

        if ((rc == ADI_WIL_ERR_SUCCESS) || (rc == ADI_WIL_ERR_CONFIGURATION_MISMATCH) || (rc == ADI_WIL_ERR_INVALID_STATE))
        {
            /* STEP 5  : Reset Device (ALL MANAGERS) *************************************/
            adk_debug_BootTimeLog(Interval, LogStart, 121, Demo_ExecuteResetDevice_single_mngr_0_);
            returnOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS));
            ADK_DEMO.BOOT = 121;
            iMgrConnectCount = 0;
            while(iMgrConnectCount < ADI_WIL_NUM_NW_MANAGERS) {}; /* i.e. wait for two ADI_WIL_EVENT_COMM_MGR_CONNECTED events */
            adk_debug_BootTimeLog(Interval, LogEnd__, 121, Demo_ExecuteResetDevice_single_mngr_0_);

            /* STEP 6  : Load files (CONFIGURATION, ALL MANAGERS) ************************/
            adk_debug_BootTimeLog(Interval, LogStart, 122, Demo_ExecuteLoadFile_singlecon_allmngr);
            returnOnWilError(adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS, SET_MODE));
            bLoadMNGConfig = true;
            ADK_DEMO.BOOT = 122;
            adk_debug_BootTimeLog(Interval, LogEnd__, 122, Demo_ExecuteLoadFile_singlecon_allmngr);

            /* STEP 7  : Reset Device (ALL MANAGERS) *************************************/
            adk_debug_BootTimeLog(Interval, LogStart, 123, Demo_ExecuteResetDevice_single_mngr_1_);
            returnOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS));
            ADK_DEMO.BOOT = 123;
            iMgrConnectCount = 0;
            while(iMgrConnectCount < ADI_WIL_NUM_NW_MANAGERS) {}; /* i.e. wait for two ADI_WIL_EVENT_COMM_MGR_CONNECTED events */
            adk_debug_BootTimeLog(Interval, LogEnd__, 123, Demo_ExecuteResetDevice_single_mngr_1_);

            /* STEP 8  : Disconnect ******************************************************/
            adk_debug_BootTimeLog(Interval, LogStart, 123, Demo_ExecuteDisconnect_0______________);
            returnOnWilError(adi_wil_example_ExecuteDisconnect(&packInstance));
            ADK_DEMO.BOOT = 124;
            adk_debug_BootTimeLog(Interval, LogEnd__, 123, Demo_ExecuteDisconnect_0______________);
        }
        else
        {
            /* Fail */
            fatalError(rc);
        }
        
        /* STEP  9 : Check Dual Manager configuration ************************************/
        bDualMgrConfiguration = adi_wil_example_isDualConfig(&portConfig[0], &portConfig[1], portVar);
        
    }

    /* STEP 10 : Connect *****************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 130, Demo_ExecuteConnect_1_________________);
    returnOnWilError(adi_wil_example_ExecuteConnect(&packInstance,
                                        wbmsSysSensorData,
                                        (BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT)));
    ADK_DEMO.BOOT = 130;
    adk_debug_BootTimeLog(Interval, LogEnd__, 130, Demo_ExecuteConnect_1_________________);

    /* CONDITION 2 : Check STANDBY mode **************************************************/
    /* STEP 11 : Set Mode (STANDBY) ******************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 140, Demo_ExecuteSetMode_0_________________);
    returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY));
    ADK_DEMO.BOOT = 140;
    adk_debug_BootTimeLog(Interval, LogEnd__, 140, Demo_ExecuteSetMode_0_________________);

    /* STEP 12 : Get Acl *****************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 150, Demo_ExecuteGetACL_0__________________);
    fatalOnWilError(adi_wil_example_ExecuteGetACL(&packInstance));
    ADK_DEMO.BOOT = 150;
    adk_debug_BootTimeLog(Interval, LogEnd__, 150, Demo_ExecuteGetACL_0__________________);

    /* STEP 13 : Compare Acl *************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 155, Demo_ADK_CompareACL___________________);
    ADK_DEMO.ACL_UPDATE_NO_NEED = adi_wil_example_ADK_CompareACL();
    ADK_DEMO.BOOT = 155;
    adk_debug_BootTimeLog(Interval, LogEnd__, 155, Demo_ADK_CompareACL___________________);

#ifndef _SURE_TEST_
    ADK_DEMO.ACL_UPDATE_FORCE = true;
#endif
    /* CONDITION 3 : Check ACL update ****************************************************/
    if(ADK_DEMO.ACL_UPDATE_NO_NEED == false || ADK_DEMO.ACL_UPDATE_FORCE == true || ADK_DEMO.OTAP_RE_UPDATE_FORCE == true)
    {
        if(!ADK_DEMO.OTAP_RE_UPDATE_FORCE)
        {
            adk_debug_BootTimeLog(Interval, LogStart, 200, Demo_ACL_UPDATE_FORCE_________________);
            ADK_DEMO.BOOT = 200;
            adk_debug_BootTimeLog(Interval, LogEnd__, 200, Demo_ACL_UPDATE_FORCE_________________);
            
            if (ACL_EMPTY == false){
                adk_debug_BootTimeLog(Interval, LogStart, 209, Demo_ExecuteResetDevice_______________);
                fatalOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES));
                ADK_DEMO.BOOT = 209;
                adk_debug_BootTimeLog(Interval, LogEnd__, 209, Demo_ExecuteResetDevice_______________);
            }
            
            adk_debug_BootTimeLog(Interval, LogStart, 210, Demo_ExecuteSetACL____________________);
            do {
                /* STEP 14 : SetAcl **********************************************************/
                if(realAcl.iCount == 0)
                {
                    fatalOnWilError(adi_wil_example_ExecuteSetACL(userAcl.Data, userAcl.iCount));
                    ADK_DEMO.BOOT = 210;
                }
                else
                {
                    /* @remark : SetAcl from realAcl,  */
                    returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY));
                    ADK_DEMO.BOOT = 218;
                    /* @remark : should call ResetDevice */
                    fatalOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES));
                    ADK_DEMO.BOOT = 209;
                    fatalOnWilError(adi_wil_example_ExecuteSetACL(realAcl.Data, realAcl.iCount));
                    ADK_DEMO.BOOT = 210;
                }
                adk_debug_BootTimeLog(Interval, LogEnd__, 210, Demo_ExecuteSetACL____________________);

                adk_debug_BootTimeLog(Interval, LogStart, 211, Demo_ExecuteGetACL_1__________________);
                fatalOnWilError(adi_wil_example_ExecuteGetACL(&packInstance));
                ADK_DEMO.BOOT = 211;
                adk_debug_BootTimeLog(Interval, LogEnd__, 211, Demo_ExecuteGetACL_1__________________);

                /* STEP 15 : Set Mode (COMMISSIONING) ****************************************/
                /* STEP 16 : Get Network Status **********************************************/
                adk_debug_BootTimeLog(Interval, LogStart, 212, Demo_ExecuteGetNetworkStatus_0________);
                bAllNodesJoined = adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &networkStatus, SET_MODE);
                if((bAllNodesJoined == false) && (ADK_DEMO.OTAP_UPDATE_FORCE == true))
                {
                    memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
                    networkStatus.iCount = realAcl.iCount;
                    for(uint8_t k=0; k<networkStatus.iCount; k++) {
                        networkStatus.iConnectState += 1ULL<<k;
                    }                    
                    break;
                }
                ADK_DEMO.BOOT = 212;
                adk_debug_BootTimeLog(Interval, LogEnd__, 212, Demo_ExecuteGetNetworkStatus_0________);

                /* CONDITION 4 : Check Node response *****************************************/
                adk_debug_BootTimeLog(Interval, LogStart, 215, Demo_bAllNodesJoined__________________);
                if(!bAllNodesJoined){
                    ADK_DEMO.BOOT = 215;
                    uint8_t l = 0;
                    memset(&realAcl, 0, sizeof(adi_wil_acl_t));
                    #ifdef DBG_ACL_MAP_TEST
                    /* realAcl_map Initialize */
                    for(i=0; i<userAcl.iCount; i++)
                    {
                        realAcl_map[i] = false;
                    }
                    #endif
                    for(uint8_t k=0; k<networkStatus.iCount; k++) {
                        if(networkStatus.iConnectState & (1<<k))
                        {
                            /* STEP 17 : realAcl generation from userAcl *********************/
                            memcpy(realAcl.Data + (l * ADI_WIL_MAC_ADDR_SIZE), userAcl.Data + (k * ADI_WIL_MAC_ADDR_SIZE), ADI_WIL_MAC_ADDR_SIZE);
                            #ifdef DBG_ACL_MAP_TEST
                            realAcl_map[k] = true;
                            #endif
                            ADK_DEMO.BOOT = 216;
                            realAcl.iCount++;
                            l++;
                        }
                    }
                }
            }while(!bAllNodesJoined);
            adk_debug_BootTimeLog(Interval, LogEnd__, 215, Demo_bAllNodesJoined__________________);

            /* CONDITION 5 : Check First trial for realAcl ***********************************/
            adk_debug_BootTimeLog(Interval, LogStart, 218, Demo_realAcl_generation_______________);
            if(realAcl.iCount == 0){
                /* STEP 18 : realAcl generation from userAcl *********************************/
                memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
                #ifdef DBG_ACL_MAP_TEST
                for(i=0; i<userAcl.iCount; i++)
                {
                    realAcl_map[i] = true;
                }
                #endif
                ADK_DEMO.BOOT = 218;
                adk_debug_BootTimeLog(Interval, LogEnd__, 218, Demo_realAcl_generation_______________);
            }

            /* STEP 19 : Set Mode (STANDBY) **************************************************/
            adk_debug_BootTimeLog(Interval, LogStart, 220, Demo_ExecuteSetMode_STANDBY___________);
            returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY));
            ADK_DEMO.BOOT = 220;
            adk_debug_BootTimeLog(Interval, LogEnd__, 220, Demo_ExecuteSetMode_STANDBY___________);
        }
        else
        {
            memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
            networkStatus.iCount = realAcl.iCount;
            for(uint8_t k=0; k<networkStatus.iCount; k++) {
                networkStatus.iConnectState += 1ULL<<k;
            }
        }

        /* CONDITION 6 : OTAP Force update ***************************************************/
        if(ADK_DEMO.OTAP_UPDATE_FORCE || ADK_DEMO.OTAP_RE_UPDATE_FORCE)
        {
            adk_debug_BootTimeLog(Interval, LogStart, 501, Demo_OTAP_Update_Triggered____________);
            ADK_DEMO.OTAP_STAT = Demo_OTAP_Update_Triggered____________;
            ADK_DEMO.BOOT = 501;
            adk_debug_BootTimeLog(Interval, LogEnd__, 501, Demo_OTAP_Update_Triggered____________);

            adi_wil_example_GetOTAPVersion(bAllNodesJoined);

            switch(OTAP_DEVICE_VER)
            {
                /*********************************************************************************/
                /*************************** OTAP download OPFW 2.2.0 ****************************/
                /*********************************************************************************/
                case VER_2_1_0_NODE:
                    adi_wil_example_OTA_mode();
                    adi_wil_example_OTA_Node();
                    adi_wil_example_OTA_MNG();
                    break;
                case VER_2_1_0_MNG:                   
                    adi_wil_example_OTA_mode();
                    adi_wil_example_OTA_MNG();
                    /* End of OTA */
                    break;
                default:
                    break;
            }
        }
        
        /* STEP 21 : Load files (CONFIGURATION, ALL NODES) *******************************/
        adk_debug_BootTimeLog(Interval, LogStart, 233, Demo_ExecuteLoadFile_config_allnode___);
        fatalOnWilError(adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_NODES, NO_SET_MODE));
        adi_wil_example_LoadFileRetry(ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_NODES);
        ADK_DEMO.BOOT = 233;
        adk_debug_BootTimeLog(Interval, LogEnd__, 233, Demo_ExecuteLoadFile_config_allnode___);

        if(!bLoadMNGConfig)
        {
            /* STEP 22 : Load files (CONFIGURATION, ALL MANAGERS) ****************************/
            adk_debug_BootTimeLog(Interval, LogStart, 234, Demo_ExecuteLoadFile_config_allmngr___);
            fatalOnWilError(adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS, NO_SET_MODE));
            adi_wil_example_LoadFileRetry(ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS);
            ADK_DEMO.BOOT = 234;
            adk_debug_BootTimeLog(Interval, LogEnd__, 234, Demo_ExecuteLoadFile_config_allmngr___);
        }
        
        /* STEP 23 : Load files (CONTAINER, ALL NODES) ***********************************/
        adk_debug_BootTimeLog(Interval, LogStart, 235, Demo_ExecuteLoadFile_cntain_allnode___);
        fatalOnWilError(adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_BMS_CONTAINER, ADI_WIL_DEV_ALL_NODES, NO_SET_MODE));
        ADK_DEMO.BOOT = 235;
        adk_debug_BootTimeLog(Interval, LogEnd__, 235, Demo_ExecuteLoadFile_cntain_allnode___);

        adk_debug_BootTimeLog(Interval, LogStart, 236, Demo_ExecuteLoadFile_cntain_node_retry);
        adi_wil_example_LoadFileRetry(ADI_WIL_FILE_TYPE_BMS_CONTAINER, ADI_WIL_DEV_ALL_NODES);
        ADK_DEMO.BOOT = 236;
        adk_debug_BootTimeLog(Interval, LogEnd__, 236, Demo_ExecuteLoadFile_cntain_node_retry);

        /* STEP 24 : Reset Device (ALL NODES) ********************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 250, Demo_ExecuteResetDevice_allnode_______);
        fatalOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES));
        ADK_DEMO.BOOT = 250;
        adk_debug_BootTimeLog(Interval, LogEnd__, 250, Demo_ExecuteResetDevice_allnode_______);

        /* STEP 25 : Reset Device (ALL MANAGERS) *****************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 251, Demo_ExecuteResetDevice_allmngr_______);
        //while(iNodeDisconnectedCount < 2) {};
        //if(!test1)
        {
            fatalOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS));
        }
        ADK_DEMO.BOOT = 251;
        adk_debug_BootTimeLog(Interval, LogEnd__, 251, Demo_ExecuteResetDevice_allmngr_______);

        /* STEP 26 : Disconnect **********************************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 260, Demo_ExecuteDisconnect_1______________);
        returnOnWilError(adi_wil_example_ExecuteDisconnect(&packInstance));
        ADK_DEMO.BOOT = 260;
        adk_debug_BootTimeLog(Interval, LogEnd__, 260, Demo_ExecuteDisconnect_1______________);

        /* STEP 27 : Connect *************************************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 261, Demo_ExecuteConnect_2_________________);
        returnOnWilError(adi_wil_example_ExecuteConnect(&packInstance, wbmsSysSensorData, (BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT)));
        ADK_DEMO.BOOT = 261;
        adk_debug_BootTimeLog(Interval, LogEnd__, 261, Demo_ExecuteConnect_2_________________);

        /* STEP 28 : Set Mode (COMMISSIONING) ********************************************/
        /* STEP 29 : Get Network Status **************************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 270, Demo_ExecuteGetNetworkStatus_1________);
        adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &networkStatus, SET_MODE);
        ADK_DEMO.BOOT = 270;
        adk_debug_BootTimeLog(Interval, LogEnd__, 270, Demo_ExecuteGetNetworkStatus_1________);

        /* STEP 30 : Set Mode (STANDBY) **************************************************/
        /* STEP 31 : Get File CRC (Container) ********************************************/
        adk_debug_BootTimeLog(Interval, LogStart, 272, Demo_GetFileCRC_Container_____________);
        adi_wil_example_GetFileCRC(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_FILE_TYPE_BMS_CONTAINER, SET_MODE);
        ADK_DEMO.BOOT = 272;
        adk_debug_BootTimeLog(Interval, LogEnd__, 272, Demo_GetFileCRC_Container_____________);
        
        #ifdef DBG_GET_FILE_TEST
        /* GetFile for single manager / single node */
        memset(&userGetFileContainer[0], 0, GET_FILE_BUFFER_COUNT);
        adi_wil_example_ExecuteGetFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_MANAGER_0);
        // adi_wil_example_ExecuteGetFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_MANAGER_1);
        // adi_wil_example_ExecuteGetFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_NODE_0);
        // adi_wil_example_ExecuteGetFile(&packInstance, ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_NODE_1);
        #endif
    }
    else{
        /* STEP 32 : realAcl generation from userAcl *************************************/
        if(realAcl.iCount == 0){          
            memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
            #ifdef DBG_ACL_MAP_TEST
            for(i=0; i<userAcl.iCount; i++)
            {
                realAcl_map[i] = true;
            }
            #endif
            ADK_DEMO.BOOT = 290;
        }
        do{
            /* STEP 33 : Set Mode (COMMISSIONING) ********************************************/
            /* STEP 34 : Get Network Status **************************************************/
            adk_debug_BootTimeLog(Interval, LogStart, 300, Demo_GetNetworkStatus_________________);
            bAllNodesJoined = adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &networkStatus, SET_MODE);
            ADK_DEMO.BOOT = 300;
            adk_debug_BootTimeLog(Interval, LogEnd__, 300, Demo_GetNetworkStatus_________________);

            if(!bAllNodesJoined){
                ADK_DEMO.BOOT = 301;
                uint8_t l = 0;
                memset(&realAcl, 0, sizeof(adi_wil_acl_t));
                #ifdef DBG_ACL_MAP_TEST
                /* realAcl_map Initialize */
                for(i=0; i<userAcl.iCount; i++)
                {
                    realAcl_map[i] = false;
                }
                #endif
                for(uint8_t k=0; k<networkStatus.iCount; k++) {
                    if(networkStatus.iConnectState & (1ULL<<k))
                    {
                        memcpy(realAcl.Data + (l * ADI_WIL_MAC_ADDR_SIZE), userAcl.Data + (k * ADI_WIL_MAC_ADDR_SIZE), ADI_WIL_MAC_ADDR_SIZE);
                        #ifdef DBG_ACL_MAP_TEST
                        realAcl_map[k] = true;
                        #endif
                        ADK_DEMO.BOOT = 320;
                        realAcl.iCount++;
                        l++;
                    }
                }
                returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY));
                ADK_DEMO.BOOT = 322;
                fatalOnWilError(adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES));
                ADK_DEMO.BOOT = 325;
                fatalOnWilError(adi_wil_example_ExecuteSetACL(realAcl.Data, realAcl.iCount));
                ADK_DEMO.BOOT = 330;
                fatalOnWilError(adi_wil_example_ExecuteGetACL(&packInstance));
                ADK_DEMO.BOOT = 331;
            }
        }while(!bAllNodesJoined);

        /* STEP 35 : Set Mode (STANDBY) **************************************************/
        returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY));
        ADK_DEMO.BOOT = 350;
    } 
    if(ADK_DEMO.OTAP_UPDATE_FORCE || ADK_DEMO.OTAP_RE_UPDATE_FORCE)
    {
        adk_debug_BootTimeLog(Interval, LogStart, 360, Demo_GetDeviceVersion_Mngr_Now________);
        adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_MANAGER_0, NO_SET_MODE);
        adk_debug_log_GetDeviceVersion(OTAP_Mngr, OTAP_NOW, 0);
        adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_MANAGER_1, NO_SET_MODE);
        adk_debug_log_GetDeviceVersion(OTAP_Mngr, OTAP_NOW, 1);
        ADK_DEMO.BOOT = 360;
        adk_debug_BootTimeLog(Interval, LogEnd__, 360, Demo_GetDeviceVersion_Mngr_Now________);

        adk_debug_BootTimeLog(Interval, LogStart, 361, Demo_GetDeviceVersion_Node_Now________);
        for(uint8_t k=0; k<networkStatus.iCount; k++) {
            adi_wil_example_GetDeviceVersion(&packInstance, 1ULL << k, NO_SET_MODE);
            adk_debug_log_GetDeviceVersion(OTAP_Node, OTAP_NOW, k);
        }
        ADK_DEMO.BOOT = 361;
        adk_debug_BootTimeLog(Interval, LogEnd__, 361, Demo_GetDeviceVersion_Node_Now________);   
    }
 

    /* STEP 36 : Set Mode (ACTIVE) *******************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 400, example_ExecuteSetMode_1______________);
    returnOnWilError(adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_ACTIVE));
    ADK_DEMO.BOOT = 400;
    adk_debug_BootTimeLog(Interval, LogEnd__, 400, example_ExecuteSetMode_1______________);

    /* STEP 37 : Enable Network Data Capture *********************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 420, Demo_EnableNetworkDataCapture_________);
    NWBufferSize = networkStatus.iCount*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL;
    adi_wil_EnableNetworkDataCapture(&packInstance, networkDataBuffer, (networkStatus.iCount*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL), true);
    ADK_DEMO.BOOT = 420;
    adk_debug_BootTimeLog(Interval, LogEnd__, 420, Demo_EnableNetworkDataCapture_________);
    gTotalNodes= networkStatus.iCount;     /*  @remark : Variable to store total no. of Nodes in network */

    /* STEP 38 : Init CB_CELL for demo ***************************************************/
    for(i=0; i<12; i++){
#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
        for(j=0; j<16; j++){
            ADK_DEMO.CB_CELL[i][j] = true;
        }
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
        for(j=0; j<18; j++){
            ADK_DEMO.CB_CELL[i][j] = true;
        }
#else   /* Not supported */
#endif
    }
    /* STEP 39 : Run task for Read BMS ***************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 450, Demo_PeriodicallyCallProcessTaskCB____);
    adi_wil_example_PeriodicallyCallProcessTaskCB();
    ADK_DEMO.BOOT = 450;
    adk_debug_BootTimeLog(Interval, LogEnd__, 450, Demo_PeriodicallyCallProcessTaskCB____);

    /* STEP 40 : Start scheduler *********************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 999, Demo_SchedulerInit____________________);
    if(true == adi_wil_example_SchedulerInit())
    {
        ADK_DEMO.BOOT = 999;
        adk_debug_BootTimeLog(Interval, LogEnd__, 999, Demo_SchedulerInit____________________);
        adk_debug_BootTimeLog(Overall_, LogEnd__, 999, Demo_Total_boot_time__________________);
        ADK_DEMO.BOOT_TIME = BOOT_TIME[0].DURATION;
        adi_wil_example_scheduleTasks();
    }
    else
    {
        /* Scheduler Initialization Failed */
        while(1);
    }

    return 0;
}

/******************************************************************************************/
/* End of adi_wil_example_Main                                                            */
/******************************************************************************************/


/******************************************************************************************/
/* OTAP Node v2.2.0 update                                                                */
/******************************************************************************************/
void adi_wil_example_OTA_mode()
{
    adk_debug_BootTimeLog(Interval, LogStart, 502, Demo_OTAP_SetMode_STANDBY_0___________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_SetMode_STANDBY_0___________;
    adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
    ADK_DEMO.BOOT = 502;
    adk_debug_BootTimeLog(Interval, LogEnd__, 502, Demo_OTAP_SetMode_STANDBY_0___________);
    
    adk_debug_BootTimeLog(Interval, LogStart, 503, Demo_OTAP_SetMode_OTAP________________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_SetMode_OTAP________________;
    adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_OTAP);
    ADK_DEMO.BOOT = 503;
    adk_debug_BootTimeLog(Interval, LogEnd__, 503, Demo_OTAP_SetMode_OTAP________________);

    adk_debug_BootTimeLog(Interval, LogStart, 504, Demo_OTAP_Wait_for_mngr_connect_0_____);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_Wait_for_mngr_connect_0_____;
    adi_wil_example_waitForEvent(ADI_WIL_EVENT_COMM_MGR_CONNECTED, ADI_WIL_NUM_NW_MANAGERS);
    ADK_DEMO.BOOT = 504;
    adk_debug_BootTimeLog(Interval, LogEnd__, 504, Demo_OTAP_Wait_for_mngr_connect_0_____);

    adk_debug_BootTimeLog(Interval, LogStart, 510, Demo_OTAP_GetNetworkStatus____________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_GetNetworkStatus____________;
    while (adi_wil_example_countUnconnectedNodes() != 0);
    ADK_DEMO.BOOT = 510;
    adk_debug_BootTimeLog(Interval, LogEnd__, 510, Demo_OTAP_GetNetworkStatus____________);
}

void adi_wil_example_OTA_Node()
{
    adk_debug_BootTimeLog(Interval, LogStart, 520, Demo_OTAP_LoadFile_OPFW_AllNodes______);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_LoadFile_OPFW_AllNodes______;
    adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_DEV_ALL_NODES, NO_SET_MODE);
    adi_wil_example_LoadFileRetry(ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_DEV_ALL_NODES);
    ADK_DEMO.BOOT = 520;
    adk_debug_BootTimeLog(Interval, LogEnd__, 520, Demo_OTAP_LoadFile_OPFW_AllNodes______);
}

/******************************************************************************************/
/* OTAP Manager v2.2.0 update                                                             */
/******************************************************************************************/
void adi_wil_example_OTA_MNG()
{
    adk_debug_BootTimeLog(Interval, LogStart, 530, Demo_OTAP_LoadFile_OPFW_AllMngrs______);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_LoadFile_OPFW_AllMngrs______;
    adi_wil_example_ExecuteLoadFile(&packInstance, ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_DEV_ALL_MANAGERS, NO_SET_MODE);
    adi_wil_example_LoadFileRetry(ADI_WIL_FILE_TYPE_FIRMWARE, ADI_WIL_DEV_ALL_MANAGERS);
    ADK_DEMO.BOOT = 530;
    adk_debug_BootTimeLog(Interval, LogEnd__, 530, Demo_OTAP_LoadFile_OPFW_AllMngrs______);

    adk_debug_BootTimeLog(Interval, LogStart, 531, Demo_OTAP_ResetDevice_AllNodes________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_ResetDevice_AllNodes________;
    adi_wil_example_ExecuteResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES);
    ADK_DEMO.BOOT = 531;
    adk_debug_BootTimeLog(Interval, LogEnd__, 531, Demo_OTAP_ResetDevice_AllNodes________);

    adk_debug_BootTimeLog(Interval, LogStart, 532, Demo_OTAP_SetMode_STANDBY_1___________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_SetMode_STANDBY_1___________;
    adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
    ADK_DEMO.BOOT = 532;
    adk_debug_BootTimeLog(Interval, LogEnd__, 532, Demo_OTAP_SetMode_STANDBY_1___________);

    adk_debug_BootTimeLog(Interval, LogStart, 533, Demo_OTAP_Wait_for_mngr_connect_1_____);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_Wait_for_mngr_connect_1_____;
    adi_wil_example_waitForEvent(ADI_WIL_EVENT_COMM_MGR_CONNECTED, ADI_WIL_NUM_NW_MANAGERS);
    ADK_DEMO.BOOT = 533;
    adk_debug_BootTimeLog(Interval, LogEnd__, 533, Demo_OTAP_Wait_for_mngr_connect_1_____);

    adk_debug_BootTimeLog(Interval, LogStart, 534, Demo_OTAP_Wait_for_Nodes_to_join______);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_Wait_for_Nodes_to_join______;
    adi_wil_example_waitForNodesToJoin();
    ADK_DEMO.BOOT = 534;
    adk_debug_BootTimeLog(Interval, LogEnd__, 534, Demo_OTAP_Wait_for_Nodes_to_join______);

    adk_debug_BootTimeLog(Interval, LogStart, 540, Demo_OTAP_SetMode_STANDBY_2___________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_SetMode_STANDBY_2___________;
    adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
    ADK_DEMO.BOOT = 540;
    adk_debug_BootTimeLog(Interval, LogEnd__, 540, Demo_OTAP_SetMode_STANDBY_2___________);

    adk_debug_BootTimeLog(Interval, LogStart, 541, Demo_OTAP_GetDeviceVersion_Mngr_Next__);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_GetDeviceVersion_Mngr_Next__;
    adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_MANAGER_0, NO_SET_MODE);
    adk_debug_log_GetDeviceVersion(OTAP_Mngr, OTAP_P1, 0);
    adi_wil_example_GetDeviceVersion(&packInstance, ADI_WIL_DEV_MANAGER_1, NO_SET_MODE);
    adk_debug_log_GetDeviceVersion(OTAP_Mngr, OTAP_P1, 1);
    ADK_DEMO.BOOT = 541;
    adk_debug_BootTimeLog(Interval, LogEnd__, 541, Demo_OTAP_GetDeviceVersion_Mngr_Next__);

    adk_debug_BootTimeLog(Interval, LogStart, 542, Demo_OTAP_GetDeviceVersion_Node_Next__);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_GetDeviceVersion_Node_Next__;
    for(uint8_t k=0; k<networkStatus.iCount; k++) {
        adi_wil_example_GetDeviceVersion(&packInstance, 1ULL << k, NO_SET_MODE);
        adk_debug_log_GetDeviceVersion(OTAP_Node, OTAP_P1, k);
    }
    ADK_DEMO.BOOT = 542;
    adk_debug_BootTimeLog(Interval, LogEnd__, 542, Demo_OTAP_GetDeviceVersion_Node_Next__);

    adk_debug_BootTimeLog(Interval, LogStart, 550, Demo_OTAP_Complete____________________);
    ADK_DEMO.OTAP_STAT = Demo_OTAP_Complete____________________;
    ADK_DEMO.BOOT = 550;
    adk_debug_BootTimeLog(Interval, LogEnd__, 550, Demo_OTAP_Complete____________________);
}

/******************************************************************************************/
/* BMS Data retrieval Task                                                                */
/******************************************************************************************/
void adi_task_bmsDataRetrieval(void)
{   
    if(adi_gNotifyBmsData == true) //&& (adi_gNotifyNetworkMeta == true))
    {
        adi_gNotifyBmsData = false;
        //adi_gNotifyNetworkMeta = false;
    }
}

/******************************************************************************************/
/* Script Select example                                                                  */
/******************************************************************************************/
/* @remark : This function will be called with 50ms period */
void adi_example_select_script_statemachine(void){

    if(ADK_DEMO.DEMO_MODE == NORMAL_BMS_SENSING){
        if(BASE_PACKET_RECEIVED == false){
            /*************************** Normal BMS ***************************************/
            adi_wil_example_ExecuteSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID, NO_SET_MODE); // BASE
        }
        else{
            if(ADK_DEMO.PREV_MODE == CELL_BALANCING_EVEN || ADK_DEMO.PREV_MODE == CELL_BALANCING_ODD){
                adi_wil_example_ExecuteModifyScript(&packInstance, CB_DEFAULT); //Default DCC
                for (selectscript_node_count = 0; selectscript_node_count < realAcl.iCount; selectscript_node_count++){
                    ADK_DEMO.NODE[selectscript_node_count].CB_STAT = 0;
                }
            }
            G_CB_INITIAL = 0;
        }
        ADK_DEMO.PREV_MODE = NORMAL_BMS_SENSING;
    }
    else if(ADK_DEMO.DEMO_MODE == CELL_BALANCING_EVEN){
        /*************************** Cell Balance *************************************/
        /* @warning : Cell balance scheduling period is depends on application scenario */
        if(BASE_PACKET_RECEIVED == false){
            /*************************** Normal BMS ***********************************/
            adi_wil_example_ExecuteSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID, NO_SET_MODE); // BASE
        }
        else{
            if(ADK_DEMO.PREV_MODE == CELL_BALANCING_ODD){
                #if 0       
                /* Disable to change default DCC */
                adi_wil_example_ExecuteModifyScript(&packInstance, CB_DEFAULT); //Default DCC
                for (selectscript_node_count = 0; selectscript_node_count < realAcl.iCount; selectscript_node_count++){
                    ADK_DEMO.NODE[selectscript_node_count].CB_STAT = 0;
                }
                #endif
                G_CB_INITIAL = 0;
            }          
            if(G_CB_INITIAL == 0){
                adi_wil_example_ExecuteModifyScript(&packInstance, CB_EVEN); //Even cells balancing       
                G_CB_INITIAL = 1;
            }
        }
        ADK_DEMO.PREV_MODE = CELL_BALANCING_EVEN;
    }
    else if(ADK_DEMO.DEMO_MODE == CELL_BALANCING_ODD){
        /*************************** Cell Balance *************************************/
        /* @warning : Cell balance scheduling period is depends on application scenario */
        if(BASE_PACKET_RECEIVED == false){
            /*************************** Normal BMS ***********************************/
            adi_wil_example_ExecuteSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID, NO_SET_MODE); // BASE
        }
        else{
            if(ADK_DEMO.PREV_MODE == CELL_BALANCING_EVEN){
                #if 0
                /* Disable to change default DCC */
                adi_wil_example_ExecuteModifyScript(&packInstance, CB_DEFAULT); // Default DCC
                for (selectscript_node_count = 0; selectscript_node_count < realAcl.iCount; selectscript_node_count++){
                    ADK_DEMO.NODE[selectscript_node_count].CB_STAT = 0;
                }
                #endif
                G_CB_INITIAL = 0;
            }                   
            if(G_CB_INITIAL == 0){
                adi_wil_example_ExecuteModifyScript(&packInstance, CB_ODD); // Odd cells balancing
                G_CB_INITIAL = 1;
            }        
        }        
        ADK_DEMO.PREV_MODE = CELL_BALANCING_ODD;
    }
    else if(ADK_DEMO.DEMO_MODE == KEY_ON_EVENT){
        if(KEY_ON == false)
        {
            KEY_ON = true;
            KEY_OFF = false;
            bFirstBMSdata=false;
            #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
            #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
            adi_gNotifyLatent0 = false;
            adi_gNotifyLatent1 = false;
            memset(latent0_recv_confirm, 0, ADK_MAX_node);
            memset(latent1_recv_confirm, 0, ADK_MAX_node);
            #else
            #endif
            adi_example_key_on_event();
            ADK_DEMO.PREV_MODE = KEY_ON_EVENT;
        }
    }
    else if(ADK_DEMO.DEMO_MODE == KEY_OFF_EVENT){
        if(KEY_OFF == false)
        {
            adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
            KEY_OFF = true;
            KEY_ON = false;
            iInitBuffer0index = iInitBuffer1index = iInitBuffer2index = 0;
            memset(initBuffer0, 0, sizeof(initBuffer0));
            memset(initBuffer1, 0, sizeof(initBuffer1));
            memset(initBuffer2, 0, sizeof(initBuffer2));
            
            #if 1
            #if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
            #elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
            /* change script to Latent0 */
            adi_wil_example_ExecuteSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_SENSOR_ID_BMS, ADI_BMS_LATENT0_ID, NO_SET_MODE);
            while(1)
            {
                if(adi_gNotifyLatent0) {break;}
            }
            /* change script to Latent1 */
            adi_wil_example_ExecuteSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_SENSOR_ID_BMS, ADI_BMS_LATENT1_ID, NO_SET_MODE);
            while(1)
            {
                if(adi_gNotifyLatent1) {break;}
            }
            ADK_DEMO.BOOT = 750;
            #else
            #endif
            #endif

            /* Set to Standby mode */
            noreturnOnWilError(adi_wil_SetMode(&packInstance, ADI_WIL_MODE_STANDBY));
            
            /* Wait for non-blocking API to complete */
            WaitForWilAPI(&packInstance);
            
            if(gNotifRc == ADI_WIL_ERR_PARTIAL_SUCCESS) {
                noreturnOnWilError(adi_wil_example_RetrySetMode(&packInstance, ADI_WIL_MODE_STANDBY));
            }
            ADK_DEMO.BOOT = 760;

            /* Set to Sleep mode */
            noreturnOnWilError(adi_wil_SetMode(&packInstance, ADI_WIL_MODE_SLEEP));

            /* Wait for non-blocking API to complete */
            WaitForWilAPI(&packInstance);
            ADK_DEMO.BOOT = 761;
            
            /* Disconnect the pack from the system. This is a blocking call so no API
            callback is generated. */
            errorCode = adi_wil_Disconnect(&packInstance);

            if (errorCode != ADI_WIL_ERR_SUCCESS)
            {
                adk_debug_Report(DBG_wil_Disconnect, errorCode);
            }
            else
            {
                //adi_wil_ex_info("Disconnect from pack successful!");
            }
            ADK_DEMO.BOOT = 762;
            if ((errorCode = adi_wil_Terminate()) != ADI_WIL_ERR_SUCCESS)
            {
                adk_debug_Report(DBG_wil_Terminate, errorCode);
            }
            ADK_DEMO.BOOT = 763;
            ADK_DEMO.PREV_MODE = KEY_OFF_EVENT;
        }
    }
}

void adi_example_key_on_event(void){
    adi_wil_err_t rc;
    adi_wil_network_status_t networkStatus;
    ADK_DEMO.BOOT = 700;
    adk_debug_TickerBTInit();
    ADK_DEMO.BOOT = 701;
    adk_debug_BootTimeLog(Overall_, LogStart, 730, Demo_key_on_event_____________________);

    adk_debug_BootTimeLog(Interval, LogStart, 703, Demo_TaskStart________________________);
    adi_wil_example_ExecuteInitialize();
    ADK_DEMO.BOOT = 702;
    adi_wil_example_PeriodicallyCallProcessTask();
    ADK_DEMO.BOOT = 703;
    adk_debug_BootTimeLog(Interval, LogEnd__, 703, Demo_TaskStart________________________);

    adk_debug_BootTimeLog(Interval, LogStart, 710, Demo_ExcuteConnect____________________);
    rc = (adi_wil_example_ExecuteConnect(&packInstance,
                                        wbmsSysSensorData,
                                        (BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT)));
    ADK_DEMO.BOOT = 710;

    /* Set to Standby mode */
    //noreturnOnWilError(adi_wil_SetMode(&packInstance, ADI_WIL_MODE_STANDBY));
    
    /* Wait for non-blocking API to complete */
    //WaitForWilAPI(&packInstance);
    // ADK_DEMO.BOOT = 711;
    adk_debug_BootTimeLog(Interval, LogEnd__, 710, Demo_ExcuteConnect____________________);

    if(realAcl.iCount == 0){          
        memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
    }
    
    adk_debug_BootTimeLog(Interval, LogStart, 712, Demo_GetNetworkStatusACTmode__________);
    adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &networkStatus, NO_SET_MODE);
    ADK_DEMO.BOOT = 712;
    adk_debug_BootTimeLog(Interval, LogEnd__, 712, Demo_GetNetworkStatusACTmode__________);

    adk_debug_BootTimeLog(Interval, LogStart, 720, Demo_TaskStartCB______________________);
    adi_wil_example_ExecuteSetMode(&packInstance, ADI_WIL_MODE_ACTIVE);
    ADK_DEMO.BOOT = 713;
    adi_wil_EnableNetworkDataCapture(&packInstance, networkDataBuffer, (networkStatus.iCount*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL), true);
    ADK_DEMO.BOOT = 714;
    adi_wil_example_PeriodicallyCallProcessTaskCB();
    ADK_DEMO.BOOT = 720;
    adk_debug_BootTimeLog(Interval, LogEnd__, 720, Demo_TaskStartCB______________________);

    if(WAKEUP)
    {
        WAKEUP = false;
        if(true == adi_wil_example_SchedulerInit())
        {
            ADK_DEMO.BOOT = 999;            
            adi_wil_example_scheduleTasks();
        }
        else
        {
            /* Scheduler Initialization Failed */
            while(1);
        }
    }
    else
    {
        ADK_DEMO.BOOT = 999;
    }
}

void adi_example_null_function(void){
    
}

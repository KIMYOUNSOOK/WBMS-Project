/*
 * CmicM.c
 *
 *  Created on: 2023. 10. 12.
 *      Author: sure
 */

#include <string.h>

#include "CmicM.h"
#include "CmicMConfig.h"
#include "adi_wil_app_interface.h"



typedef struct
{
	MAIN_STATE_E	m_eMain;
    BOOT_STATE_E    m_eBoot;
    BOOTSUB_INIT_E      m_eSubInit;
    BOOTSUB_CONNECT_E   m_eSubConnect;
	BOOTSUB_JOIN_E		m_eSubJoin;
	BOOTSUB_ACTIVE_E	m_eSubActive;
	MAIN_STATE_E		m_ePrevMain;  //Previous MainState
	MAINSUB_SENSING_E	m_eSubSensing;	//Sensing SubState
	MAINSUB_BALANCING_E  m_eSubBalancing; //Balancing SubState
}CmicM_State_t;

typedef struct
{

    uint16 					m_nTaskCnt;
	uint16					m_nDebugCnt1;
	uint16					m_nDebugCnt2;
	uint32  				m_nTick1ms;
	uint16   				m_nBOOT;
	float					m_fBOOT_TIME;
	CmicM_State_t			m_tSt;
	adi_wil_configuration_t m_portConfig[2];
	adi_wil_sensor_data_t   m_wbmsSysSensorData[BMS_DATA_PACKET_COUNT];
	adi_wil_sensor_data_t	m_userBMSBuf[BMS_DATA_PACKET_COUNT];
	sint16 					m_tempBuf[22];

	adi_wil_acl_t			m_sysAcl;
	adi_wil_network_status_t m_networkStatus;
	adi_wil_network_data_t   m_networkDataBuffer[ADI_WIL_MAX_NODES*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
	adi_wil_err_t			m_notifyRC;
    NODESTR 				m_NODE;
    bool 					m_bCB_CELL[12][ADK_MAX_cell];
    bool 					m_bCB_NODE[12];
	
	uint8 *					m_pOtapImage;
    uint32 					m_iOtapImageLen;
    uint32 					m_iImageLength;
	client_data_t 			m_clientData;
    adi_bms_base_pkt_0_t* 	pBMS_Pkt_0;
    adi_bms_base_pkt_1_t* 	pBMS_Pkt_1;
    adi_bms_base_pkt_2_t* 	pBMS_Pkt_2;

	uint8           		m_nPortCnt;
	uint8					m_nMgrConnectCnt;
	uint8					m_nMgrDisConnectCnt;
	uint8					m_nNodeConnectCnt;

	bool					m_bAclUpdateNoNeed;
	bool					m_bAclUpdateForce;
	bool					m_bAllNodesJoined;
	
	uint16					m_nTotalPacketRcvd;	   /*  @remark : Variable to store total no. of bms packets received */
	
} CmicM_Instance_t;

static CmicM_Instance_t CmicM_Inst;
static void CmicM_ControlBootState(void);
static void CmicM_ControlSensingState(void);
static void CmicM_ControlBalancingState(void);

static void Cmic_Init_Step1_REQ(void);
static void Cmic_Init_Step1_RES(void);

static void Cmic_Connect_Step1_REQ(void);
static void Cmic_Connect_Step1_RES(void);
static void Cmic_Connect_Step2_REQ(void);
static void Cmic_Connect_Step2_RES(void);
static void Cmic_Connect_Step3_REQ(void);
static void Cmic_Connect_Step3_RES(void);
static void Cmic_Connect_Step4_REQ(void);
static void Cmic_Connect_Step4_RES(void);
static void Cmic_Connect_Step5_REQ(void);
static void Cmic_Connect_Step5_RES(void);

static void Cmic_Connect_Step6_REQ(void);
static void Cmic_Connect_Step6_RES(void);
static void Cmic_Connect_Step7_REQ(void);
static void Cmic_Connect_Step7_RES(void);
static void Cmic_Connect_Step8_REQ(void);
static void Cmic_Connect_Step8_RES(void);

static void Cmic_Join_Step1_REQ(void);
static void Cmic_Join_Step1_RES(void);

static void Cmic_Active_Step1_REQ(void);
static void Cmic_Active_Step1_RES(void);
static void Cmic_Active_Step2_REQ(void);
static void Cmic_Active_Step2_RES(void);
static void Cmic_Active_Step3_REQ(void);
static void Cmic_Active_Step3_RES(void);
static void Cmic_Active_Step4_REQ(void);
static void Cmic_Active_Step4_RES(void);
static void Cmic_Active_Step5_REQ(void);
static void Cmic_Active_Step5_RES(void);
static void Cmic_Active_Step6_REQ(void);
static void Cmic_Active_Step6_RES(void);

static void Cmic_Active_Step7_REQ(void);
static void Cmic_Active_Step7_RES(void);
static void Cmic_Active_Step8_REQ(void);
static void Cmic_Active_Step8_RES(void);
static void Cmic_Active_Step9_REQ(void);
static void Cmic_Active_Step9_RES(void);
static void Cmic_Active_Step10_REQ(void);
static void Cmic_Active_Step10_RES(void);

static void	Cmic_Sensing_Step0_IDLE(void);
static void	Cmic_Sensing_Step1_REQ(void);
static void	Cmic_Sensing_Step1_RES(void);

static void	Cmic_Balancing_Step0_IDLE(void);
static void	Cmic_Balancing_Step1_REQ(void);
static void	Cmic_Balancing_Step1_RES(void);

typedef void (*pFunc)(void); 

static const pFunc  pArrayInit[]=
{
	Cmic_Init_Step1_REQ, //eINIT_st1_REQ
	Cmic_Init_Step1_RES, //eINIT_st1_RES
};

static const pFunc  pArrayConnect[]=
{
	Cmic_Connect_Step1_REQ, //eCONNECT_st1_REQ
	Cmic_Connect_Step1_RES, //eCONNECT_st1_RES
	Cmic_Connect_Step2_REQ,	//	eCONNECT_st2_REQ, //
	Cmic_Connect_Step2_RES,	//eCONNECT_st2_RES, //
	Cmic_Connect_Step3_REQ,	//eCONNECT_st3_REQ, //
	Cmic_Connect_Step3_RES,	//eCONNECT_st3_RES, //	
	Cmic_Connect_Step4_REQ,	//eCONNECT_st4_REQ, //
	Cmic_Connect_Step4_RES,	//eCONNECT_st4_RES, //
	Cmic_Connect_Step5_REQ,	//eCONNECT_st5_REQ, //
	Cmic_Connect_Step5_RES,	//eCONNECT_st5_RES, //
		
	Cmic_Connect_Step6_REQ,	//eCONNECT_st6_REQ, //
	Cmic_Connect_Step6_RES,	//eCONNECT_st6_RES, //
	Cmic_Connect_Step7_REQ,	//eCONNECT_st7_REQ, //
	Cmic_Connect_Step7_RES,	//eCONNECT_st7_RES, //
	Cmic_Connect_Step8_REQ,	//eCONNECT_st8_REQ, //
	Cmic_Connect_Step8_RES,	//eCONNECT_st8_RES, //
};

static const pFunc  pArrayJoin[]=
{
	Cmic_Join_Step1_REQ, //eJOIN_st1_REQ
	Cmic_Join_Step1_RES, //eJOIN_st1_RES
};

static const pFunc  pArrayActive[]=
{
	Cmic_Active_Step1_REQ, //eACTIVE_st1_REQ, //
	Cmic_Active_Step1_RES, //eACTIVE_st1_RES, //
	Cmic_Active_Step2_REQ, //eACTIVE_st2_REQ, //
	Cmic_Active_Step2_RES, //eACTIVE_st2_RES, //	
	Cmic_Active_Step3_REQ, //eACTIVE_st3_REQ, //
	Cmic_Active_Step3_RES, //eACTIVE_st3_RES, //	
	Cmic_Active_Step4_REQ, //eACTIVE_st4_REQ, //
	Cmic_Active_Step4_RES, //eACTIVE_st4_RES, //	
	Cmic_Active_Step5_REQ, //eACTIVE_st5_REQ, //
	Cmic_Active_Step5_RES, //eACTIVE_st5_RES, //
	Cmic_Active_Step6_REQ, //eACTIVE_st6_REQ, //
	Cmic_Active_Step6_RES, //eACTIVE_st6_RES, //
	Cmic_Active_Step7_REQ, //eACTIVE_st7_REQ, //
	Cmic_Active_Step7_RES, //eACTIVE_st7_RES, //	
	Cmic_Active_Step8_REQ, //eACTIVE_st8_REQ, //
	Cmic_Active_Step8_RES, //eACTIVE_st8_RES, //	
	Cmic_Active_Step9_REQ, //eACTIVE_st9_REQ, //
	Cmic_Active_Step9_RES, //eACTIVE_st9_RES, //	
	Cmic_Active_Step10_REQ, //eACTIVE_st10_REQ, //
	Cmic_Active_Step10_RES, //eACTIVE_st10_RES, //	
};

static const pFunc pArraySensing[]={
	Cmic_Sensing_Step0_IDLE,
	Cmic_Sensing_Step1_REQ,
	Cmic_Sensing_Step1_RES,
};

static const pFunc pArrayBalancing[]={
	Cmic_Balancing_Step0_IDLE,
	Cmic_Balancing_Step1_REQ,
	Cmic_Balancing_Step1_RES,
};

extern void Adbms683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt2(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Cell_OWD(adi_wil_device_t eNode);
extern void adk_debug_BootTimeLog(bool final, bool start, uint16_t step, ADK_LOG_FUNCTION api);
extern boolean IsReleaseWilAPI(void const * const pPack);
extern adi_wil_pack_t packInstance;
extern adi_wil_acl_t  realAcl;
extern uint8		  iOWDPcktsRcvd[ADI_WIL_MAX_NODES];
extern BOOTTIMESTR 		BOOT_TIME;
extern volatile adi_wil_err_t adi_gProcessTaskErrorCode;



void CmicM_Init(void)
{

	memset(&CmicM_Inst, 0, sizeof(CmicM_Instance_t));

	CmicM_Inst.m_nBOOT = 1;
	
	adk_debug_TickerBTInit();

	CmicM_Inst.m_nTick1ms = (uint32)GetTick_1ms();

	CmicM_Inst.m_tSt.m_eMain = eMAIN_BOOT;
	CmicM_Inst.m_tSt.m_eBoot = eBOOT_INIT;
	CmicM_Inst.m_tSt.m_eSubInit = eINIT_st1_REQ;

   adk_debug_BootTimeLog(Overall_, LogStart, 999, Demo_Total_boot_time__________________);
 
}



void CmicM_Handler(void)
{
	uint32  aTick;
    
	aTick =	GetTick_1ms();

	if (CmicM_Inst.m_nTick1ms != aTick) { //1ms condition.

	    CmicM_Inst.m_nTick1ms = aTick;

		switch(CmicM_Inst.m_tSt.m_eMain)
		{
			case eMAIN_BOOT :
				CmicM_ControlBootState();
				break;
			case eMAIN_SENSING :
				CmicM_ControlSensingState();			
				break;
			case eMAIN_BALANCING_EVEN :						
			case eMAIN_BALANCING_ODD :
				CmicM_ControlBalancingState();
				break;
			default :
				break;
		}
		
		
	}
}

static void CmicM_ControlSensingState(void)
{
	if ( pArraySensing[CmicM_Inst.m_tSt.m_eSubSensing] != 0){
		pArraySensing[CmicM_Inst.m_tSt.m_eSubSensing]();
	}else {
		//error
	}
}

static void CmicM_ControlBalancingState(void)
{
	if ( pArrayBalancing[CmicM_Inst.m_tSt.m_eSubBalancing] != 0){
		pArrayBalancing[CmicM_Inst.m_tSt.m_eSubBalancing]();
	}else {
		//error
	}
}

static void CmicM_ControlBootState(void)
{

    switch(CmicM_Inst.m_tSt.m_eBoot)
   {
		case eBOOT_INIT :
			if ( pArrayInit[CmicM_Inst.m_tSt.m_eSubInit] != 0){
				pArrayInit[CmicM_Inst.m_tSt.m_eSubInit]();
			}else {
			//error
			}
			break;
			
		case eBOOT_CONNECT :
			if ( pArrayConnect[CmicM_Inst.m_tSt.m_eSubConnect] != 0){
				pArrayConnect[CmicM_Inst.m_tSt.m_eSubConnect]();
			}else {
			//error
			}
			break;
		case eBOOT_JOIN :
			if ( pArrayJoin[CmicM_Inst.m_tSt.m_eSubJoin] != 0){
				pArrayJoin[CmicM_Inst.m_tSt.m_eSubJoin]();
			}else {	
			//error
			}
			break;
		case eBOOT_ACTIVE :
			if ( pArrayActive[CmicM_Inst.m_tSt.m_eSubActive] != 0){
				pArrayActive[CmicM_Inst.m_tSt.m_eSubActive]();
			}else {
			//error
			}
			break;
		default :
			break;
    }


}


/********************************************************************************
	FUNCTION :  Cmic_Init_Step1_REQ
    	STATUS :   BOOTSUB_INIT_E =  eINIT_st1_REQ, 
    	DESCRIPTION : REQUEST WBMS INIT & QUERY DEVICE 
**********************************************************************************/
static void Cmic_Init_Step1_REQ(void)
{

	/* STEP 1  : WBMS System Initialization **********************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 100, Demo_ExecuteInitialize________________);
	Cmic_ExecuteInitialize();
	
	CmicM_Inst.m_nBOOT = 100;
	adk_debug_BootTimeLog(Interval, LogEnd__, 100, Demo_ExecuteInitialize________________);
	
	/* STEP 2  : Start calling processTask periodically **********************************/
	adk_debug_BootTimeLog(Interval, LogStart, 101, Demo_PeriodicallyCallProcessTask______);
	Cmic_CallProcessTask();
	
	CmicM_Inst.m_nBOOT = 101;
	adk_debug_BootTimeLog(Interval, LogEnd__, 101, Demo_PeriodicallyCallProcessTask______);
	
	/* STEP 3  : Query device ************************************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 102, Demo_ExecuteQueryDevice_______________);
	/* Determine whether the two managers on the two SPI ports are in dual manager mode */
	CmicM_Inst.m_nPortCnt=0;
	Cmic_RequestQueryDevice(CmicM_Inst.m_nPortCnt);
	
	CmicM_Inst.m_tSt.m_eSubInit = eINIT_st1_RES;

}

/********************************************************************************
	FUNCTION :  Cmic_Init_Step1_RES
    	STATUS :   BOOTSUB_INIT_E =  eINIT_st1_RES, 
    	DESCRIPTION : RESPONSE  WBMS INIT & QUERY DEVICE 
**********************************************************************************/
static void Cmic_Init_Step1_RES(void)
{
	
	adi_wil_pack_t *pPackLock = NULL_PTR;
	
    (void)memset(&pPackLock, 0xFF, sizeof(void*));

	if (IsReleaseWilAPI(pPackLock))
	{
		if(!CmicM_Inst.m_nPortCnt){
			Cmic_RequestQueryDevice(++CmicM_Inst.m_nPortCnt);
		}
		else {
			
			CmicM_Inst.m_nBOOT	= 102;
			adk_debug_BootTimeLog(Interval, LogEnd__, 102, Demo_ExecuteQueryDevice_______________);

			/* CONDITION 1 : Check Dual Manager configuration ************************************/
			adk_debug_BootTimeLog(Interval, LogStart, 110, Demo_isDualConfig_____________________);

			CmicM_Inst.m_tSt.m_eBoot = eBOOT_CONNECT;
			
			if (!Cmic_IsDualConfig(&CmicM_Inst.m_portConfig[0], &CmicM_Inst.m_portConfig[1]))
			{
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st1_REQ;
				CmicM_ControlBootState();

			}else
			{
			#ifdef _SURE_DBG
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st1_REQ; //case of not dual config 
			#else
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st6_REQ;
			#endif
				CmicM_ControlBootState();
			}
			adk_debug_BootTimeLog(Interval, LogEnd__, 110, Demo_isDualConfig_____________________);
			CmicM_Inst.m_tSt.m_eSubInit = eINIT_stEND;
			
		}
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	}

}


static void Cmic_Connect_Step1_REQ(void)
{
	/* Dual Manager mode config download for empty device */
	 /* STEP 4	: Connect *************************************************************/
	 adk_debug_BootTimeLog(Interval, LogStart, 120, Demo_ExecuteConnect_0_________________);


	 Cmic_RequestConnect(&packInstance,	  CmicM_Inst.m_wbmsSysSensorData,
										  (BMS_DATA_PACKET_COUNT));

	 CmicM_Inst.m_nBOOT = 120;
	 
	 CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st1_RES;
	 CmicM_Inst.m_nTaskCnt=0;
	 CmicM_Inst.m_nPortCnt=0;

}

static void Cmic_Connect_Step1_RES(void)
{

		if (IsReleaseWilAPI(&packInstance)){
	
			adk_debug_BootTimeLog(Interval, LogEnd__, 120, Demo_ExecuteConnect_0_________________);

			if ((CmicM_Inst.m_notifyRC == ADI_WIL_ERR_SUCCESS) || 
				(CmicM_Inst.m_notifyRC == ADI_WIL_ERR_CONFIGURATION_MISMATCH) || 
				(CmicM_Inst.m_notifyRC == ADI_WIL_ERR_INVALID_STATE))
				{
					CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st2_REQ;
					CmicM_ControlBootState();
				}
			else{ //error 
				//fatalError(CmicM_Inst.m_notifyRC);
			}	
	
		}
		else {
			if( ++CmicM_Inst.m_nTaskCnt > 100) {
					//Fault
			}	
		}

}

static void Cmic_Connect_Step2_REQ(void)
{
	/* STEP 5  : Reset Device (ALL MANAGERS) *************************************/
	   adk_debug_BootTimeLog(Interval, LogStart, 121, Demo_ExecuteResetDevice_single_mngr_0_);
	
	   Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS);

	   CmicM_Inst.m_nBOOT = 121;
	   
	   
	   CmicM_Inst.m_nTaskCnt = 0;
	   CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st2_RES;
	   	   
}

static void Cmic_Connect_Step2_RES(void)
{
	static bool bIsReleased = FALSE;
	
	if (!bIsReleased){

		if (IsReleaseWilAPI(&packInstance)){

				bIsReleased = TRUE;	
				CmicM_Inst.m_nMgrConnectCnt = 0;
		}
		
	}
	else if (CmicM_Inst.m_nMgrConnectCnt >= ADI_WIL_NUM_NW_MANAGERS){

		bIsReleased = FALSE;
	   adk_debug_BootTimeLog(Interval, LogEnd__, 121, Demo_ExecuteResetDevice_single_mngr_0_);

	   CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st3_REQ;
		CmicM_ControlBootState();	
	}
	else {
	
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}	
	}
			
}


static void Cmic_Connect_Step3_REQ(void)
{
	adi_wil_mode_t   currentMode;

	Cmic_ExecuteGetMode(&packInstance, &currentMode);

	 if ( ADI_WIL_MODE_STANDBY != currentMode)
	 {		 
	 	 Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
		
	     CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st3_RES;
		 CmicM_Inst.m_nTaskCnt=0;
	 }
	 else { //currentMode == ADI_WIL_MODE_STANDBY
	 
	 	 CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st4_REQ;

	 }	
	

}

static void Cmic_Connect_Step3_RES(void)
{
		
	if (IsReleaseWilAPI(&packInstance)){
		
		if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
		 	
			 CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st3_REQ;
			 return;
		}	

		CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st4_REQ;
		CmicM_ControlBootState();
		   
	}else {

		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}	

	}
}


static void Cmic_Connect_Step4_REQ(void)
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

	/* STEP 6  : Load files (CONFIGURATION, ALL MANAGERS) ************************/
	adk_debug_BootTimeLog(Interval, LogStart, 122, Demo_ExecuteLoadFile_singlecon_allmngr);

	CmicM_Inst.m_clientData.LoadFileStatus.iOffset = 0;
	  
	CmicM_Inst.m_pOtapImage = (uint8 *)&configuration_file_configuration;
	CmicM_Inst.m_iOtapImageLen =  configuration_file_configuration_length;

	errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_DEV_ALL_MANAGERS);

	if (errorCode != ADI_WIL_ERR_SUCCESS){
		// adk_debug_Report(DBG_wil_LoadFile, errorCode);
	}
	CmicM_Inst.m_nBOOT = 122;
	CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st4_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Connect_Step4_RES(void)
{
	adi_wil_err_t errorCode;

	if (IsReleaseWilAPI(&packInstance)){

		switch(CmicM_Inst.m_notifyRC)
		{
			case  ADI_WIL_ERR_FAIL :
			case  ADI_WIL_ERR_TIMEOUT :
				CmicM_Inst.m_clientData.LoadFileStatus.iOffset = 0;
				
				errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_DEV_ALL_MANAGERS);
				break;
				
			case  ADI_WIL_ERR_IN_PROGRESS :	
				
				errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_DEV_ALL_MANAGERS);
				break;

			case  ADI_WIL_ERR_PARTIAL_SUCCESS :
			case  ADI_WIL_ERR_SUCCESS :
				adk_debug_BootTimeLog(Interval, LogEnd__, 122, Demo_ExecuteLoadFile_singlecon_allmngr);
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st5_REQ;
				CmicM_ControlBootState();
				break;

						
			default :	
				 //  adk_debug_Report(DBG_wil_LoadFile_wait, errorCode);    
				 break;
		}
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}		
	}
}

adi_wil_err_t Cmic_RequestLoadFileConfig(adi_wil_device_t eDevice)
{

    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
			
	errorCode = adi_wil_LoadFile(&packInstance, eDevice, ADI_WIL_FILE_TYPE_CONFIGURATION,
				&CmicM_Inst.m_pOtapImage[CmicM_Inst.m_clientData.LoadFileStatus.iOffset]);
	
    return errorCode;
}

static void Cmic_Connect_Step5_REQ(void)
{
	/* STEP 7  : Reset Device (ALL MANAGERS) *************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 123, Demo_ExecuteResetDevice_single_mngr_1_);
	
	Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS);
	  
	CmicM_Inst.m_nBOOT = 123;
	//CmicM_Inst.m_nMgrConnectCnt = 0;
	CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st5_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Connect_Step5_RES(void)
{
	static bool bIsReleased = FALSE;
	
	if (!bIsReleased){

		if (IsReleaseWilAPI(&packInstance)){

				bIsReleased = TRUE;		
				CmicM_Inst.m_nMgrConnectCnt = 0;
		}
		
	}
	else if (CmicM_Inst.m_nMgrConnectCnt >= ADI_WIL_NUM_NW_MANAGERS){

		bIsReleased = FALSE;
		adk_debug_BootTimeLog(Interval, LogEnd__, 123, Demo_ExecuteResetDevice_single_mngr_1_);
		
		/* STEP 8  : Disconnect ******************************************************/
		adk_debug_BootTimeLog(Interval, LogStart, 123, Demo_ExecuteDisconnect_0______________);
		
		Cmic_ExecuteDisconnect(&packInstance);
		
		CmicM_Inst.m_nBOOT = 124;
		adk_debug_BootTimeLog(Interval, LogEnd__, 123, Demo_ExecuteDisconnect_0______________);

		/* STEP  9 : Check Dual Manager configuration ************************************/
		Cmic_IsDualConfig(&CmicM_Inst.m_portConfig[0], &CmicM_Inst.m_portConfig[1]);
			  

		CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st6_REQ;
		CmicM_ControlBootState();
			
	}
	else {
	
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}	
	}

}

static void Cmic_Connect_Step6_REQ(void)
{
    /* STEP 10 : Connect *****************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 130, Demo_ExecuteConnect_1_________________);

	
    Cmic_RequestConnect(&packInstance,  CmicM_Inst.m_wbmsSysSensorData,
                                       (BMS_DATA_PACKET_COUNT));
    CmicM_Inst.m_nBOOT = 130;

    CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st6_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Connect_Step6_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

	    adk_debug_BootTimeLog(Interval, LogEnd__, 130, Demo_ExecuteConnect_1_________________);
		CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st7_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}
}

static void Cmic_Connect_Step7_REQ(void)
{
	  adi_wil_mode_t currentMode;
		
	/* CONDITION 2 : Check STANDBY mode **************************************************/
	 adk_debug_BootTimeLog(Interval, LogStart, 140, Demo_ExecuteSetMode_0_________________);
	 CmicM_Inst.m_nBOOT = 140;

	 if ( ADI_WIL_ERR_SUCCESS != Cmic_ExecuteGetMode(&packInstance, &currentMode))
 	{
	 	//error 
	}
	
	
	 if ( ADI_WIL_MODE_STANDBY != currentMode)
	 {
		  /* STEP 11 : Set Mode (STANDBY) ******************************************************/
	 	 Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
		
	     CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st7_RES;
		 CmicM_Inst.m_nTaskCnt=0;
	 }
	 else { //currentMode == ADI_WIL_MODE_STANDBY
	 
	 	 CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st8_REQ;
		 adk_debug_BootTimeLog(Interval, LogEnd__, 140, Demo_ExecuteSetMode_0_________________);
	 }	
	 
}

static void Cmic_Connect_Step7_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

	    adk_debug_BootTimeLog(Interval, LogEnd__, 140, Demo_ExecuteSetMode_0_________________);
		CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st8_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}
}

static void Cmic_Connect_Step8_REQ(void)
{
    /* STEP 12 : Get Acl *****************************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 150, Demo_ExecuteGetACL_0__________________);
    Cmic_RequestGetACL(&packInstance);
  
    CmicM_Inst.m_nBOOT = 150;

    CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st8_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Connect_Step8_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		adk_debug_BootTimeLog(Interval, LogEnd__, 150, Demo_ExecuteGetACL_0__________________);

		/* STEP 13 : Compare Acl *************************************************************/
		adk_debug_BootTimeLog(Interval, LogStart, 155, Demo_ADK_CompareACL___________________);
		
		CmicM_Inst.m_bAclUpdateNoNeed = Cmic_CompareACL();
		CmicM_Inst.m_nBOOT = 155;

		adk_debug_BootTimeLog(Interval, LogEnd__, 155, Demo_ADK_CompareACL___________________);

		if (CmicM_Inst.m_bAclUpdateNoNeed == false || CmicM_Inst.m_bAclUpdateForce == true )
		{ //New Node Join

			CmicM_Inst.m_tSt.m_eBoot = eBOOT_JOIN;
			CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st1_REQ;
		}else {

			CmicM_Inst.m_tSt.m_eBoot = eBOOT_ACTIVE;
			CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st1_REQ;
		}
		CmicM_ControlBootState();
		CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_stEND;
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}
}

static void Cmic_Join_Step1_REQ(void)
{
	
}

static void Cmic_Join_Step1_RES(void)
{

}

static void Cmic_Active_Step1_REQ(void)
{
	adi_wil_mode_t currentMode;
	
	/* STEP 32 : realAcl generation from userAcl *************************************/
   if(realAcl.iCount == 0){ 		
   	
	   memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
 
	   CmicM_Inst.m_nBOOT = 290;
   }
   
   /* STEP 33 : Set Mode (COMMISSIONING) ********************************************/
   adk_debug_BootTimeLog(Interval, LogStart, 300, Demo_GetNetworkStatus_________________);

   Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode )
	{
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY );	
		
	}else { //STANDBY -> COMMISSIONING 
	
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_COMMISSIONING );
	}

	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st1_RES;
	CmicM_Inst.m_nTaskCnt=0;   
}


static void Cmic_Active_Step1_RES(void)
{
	adi_wil_mode_t currentMode;

	if (IsReleaseWilAPI(&packInstance)){

		Cmic_ExecuteGetMode(&packInstance, &currentMode);

		switch(currentMode) //STANDBY -> COMMISSIONING 
		{
			case  ADI_WIL_MODE_COMMISSIONING :
			 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st2_REQ;
			 CmicM_ControlBootState();
			 break;
			case  ADI_WIL_MODE_STANDBY :
			 Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_COMMISSIONING );
			 break;
			default :
			 Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
			 break;
		 }

	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}
}


static void Cmic_Active_Step2_REQ(void)
{
	/* STEP 34 : Get Network Status **************************************************/
     CmicM_Inst.m_bAllNodesJoined = Cmic_RequestGetNetworkStatus(&packInstance, &CmicM_Inst.m_networkStatus);
	 
     CmicM_Inst.m_nBOOT = 300;
     adk_debug_BootTimeLog(Interval, LogEnd__, 300, Demo_GetNetworkStatus_________________);

	 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st2_RES;
	 CmicM_ControlBootState();
}

static void Cmic_Active_Step2_RES(void)
{

	if ( !CmicM_Inst.m_bAllNodesJoined )
	{
		/****320. realAcl generate ***************** */
		uint8 l = 0;
		CmicM_Inst.m_nBOOT = 301;
		memset(&realAcl, 0, sizeof(adi_wil_acl_t));
	  
		for(uint8 k = 0; k < CmicM_Inst.m_networkStatus.iCount; k++) 
		{
			if(CmicM_Inst.m_networkStatus.iConnectState & (1ULL << k ))
			{
				memcpy(realAcl.Data + (l * ADI_WIL_MAC_ADDR_SIZE), userAcl.Data + (k * ADI_WIL_MAC_ADDR_SIZE), ADI_WIL_MAC_ADDR_SIZE);
	   
				CmicM_Inst.m_nBOOT = 320;
				realAcl.iCount++;
				l++;
			}
		}
	 	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st3_REQ;

	}
	else {
		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st7_REQ;
	}
	CmicM_ControlBootState();

}

static void Cmic_Active_Step3_REQ(void)
{
	
	/****322. SetMode(STANDBY) ***************** */ //COMMISSIONING ->STANDBY 
	Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
	
    CmicM_Inst.m_nBOOT = 322;
	
	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st3_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Active_Step3_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		 if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
		 	
			 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st3_REQ;
			 return;
		 }	

		 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st4_REQ;
		 CmicM_ControlBootState();
	
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}

static void Cmic_Active_Step4_REQ(void)
{
	/****325. ResetDevice(All nodes)***************** */
	Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES);

	CmicM_Inst.m_nBOOT = 325;
	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st4_RES;
	CmicM_Inst.m_nTaskCnt=0;

}
static void Cmic_Active_Step4_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
	
		 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st5_REQ;
		 CmicM_ControlBootState();
		
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}

static void Cmic_Active_Step5_REQ(void)
{
	/****330. SetACL***************** */
	Cmic_RequestSetACL(realAcl.Data, realAcl.iCount);

	CmicM_Inst.m_nBOOT = 330;
	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st5_RES;
	CmicM_Inst.m_nTaskCnt=0;

}
static void Cmic_Active_Step5_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
	
		 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st6_REQ;
		 CmicM_ControlBootState();
		
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}


static void Cmic_Active_Step6_REQ(void)
{
	/****331. GetACL***************** */
	Cmic_RequestGetACL(&packInstance);
	
	CmicM_Inst.m_nBOOT = 331;
	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st6_RES;
	CmicM_Inst.m_nTaskCnt=0;

}

static void Cmic_Active_Step6_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		
		//Go to st1_REQ status because of (!bAllNodesJoined )
		 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st1_REQ; 
		 CmicM_ControlBootState();
		
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}


static void Cmic_Active_Step7_REQ(void)
{
    /* STEP 35 : Set Mode (STANDBY) **************************************************/
	adi_wil_mode_t currentMode;
	
   Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode )
	{
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY );	
		CmicM_Inst.m_nBOOT = 350;
	
		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st7_RES;
		CmicM_Inst.m_nTaskCnt=0;
	}else {
		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st8_REQ;
		CmicM_ControlBootState();
	}
}

static void Cmic_Active_Step7_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
			
			CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st7_REQ;//retry 
			CmicM_ControlBootState();
			return;
		}
		 CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st8_REQ;
		 CmicM_ControlBootState();
	
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}

static void Cmic_Active_Step8_REQ(void)
{
	/* STEP 36 : Set Mode (ACTIVE) *******************************************************/
   adk_debug_BootTimeLog(Interval, LogStart, 400, example_ExecuteSetMode_1______________);

   Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_ACTIVE);
   
   CmicM_Inst.m_nBOOT = 400;
   CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st8_RES;
	CmicM_Inst.m_nTaskCnt=0;

}

static void Cmic_Active_Step8_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
			
        	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st8_REQ;    //Retry    
			CmicM_ControlBootState();
        	return;
		}
		adk_debug_BootTimeLog(Interval, LogEnd__, 400, example_ExecuteSetMode_1______________);

		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st9_REQ;
		CmicM_ControlBootState();
	
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}

static void Cmic_Active_Step9_REQ(void)
{
	uint16  nNWBufferSize = 0;
	uint8   nTotalNodes=0;

	/* STEP 37 : Enable Network Data Capture *********************************************/
	  adk_debug_BootTimeLog(Interval, LogStart, 420, Demo_EnableNetworkDataCapture_________);

	  nNWBufferSize = CmicM_Inst.m_networkStatus.iCount * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL;
	  
	  adi_wil_EnableNetworkDataCapture(&packInstance, CmicM_Inst.m_networkDataBuffer,
	  	                                (CmicM_Inst.m_networkStatus.iCount * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL), true);

	  CmicM_Inst.m_nBOOT = 420;
	  
	  adk_debug_BootTimeLog(Interval, LogEnd__, 420, Demo_EnableNetworkDataCapture_________);

	  nTotalNodes= CmicM_Inst.m_networkStatus.iCount;	 /*  @remark : Variable to store total no. of Nodes in network */

	/* STEP 38 : Init CB_CELL for demo ***************************************************/
	  for(uint8 i=0; i<12; i++){	 /* ADBMS6833 */
		  for(uint8 j=0; j<18; j++){
			  CmicM_Inst.m_bCB_CELL[i][j] = true;
		  }
	  }
	  
	  CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st10_REQ;
	  CmicM_ControlBootState();
}

static void Cmic_Active_Step9_RES(void)
{
//NoAction.
}

static void Cmic_Active_Step10_REQ(void)
{

	CmicM_Inst.m_nBOOT = 999;

    adk_debug_BootTimeLog(Overall_, LogEnd__, 999, Demo_Total_boot_time__________________);
    CmicM_Inst.m_fBOOT_TIME = BOOT_TIME[0].DURATION;
	
  	Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
		         			ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID); // BASE

	CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st10_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Active_Step10_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
	
		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_stEND;
		CmicM_Inst.m_tSt.m_eMain = eMAIN_SENSING;
		CmicM_Inst.m_tSt.m_ePrevMain = eMAIN_SENSING;

		CmicM_Inst.m_tSt.m_eSubSensing = eSENSING_st0_IDLE;
		CmicM_Inst.m_tSt.m_eSubBalancing = eBALANCING_st0_IDLE;
			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}

static void Cmic_Sensing_Step0_IDLE(void)
{
	if ( eMAIN_SENSING != CmicM_Inst.m_tSt.m_ePrevMain ){
		CmicM_Inst.m_tSt.m_eSubSensing = eSENSING_st1_REQ;
		CmicM_ControlSensingState();
	}
}

static void Cmic_Sensing_Step1_REQ(void)
{
	Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
		         			ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID); // BASE

	CmicM_Inst.m_tSt.m_eSubSensing = eSENSING_st1_RES;
	CmicM_Inst.m_nTaskCnt=0;
}


static void Cmic_Sensing_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
	
		CmicM_Inst.m_tSt.m_eSubSensing = eSENSING_st0_IDLE;
		CmicM_Inst.m_tSt.m_ePrevMain = eMAIN_SENSING;
	
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}

static void Cmic_Balancing_Step0_IDLE(void)
{
	if ( CmicM_Inst.m_tSt.m_eMain != CmicM_Inst.m_tSt.m_ePrevMain ){
		CmicM_Inst.m_tSt.m_eSubBalancing = eBALANCING_st1_REQ;
		CmicM_ControlBalancingState();
	}
}

static void Cmic_Balancing_Step1_REQ(void)
{
	//Balancing 
	//Cmic_ExecuteModifyScript();
	
	CmicM_Inst.m_tSt.m_eSubBalancing = eBALANCING_st1_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Balancing_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
	
		CmicM_Inst.m_tSt.m_eSubBalancing = eBALANCING_st0_IDLE;
		CmicM_Inst.m_tSt.m_ePrevMain = CmicM_Inst.m_tSt.m_eMain;
	
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}




#ifndef _ADI_ONLY

void adi_wil_HandlePortCallback (adi_wil_port_t const * const pPort,
                                 adi_wil_api_t eAPI,
                                 adi_wil_err_t rc,
                                 void const * const pData)
{
        if ((rc == ADI_WIL_ERR_SUCCESS) && (pData != (void*)0)&& (CmicM_Inst.m_nPortCnt < 2))// && (pConfiguration != (void*)0))
        {
            memcpy(&CmicM_Inst.m_portConfig[CmicM_Inst.m_nPortCnt], pData, sizeof(adi_wil_configuration_t));
		
		}else {

			if (rc != ADI_WIL_ERR_SUCCESS){
				//fail count
				
			}
		}
		
}

void adi_wil_HandleCallback (adi_wil_pack_t const * const pPack,
                             void const * const pClientData,
                             adi_wil_api_t eAPI,
                             adi_wil_err_t rc,
                             void const * const pData)
{
  
	if (pData == NULL) {
		//	
	}else {
	
		switch(eAPI)
	 	{
	 		case ADI_WIL_API_LOAD_FILE:
				if (rc == ADI_WIL_ERR_IN_PROGRESS)
				{
				   (void)memcpy(&CmicM_Inst.m_clientData.LoadFileStatus, pData, sizeof(adi_wil_loadfile_status_t));
				}else if (rc == ADI_WIL_ERR_SUCCESS)
				{
				
				}
				break;
		 	case ADI_WIL_API_CONNECT :
				(void)memcpy(&CmicM_Inst.m_clientData.ConnectionDetails, pData, sizeof(adi_wil_connection_details_t)); 
				break;

			case ADI_WIL_API_GET_ACL :
        
    	   	    (void)memcpy(&CmicM_Inst.m_sysAcl, pData, sizeof(adi_wil_acl_t));
        		break;

			default : break;
		 }
	}		
		
	CmicM_Inst.m_notifyRC = rc;
	
}


 
void adi_wil_HandleEvent (adi_wil_pack_t const * const pPack,
                          void const * const pClientData,
                          adi_wil_event_id_t EventCode,
                          void const * const pData)
{
  
	  switch (EventCode)
	  {
		  case ADI_WIL_EVENT_COMM_MGR_CONNECTED:
			  CmicM_Inst.m_nMgrConnectCnt++;
			  break;
	
		  case ADI_WIL_EVENT_COMM_MGR_DISCONNECTED:
			  CmicM_Inst.m_nMgrDisConnectCnt++;
			  break;
  	     case ADI_WIL_EVENT_COMM_NODE_CONNECTED:
			  CmicM_Inst.m_nNodeConnectCnt++;
			  break;
		
		 case ADI_WIL_EVENT_DATA_READY_BMS:		
			 memcpy(CmicM_Inst.m_userBMSBuf, ((adi_wil_sensor_data_buffer_t *)pData)->pData, 
			  (((adi_wil_sensor_data_buffer_t *)pData)->iCount)*(sizeof(adi_wil_sensor_data_t)));
		
			 Cmic_ReadBMS();
			 
			 CmicM_Inst.m_nTotalPacketRcvd = ((adi_wil_sensor_data_buffer_t *)pData)->iCount;  /*  @remark Akash : Variable to store total no. of bms packets received */

			 Cmic_SaveLatencyPkt();
	
			 break;
			 
		  default:
		  	break;
	 }
}
#endif


bool Cmic_IsDualConfig(adi_wil_configuration_t * C1, adi_wil_configuration_t * C2)
{
	bool rc = false;
	uint8_t BlankMac[ADI_WIL_MAC_ADDR_SIZE] = {0};
	
	 if (C1->bDualConfiguration && C2->bDualConfiguration 
	 	&& (!memcmp(C1->PeerMAC, C2->MAC, ADI_WIL_MAC_ADDR_SIZE)) 
	 	&& (!memcmp(C2->PeerMAC, C1->MAC, ADI_WIL_MAC_ADDR_SIZE)) 
	 	&& (C1->iConfigurationHash == C2->iConfigurationHash))
	 {
		 rc = true;
	 }
	 if (!memcmp(C1->PeerMAC, BlankMac, ADI_WIL_MAC_ADDR_SIZE))
	 {
	
	 }
	 if (!memcmp(C2->PeerMAC, BlankMac, ADI_WIL_MAC_ADDR_SIZE))
	 {
	
	 }
	 return rc;

}

bool Cmic_CompareACL(void){

    bool bACL_rc = false; /* false = ACL update needed, true = no need to update */
	bool bACL_EMPTY = false;
	
	uint16 nCnt = 0; //nCompare_cnt
	uint16 nUCnt = 0; //UserCnt

	uint32 aUser_ACL[64] = {0,}; // @remark  max noode
	uint32 aRead_ACL[64] = {0,}; // @remark  max noode
	
	
    if(userAcl.iCount == CmicM_Inst.m_sysAcl.iCount){
        /* iCount is same */
        while(1){ /* Parse user MAC */
            if(userAcl.Data[nCnt * 8] == 0){
                /* @remark : There are no MAC exist in user setting */
                break;
            }
            else{
                /* @remark : Parse node specific MAC */
                aUser_ACL[nUCnt] |= userAcl.Data[nCnt * 8 + 5] << 16; //6th MAC
                aUser_ACL[nUCnt] |= userAcl.Data[nCnt * 8 + 6] << 8; //7th MAC
                aUser_ACL[nUCnt] |= userAcl.Data[nCnt * 8 + 7]; //8th MAC
                nUCnt ++;
            }
            nCnt++;
        }

        nCnt = 0; // counter re-init
        nUCnt = 0; // counter re-init

        while(1){ /* Parse read MAC */
            if(CmicM_Inst.m_sysAcl.Data[nCnt * 8] == 0){
                /* @remark : There are no MAC exist in get ACL setting */
                break;
            }
            else{
                /* @remark : Parse node specific MAC */
                aRead_ACL[nUCnt] |= CmicM_Inst.m_sysAcl.Data[nCnt * 8 + 5] << 16; //6th MAC
                aRead_ACL[nUCnt] |= CmicM_Inst.m_sysAcl.Data[nCnt * 8 + 6] << 8; //7th MAC
                aRead_ACL[nUCnt] |= CmicM_Inst.m_sysAcl.Data[nCnt * 8 + 7]; //8th MAC
                nUCnt ++;
            }
            nCnt++;
        }

        nCnt = 0; // counter re-init
        nUCnt = 0; // counter re-init

        /* @remark : Compare MAC */
        for(nCnt = 0; nCnt < 64; nCnt++){
            bACL_rc = false;
            for(nUCnt = 0; nUCnt < 64; nUCnt++){
                if(aUser_ACL[nCnt] == aRead_ACL[nUCnt]){
                    bACL_rc = true; /* found same MAC */
                    break;
                }
            }
            if(bACL_rc == false) break;
        }
    }
    else{
        /* iCount is different */
        bACL_rc = false;
        if (CmicM_Inst.m_sysAcl.iCount == 0) bACL_EMPTY = true; //Empty managers case
    }

    return bACL_rc;    
}

uint8 Cmic_ConvertDeviceId (adi_wil_device_t WilDeviceId)
{
    uint8_t DeviceId = 0;
    
    if(WilDeviceId != 0)
    {
        if 		(WilDeviceId == ADI_WIL_DEV_MANAGER_0) return 240;
        else if (WilDeviceId == ADI_WIL_DEV_MANAGER_1) return 241;
        else if (WilDeviceId == ADI_WIL_DEV_ALL_MANAGERS) return 254;
        else if (WilDeviceId == ADI_WIL_DEV_ALL_NODES) return 255;
    
        while ((WilDeviceId & (ADI_WIL_DEV_NODE_0 << DeviceId)) == 0)
        {
            if(DeviceId < 255) {
                DeviceId++;
            }
            else {
                DeviceId = 200;
                break;
            }
        }
    }
    else {
        DeviceId = 200;
    }
	
    return DeviceId;
}

void Cmic_ReadInitPacket(void)
{

    uint8_t cnt, packetId, eNode, i;
   	uint8_t aInitPacketRcv[48u];
	
    adi_bms_init_pkt_0_t*  pBMS_Pkt_0;
  
    memset(aInitPacketRcv, 0, sizeof(aInitPacketRcv));

    for(cnt = 0; cnt < CmicM_Inst.m_nTotalPacketRcvd ; cnt++)
	{
        if (CmicM_Inst.m_userBMSBuf[cnt].iLength == 0) 
			continue; // Adele: skip processing of empty packet
			
        eNode = ADK_ConvertDeviceId(CmicM_Inst.m_userBMSBuf[cnt].eDeviceId);

		packetId = CmicM_Inst.m_userBMSBuf[cnt].Data[0];
        
       
        if((packetId == ADI_BMS_INIT_PKT_0_ID) || (packetId == ADI_BMS_INIT_PKT_1_ID)
			|| (packetId == ADI_BMS_INIT_PKT_2_ID))       
        {
            aInitPacketRcv[eNode]++;

			pBMS_Pkt_0 = (adi_bms_init_pkt_0_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;

            if(pBMS_Pkt_0->Rdaca.iAc2v[1] == 0) 
				continue;
			
            CmicM_Inst.m_tempBuf[0]  = (signed short int)((pBMS_Pkt_0->Rdaca.iAc2v[1]   << 8) + pBMS_Pkt_0->Rdaca.iAc2v[0]);
            CmicM_Inst.m_tempBuf[1]  = (signed short int)((pBMS_Pkt_0->Rdaca.iAc3v[1]   << 8) + pBMS_Pkt_0->Rdaca.iAc3v[0]);
            CmicM_Inst.m_tempBuf[2]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc4v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc4v[0]);
            CmicM_Inst.m_tempBuf[3]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc5v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc5v[0]);
            CmicM_Inst.m_tempBuf[4]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc6v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc6v[0]);
            CmicM_Inst.m_tempBuf[5]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc7v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc7v[0]);
            CmicM_Inst.m_tempBuf[6]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc8v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc8v[0]);
            CmicM_Inst.m_tempBuf[7]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc9v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc9v[0]);
            CmicM_Inst.m_tempBuf[8]  = (signed short int)((pBMS_Pkt_0->Rdacd.iAc11v[1]  << 8) + pBMS_Pkt_0->Rdacd.iAc11v[0]);
            CmicM_Inst.m_tempBuf[9]  = (signed short int)((pBMS_Pkt_0->Rdacd.iAc12v[1]  << 8) + pBMS_Pkt_0->Rdacd.iAc12v[0]);
            CmicM_Inst.m_tempBuf[10] = (signed short int)((pBMS_Pkt_0->Rdace.iAc13v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc13v[0]);
            CmicM_Inst.m_tempBuf[11] = (signed short int)((pBMS_Pkt_0->Rdace.iAc14v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc14v[0]);
            CmicM_Inst.m_tempBuf[12] = (signed short int)((pBMS_Pkt_0->Rdace.iAc15v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc15v[0]);
            CmicM_Inst.m_tempBuf[13] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc16v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc16v[0]);
            CmicM_Inst.m_tempBuf[14] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc17v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc17v[0]);
            CmicM_Inst.m_tempBuf[15] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc18v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc18v[0]);
            CmicM_Inst.m_tempBuf[16] = (signed short int)((pBMS_Pkt_0->Rdauxa.iG1v[1]   << 8) + pBMS_Pkt_0->Rdauxa.iG1v[0]);
            CmicM_Inst.m_tempBuf[17] = (signed short int)((pBMS_Pkt_0->Rdauxa.iG2v[1]   << 8) + pBMS_Pkt_0->Rdauxa.iG2v[0]);
            CmicM_Inst.m_tempBuf[18] = (signed short int)((pBMS_Pkt_0->Rdauxa.iGa11v[1] << 8) + pBMS_Pkt_0->Rdauxa.iGa11v[0]);
            CmicM_Inst.m_tempBuf[19] = (signed short int)((pBMS_Pkt_0->Rdauxe.iG3v[1]   << 8) + pBMS_Pkt_0->Rdauxe.iG3v[0]);

			for( i = 0; i < 16; i++)
			{
                /* @remark : Cell voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].CELL_V[i] = (float)(CmicM_Inst.m_tempBuf[i] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : Cell voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].CELL_Vi[i] = CmicM_Inst.m_tempBuf[i] + 10000;
            }

            for( i = 0; i < 4; i++)
			{
                /* @remark : AUX voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].TEMP_V[i] = (float)(CmicM_Inst.m_tempBuf[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : AUX voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].TEMP_Vi[i] = CmicM_Inst.m_tempBuf[i + 16] + 10000;  
            }

        }
    }
}

void Cmic_ReadBMS(void)
{
    uint8_t cnt, packetId, eNode, i;
	 
	
    uint8_t temp_recv_confirm[ADK_MAX_node] = {0,};
  

    for(cnt = 0; cnt < CmicM_Inst.m_nTotalPacketRcvd; cnt++){
		
        if (CmicM_Inst.m_userBMSBuf[cnt].iLength == 0) continue; // Adele: skip processing of empty packet

        eNode = Cmic_ConvertDeviceId(CmicM_Inst.m_userBMSBuf[cnt].eDeviceId);

		packetId = CmicM_Inst.m_userBMSBuf[cnt].Data[0];
                
        if( packetId == ADI_BMS_BASE_PKT_0_ID )
		{
            temp_recv_confirm[eNode] |= 0x01;

			CmicM_Inst.pBMS_Pkt_0 = (adi_bms_base_pkt_0_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;
            

            /* @remark : Read raw data (float) */
            CmicM_Inst.m_tempBuf[0]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdaca.iAc2v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdaca.iAc2v[0]);
            CmicM_Inst.m_tempBuf[1]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdaca.iAc3v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdaca.iAc3v[0]);
            CmicM_Inst.m_tempBuf[2]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc4v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc4v[0]);
            CmicM_Inst.m_tempBuf[3]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc5v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc5v[0]);
            CmicM_Inst.m_tempBuf[4]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc6v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacb.iAc6v[0]);
            CmicM_Inst.m_tempBuf[5]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc7v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc7v[0]);
            CmicM_Inst.m_tempBuf[6]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc8v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc8v[0]);
            CmicM_Inst.m_tempBuf[7]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc9v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacc.iAc9v[0]);
            CmicM_Inst.m_tempBuf[8]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacd.iAc11v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacd.iAc11v[0]);
            CmicM_Inst.m_tempBuf[9]  = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacd.iAc12v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacd.iAc12v[0]);
            CmicM_Inst.m_tempBuf[10] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdace.iAc13v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdace.iAc13v[0]);
            CmicM_Inst.m_tempBuf[11] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdace.iAc14v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdace.iAc14v[0]);
            CmicM_Inst.m_tempBuf[12] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdace.iAc15v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdace.iAc15v[0]);
            CmicM_Inst.m_tempBuf[13] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc16v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc16v[0]);
            CmicM_Inst.m_tempBuf[14] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc17v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc17v[0]);
            CmicM_Inst.m_tempBuf[15] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc18v[1]  << 8) + CmicM_Inst.pBMS_Pkt_0->Rdacf.iAc18v[0]);
            CmicM_Inst.m_tempBuf[16] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdauxa.iG1v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdauxa.iG1v[0]);
            CmicM_Inst.m_tempBuf[17] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdauxa.iG2v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdauxa.iG2v[0]);
            CmicM_Inst.m_tempBuf[18] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdauxa.iGa11v[1] << 8) + CmicM_Inst.pBMS_Pkt_0->Rdauxa.iGa11v[0]);
            CmicM_Inst.m_tempBuf[19] = (signed short int)((CmicM_Inst.pBMS_Pkt_0->Rdauxe.iG3v[1]   << 8) + CmicM_Inst.pBMS_Pkt_0->Rdauxe.iG3v[0]);

                  
            for( i = 0; i < 16; i++){
                /* @remark : Cell voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].CELL_V[i] = (float)(CmicM_Inst.m_tempBuf[i]  * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : Cell voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].CELL_Vi[i] = CmicM_Inst.m_tempBuf[i] + 10000;
            }

            for(i = 0; i < 4; i++){
                /* @remark : AUX voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].TEMP_V[i]  = (CmicM_Inst.m_tempBuf[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;

                /* @remark : AUX voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].TEMP_Vi[i]  = CmicM_Inst.m_tempBuf[i + 16] + 10000;  
            }
  
            Adbms683x_Monitor_Base_Pkt0(&CmicM_Inst.m_userBMSBuf[cnt]);  
			
        }
        else if (packetId == ADI_BMS_BASE_PKT_1_ID)
		{
            temp_recv_confirm[eNode] |= 0x02;
            CmicM_Inst.pBMS_Pkt_1 = (adi_bms_base_pkt_1_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;

            CmicM_Inst.m_NODE[eNode].CB_STAT = ((CmicM_Inst.pBMS_Pkt_1->Rdcfga.iCfgar2 & BMS_CELLS_1_TO_2_MASK) << 16) + (CmicM_Inst.pBMS_Pkt_1->Rdcfgb.iCfgbr5 << 8) + CmicM_Inst.pBMS_Pkt_1->Rdcfgb.iCfgbr4;

			Adbms683x_Monitor_Base_Pkt1(&CmicM_Inst.m_userBMSBuf[cnt]);       
        }
        else if (packetId == ADI_BMS_BASE_PKT_2_ID)
		{
            temp_recv_confirm[eNode] |= 0x04;
            CmicM_Inst.pBMS_Pkt_2 = (adi_bms_base_pkt_2_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;

            CmicM_Inst.m_NODE[eNode].OWD_CS_STAT = ((CmicM_Inst.pBMS_Pkt_2->Rdstatc.iStcr2 & 0xC0) << 10) + (CmicM_Inst.pBMS_Pkt_2->Rdstatc.iStcr1 << 8) + CmicM_Inst.pBMS_Pkt_2->Rdstatc.iStcr0;
           
        }
        else{
           
        }

        if(iOWDPcktsRcvd[eNode] == SM_ADBMS6833_ALL_PCKTS_FOR_OWD_RCVD)
        {
            Adbms683x_Monitor_Cell_OWD(eNode);
            iOWDPcktsRcvd[eNode] = 0U;
        }
    }
	
}

adi_bms_latent0_pkt_0_t Latency0_Pkt_0;
adi_bms_latent0_pkt_1_t Latency0_Pkt_1;
adi_bms_latent0_pkt_2_t Latency0_Pkt_2;
adi_bms_latent1_pkt_0_t Latency1_Pkt_0;
adi_bms_latent1_pkt_1_t Latency1_Pkt_1;

void Cmic_SaveLatencyPkt(void)
{
	
	uint8	  eNode, count; 

	 /* @remark  : check BASE packet header */
	 for(count = 0; count < CmicM_Inst.m_nTotalPacketRcvd; count++){

		 eNode = Cmic_ConvertDeviceId(CmicM_Inst.m_userBMSBuf[count].eDeviceId);
		 
		 if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_0_ID)
		 {
			 memcpy(&Latency0_Pkt_0, (adi_bms_latent0_pkt_0_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_0.Rdstatc, (adi_bms_latent0_pkt_0_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_0_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_1_ID)
		 {
			 memcpy(&Latency0_Pkt_1, (adi_bms_latent0_pkt_1_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_1.Rdpwma_2, (adi_bms_latent0_pkt_1_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_1_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_2_ID)
		 {					
			 memcpy(&Latency0_Pkt_2, (adi_bms_latent0_pkt_2_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_2.Rdace, (adi_bms_latent0_pkt_2_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_2_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT1_PKT_0_ID)
		 {					 
			 memcpy(&Latency1_Pkt_0, (adi_bms_latent1_pkt_0_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency1_Pkt_0.Rdaca, (adi_bms_latent1_pkt_0_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_0_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT1_PKT_1_ID)
		 {					 
			 memcpy(&Latency1_Pkt_1, (adi_bms_latent1_pkt_1_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency1_Pkt_1.Rdsvd, (adi_bms_latent1_pkt_1_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_1_t) - 8);
		 }

	 }
	 				

}

void Cmic_ExecuteModifyScript(void)
{
	
}

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
	MAIN_STATE_E		m_eMain;
	MAIN_STATE_E		m_ePrevMain;  //Previous MainState
	SENSING_STATE_E		m_eSensing;	//Sensing SubState
	BALANCING_STATE_E  	m_eBalancing; //Balancing SubState
	KEY_ON_STATE_E		m_eKeyOn;
	KEY_OFF_STATE_E		m_eKeyOff;

    BOOT_STATE_E    	m_eBoot;
    BOOTSUB_INIT_E      m_eSubInit;
    BOOTSUB_CONNECT_E   m_eSubConnect;
	BOOTSUB_JOIN_E		m_eSubJoin;
	BOOTSUB_LOAD_E      m_eSubLoad;
	BOOTSUB_ACTIVE_E	m_eSubActive;

}CmicM_State_t;

typedef struct
{
	
    uint16 					m_nTaskCnt;
	uint32  				m_nTick1ms;
	uint16   				m_nBOOT;
	float					m_fBOOT_TIME;
	CmicM_State_t			m_tSt;
	adi_wil_configuration_t m_portConfig[2];
	adi_wil_sensor_data_t   m_wbmsSysSensorData[BMS_DATA_PACKET_COUNT];
	adi_wil_sensor_data_t	m_userBMSBuf[BMS_DATA_PACKET_COUNT];
	sint16 					m_tempBuf[22];
	adi_wil_file_type_t     m_eFileType;
	uint64_t 				m_DeviceType;
	ADK_LOG_FUNCTION    	m_LoadfileStep;

	adi_wil_acl_t			m_sysAcl;
	adi_wil_network_status_t m_networkStatus;
	adi_wil_network_data_t   m_networkDataBuffer[ADI_WIL_MAX_NODES*ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL];
	adi_wil_app_cell_balance_change_t 	m_tScriptChange;
	adi_wil_err_t			m_notifyRC;
    NODESTR 				m_NODE;

    bool 					m_bCB_CELL[12][ADK_MAX_cell];
    bool 					m_bCB_NODE[12];
	uint32					m_iDecTemp[12];
	uint32 					m_iDecFinal[12];
	uint8  					m_nNodeCount; //ModifyScript에서 순차적으로 스크립트 실행을 위해 저장할 현재 노드카운트값.
	
	uint8_t *					m_pOtapImage;
    uint32_t 					m_iOtapImageLen;
    uint32_t 					m_iImageLength;
	client_data_t 			m_clientData;

	uint8           		m_nPortCnt;
	uint8					m_nMgrConnectCnt;
	uint8					m_nMgrDisConnectCnt;
	uint8					m_nNodeConnectCnt;

	bool					m_bAclUpdateNoNeed;
	bool					m_bAclUpdateForce;
	bool					m_bAllNodesJoined;
	bool 					m_ACL_EMPTY;
	
	uint16					m_nTotalPacketRcvd;	   /*  @remark : Variable to store total no. of bms packets received */
	uint32 					m_nBMSNotifyCnt;  //For debug..HandleEvent-BMS 통지카운트
	uint32					m_nLatentNotifyCnt;
	bool 					m_bLatent0_Notify; //Latent0 packet all 수신시=true
	bool 					m_bLatent1_Notify; //Latent1 packet all 수신시=true

    adi_bms_base_pkt_0_t* 	m_pBMS_Pkt_0; //For debug..
    adi_bms_base_pkt_1_t* 	m_pBMS_Pkt_1;
    adi_bms_base_pkt_2_t* 	m_pBMS_Pkt_2;

} CmicM_Instance_t;

static CmicM_Instance_t CmicM_Inst;
static void CmicM_ControlBootState(void);
static void CmicM_ControlSensingState(void);
static void CmicM_ControlBalancingState(void);
static void CmicM_ControlKeyOnState(void);
static void CmicM_ControlKeyOffState(void);

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
static void Cmic_Join_Step2_REQ(void);
static void Cmic_Join_Step2_RES(void);
static void Cmic_Join_Step3_REQ(void);
static void Cmic_Join_Step3_RES(void);
static void Cmic_Join_Step4_REQ(void);
static void Cmic_Join_Step4_RES(void);
static void Cmic_Join_Step5_REQ(void);
static void Cmic_Join_Step5_RES(void);
static void Cmic_Join_Step6_REQ(void);
static void Cmic_Join_Step6_RES(void);
static void Cmic_Join_Step7_REQ(void);
static void Cmic_Join_Step7_RES(void);
static void Cmic_Join_Step8_REQ(void);
static void Cmic_Join_Step8_RES(void);

static void Cmic_Load_Step1_REQ(void);
static void Cmic_Load_Step1_RES(void);
static void Cmic_Load_Step2_REQ(void);
static void Cmic_Load_Step2_RES(void);
static void Cmic_Load_Step3_REQ(void);
static void Cmic_Load_Step3_RES(void);
static void Cmic_Load_Step4_REQ(void);
static void Cmic_Load_Step4_RES(void);
static void Cmic_Load_Step5_REQ(void);
static void Cmic_Load_Step5_RES(void);
static void Cmic_Load_Step6_REQ(void);
static void Cmic_Load_Step6_RES(void);
static void Cmic_Load_Step7_REQ(void);
static void Cmic_Load_Step7_RES(void);
static void Cmic_Load_Step8_REQ(void);
static void Cmic_Load_Step8_RES(void);
static void Cmic_Load_Step9_REQ(void);
static void Cmic_Load_Step9_RES(void);


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
static void	Cmic_Sensing_Step2_REQ(void);
static void	Cmic_Sensing_Step2_RES(void);

static void	Cmic_Balancing_Step0_IDLE(void);
static void	Cmic_Balancing_Step1_REQ(void);
static void	Cmic_Balancing_Step1_RES(void);
static void	Cmic_Balancing_Step2_REQ(void);
static void	Cmic_Balancing_Step2_RES(void);

static void	Cmic_KeyOn_Step1_REQ(void);
static void	Cmic_KeyOn_Step1_RES(void);
static void	Cmic_KeyOn_Step2_REQ(void);
static void	Cmic_KeyOn_Step2_RES(void);
static void Cmic_KeyOn_Step3_REQ(void);
static void Cmic_KeyOn_Step3_RES(void);
static void Cmic_KeyOn_Step4_REQ(void);
static void Cmic_KeyOn_Step4_RES(void);
static void Cmic_KeyOn_Step5_REQ(void);
static void Cmic_KeyOn_Step5_RES(void);
static void Cmic_KeyOn_Step6_REQ(void);
static void Cmic_KeyOn_Step6_RES(void);


static void	Cmic_KeyOff_Step0_IDLE(void);
static void	Cmic_KeyOff_Step1_REQ(void);
static void	Cmic_KeyOff_Step1_RES(void);
static void	Cmic_KeyOff_Step2_REQ(void);
static void	Cmic_KeyOff_Step2_RES(void);
static void	Cmic_KeyOff_Step3_REQ(void);
static void	Cmic_KeyOff_Step3_RES(void);
static void	Cmic_KeyOff_Step4_REQ(void);
static void	Cmic_KeyOff_Step4_RES(void);
static void	Cmic_KeyOff_Step5_REQ(void);
static void	Cmic_KeyOff_Step5_RES(void);

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
	Cmic_Join_Step2_REQ, //eJOIN_st2_REQ
	Cmic_Join_Step2_RES, //eJOIN_st2_RES
	Cmic_Join_Step3_REQ, //eJOIN_st3_REQ
	Cmic_Join_Step3_RES, //eJOIN_st3_RES
	Cmic_Join_Step4_REQ, //eJOIN_st4_REQ
	Cmic_Join_Step4_RES, //eJOIN_st4_RES
	Cmic_Join_Step5_REQ, //eJOIN_st5_REQ
	Cmic_Join_Step5_RES, //eJOIN_st5_RES
	Cmic_Join_Step6_REQ, //eJOIN_st6_REQ
	Cmic_Join_Step6_RES, //eJOIN_st6_RES
	Cmic_Join_Step7_REQ, //eJOIN_st7_REQ
	Cmic_Join_Step7_RES, //eJOIN_st7_RES
	Cmic_Join_Step8_REQ, //eJOIN_st8_REQ
	Cmic_Join_Step8_RES, //eJOIN_st8_RES
};

static const pFunc  pArrayLoad[]=
{
	Cmic_Load_Step1_REQ, //eJOIN_st1_REQ
	Cmic_Load_Step1_RES, //eJOIN_st1_RES
	Cmic_Load_Step2_REQ, //eJOIN_st2_REQ
	Cmic_Load_Step2_RES, //eJOIN_st2_RES
	Cmic_Load_Step3_REQ, //eJOIN_st3_REQ
	Cmic_Load_Step3_RES, //eJOIN_st3_RES
	Cmic_Load_Step4_REQ, //eJOIN_st4_REQ
	Cmic_Load_Step4_RES, //eJOIN_st4_RES
	Cmic_Load_Step5_REQ, //eJOIN_st5_REQ
	Cmic_Load_Step5_RES, //eJOIN_st5_RES
	Cmic_Load_Step6_REQ, //eJOIN_st6_REQ
	Cmic_Load_Step6_RES, //eJOIN_st6_RES
	Cmic_Load_Step7_REQ, //eJOIN_st7_REQ
	Cmic_Load_Step7_RES, //eJOIN_st7_RES
	Cmic_Load_Step8_REQ, //eJOIN_st8_REQ
	Cmic_Load_Step8_RES, //eJOIN_st8_RES
	Cmic_Load_Step9_REQ, //eJOIN_st8_REQ
	Cmic_Load_Step9_RES, //eJOIN_st8_RES
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
	Cmic_Sensing_Step2_REQ,
	Cmic_Sensing_Step2_RES,	
};

static const pFunc pArrayBalancing[]={
	Cmic_Balancing_Step0_IDLE,
	Cmic_Balancing_Step1_REQ,
	Cmic_Balancing_Step1_RES,
	Cmic_Balancing_Step2_REQ,
	Cmic_Balancing_Step2_RES,
};

static const pFunc pArrayKeyOn[]={
	
	Cmic_KeyOn_Step1_REQ,
	Cmic_KeyOn_Step1_RES,
	Cmic_KeyOn_Step2_REQ,
	Cmic_KeyOn_Step2_RES,
	Cmic_KeyOn_Step3_REQ,
	Cmic_KeyOn_Step3_RES,
	Cmic_KeyOn_Step4_REQ,
	Cmic_KeyOn_Step4_RES,
	Cmic_KeyOn_Step5_REQ,
	Cmic_KeyOn_Step5_RES,
	Cmic_KeyOn_Step6_REQ,
	Cmic_KeyOn_Step6_RES,	
};

static const pFunc pArrayKeyOff[]={
	Cmic_KeyOff_Step0_IDLE,
	Cmic_KeyOff_Step1_REQ,
	Cmic_KeyOff_Step1_RES,
	Cmic_KeyOff_Step2_REQ,
	Cmic_KeyOff_Step2_RES,
	Cmic_KeyOff_Step3_REQ,
	Cmic_KeyOff_Step3_RES,
	Cmic_KeyOff_Step4_REQ,
	Cmic_KeyOff_Step4_RES,
	Cmic_KeyOff_Step5_REQ,
	Cmic_KeyOff_Step5_RES,	
};


extern void Adbms683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Base_Pkt2(adi_wil_sensor_data_t* BMSBufferPtr);
extern void Adbms683x_Monitor_Cell_OWD(adi_wil_device_t eNode);
extern void adk_debug_BootTimeLog(bool final, bool start, uint16_t step, ADK_LOG_FUNCTION api);
extern boolean IsReleaseWilAPI(void const * const pPack);
extern adi_wil_pack_t packInstance;
extern adi_wil_acl_t  realAcl;
extern BOOTTIMESTR 		BOOT_TIME;
extern volatile adi_wil_err_t adi_gProcessTaskErrorCode;

static uint8_t  aiOWStatus[ADI_WIL_MAX_NODES]; //Array to store all Open Wire status for each Node in the network
static int16_t  aiOWCellData[ADI_WIL_MAX_NODES][SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE]; //Array to store Open Wire cell data 
static int16_t  aiNominalCellData[ADI_WIL_MAX_NODES][SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE];//Array to store nominal cell data
static uint8_t  iOWDPcktsRcvd[ADI_WIL_MAX_NODES];// Status flag to indicate if all the packets for Open Wire detection are received

boolean  powerOn=TRUE;

void CmicM_Init(void)
{
	if (powerOn == FALSE ){
		CmicM_IG_Init();
		return;
	}

	memset(&CmicM_Inst, 0, sizeof(CmicM_Instance_t));

	CmicM_Inst.m_nBOOT = 1;
	
	adk_debug_TickerBTInit();

	CmicM_Inst.m_nTick1ms = (uint32)GetTick_1ms();

	CmicM_Inst.m_tSt.m_eMain = eMAIN_BOOT;
	CmicM_Inst.m_tSt.m_eBoot = eBOOT_INIT;
	CmicM_Inst.m_tSt.m_eSubInit = eINIT_st1_REQ;
	CmicM_Inst.m_LoadfileStep = Demo_ExecuteLoadFile_config_allnode___;

	CmicM_Inst.m_eFileType = ADI_WIL_FILE_TYPE_CONFIGURATION;
	CmicM_Inst.m_DeviceType = ADI_WIL_DEV_ALL_NODES;

   adk_debug_BootTimeLog(Overall_, LogStart, 999, Demo_Total_boot_time__________________);
 
}

void CmicM_IG_Init(void)
{

	memset(&CmicM_Inst, 0, sizeof(CmicM_Instance_t));

	CmicM_Inst.m_nBOOT = 700;
	
	adk_debug_TickerBTInit();

	CmicM_Inst.m_nTick1ms = (uint32)GetTick_1ms();

	CmicM_Inst.m_tSt.m_eMain = eMAIN_KEY_ON_EVENT;
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st1_REQ;
	
    adk_debug_BootTimeLog(Overall_, LogStart, 730, Demo_key_on_event_____________________);
     
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
			case eMAIN_KEY_ON_EVENT :
				CmicM_ControlKeyOnState();
				break;
			case eMAIN_KEY_OFF_EVENT :
				CmicM_ControlKeyOffState();
				break;	
			default :
				break;
		}
		
		
	}
}

static void CmicM_ControlSensingState(void)
{
	if ( pArraySensing[CmicM_Inst.m_tSt.m_eSensing] != 0){
		pArraySensing[CmicM_Inst.m_tSt.m_eSensing]();
	}else {
		//error
	}
}

static void CmicM_ControlBalancingState(void)
{
	if ( pArrayBalancing[CmicM_Inst.m_tSt.m_eBalancing] != 0){
		pArrayBalancing[CmicM_Inst.m_tSt.m_eBalancing]();
	}else {
		//error
	}
}

static void CmicM_ControlKeyOnState(void)
{
	if ( pArrayKeyOn[CmicM_Inst.m_tSt.m_eKeyOn] != 0){
		pArrayKeyOn[CmicM_Inst.m_tSt.m_eKeyOn]();
	}else {
		//error
	}
}

static void CmicM_ControlKeyOffState(void)
{
	if ( pArrayKeyOff[CmicM_Inst.m_tSt.m_eKeyOff] != 0){
		pArrayKeyOff[CmicM_Inst.m_tSt.m_eKeyOff]();
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
		case eBOOT_LOAD :
			if ( pArrayLoad[CmicM_Inst.m_tSt.m_eSubLoad] != 0){
				pArrayLoad[CmicM_Inst.m_tSt.m_eSubLoad]();
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
				adk_debug_BootTimeLog(Interval, LogEnd__, 110, Demo_isDualConfig_____________________);
				CmicM_ControlBootState();

			}else
			{
			#ifdef _SURE_DBG
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st1_REQ; //case of not dual config 
			#else
				CmicM_Inst.m_tSt.m_eSubConnect = eCONNECT_st6_REQ;
			#endif
				adk_debug_BootTimeLog(Interval, LogEnd__, 110, Demo_isDualConfig_____________________);
				CmicM_ControlBootState();
			}
			
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
			 CmicM_ControlBootState();
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

	errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS);

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
				
				errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS);
				break;
				
			case  ADI_WIL_ERR_IN_PROGRESS :	
				
				errorCode = Cmic_RequestLoadFileConfig(ADI_WIL_FILE_TYPE_CONFIGURATION, ADI_WIL_DEV_ALL_MANAGERS);
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

adi_wil_err_t Cmic_RequestLoadFileConfig(adi_wil_file_type_t eFileType, adi_wil_device_t eDevice)
{

    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
	if (eFileType != ADI_WIL_FILE_TYPE_BMS_CONTAINER){
		
		CmicM_Inst.m_pOtapImage = (uint8_t *)&configuration_file_configuration;
        CmicM_Inst.m_iOtapImageLen =  configuration_file_configuration_length;

		switch (eDevice)
		{
		case ADI_WIL_DEV_ALL_MANAGERS:
			errorCode = adi_wil_LoadFile(&packInstance, eDevice, eFileType,
					&CmicM_Inst.m_pOtapImage[CmicM_Inst.m_clientData.LoadFileStatus.iOffset]);
			break;
		case ADI_WIL_DEV_ALL_NODES:
			errorCode = adi_wil_LoadFile(&packInstance, eDevice, eFileType,
					&CmicM_Inst.m_pOtapImage[CmicM_Inst.m_clientData.LoadFileStatus.iOffset]);
			break;
		
		default:
			break;
		}
	}
	else{
		adi_bms_GetContainerPtr(&CmicM_Inst.m_pOtapImage, &CmicM_Inst.m_iOtapImageLen);

		errorCode = adi_wil_LoadFile(&packInstance, eDevice, eFileType,
				&CmicM_Inst.m_pOtapImage[CmicM_Inst.m_clientData.LoadFileStatus.iOffset]);
	}
	
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
	/* CONDITION 3 : Check ACL update ****************************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 200, Demo_ACL_UPDATE_FORCE_________________);
	CmicM_Inst.m_nBOOT = 200;

	if(CmicM_Inst.m_ACL_EMPTY == false){
		adk_debug_BootTimeLog(Interval, LogStart, 209, Demo_ExecuteResetDevice_______________);
		Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES);

		CmicM_Inst.m_nBOOT = 209;

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st1_RES;
		CmicM_Inst.m_nTaskCnt=0;
	}
	else{
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st2_REQ;
		CmicM_ControlBootState();
	}
}

static void Cmic_Join_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

	    adk_debug_BootTimeLog(Interval, LogEnd__, 209, Demo_ExecuteResetDevice_______________);

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st2_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	}
}


static void Cmic_Join_Step2_REQ(void)
{
	adk_debug_BootTimeLog(Interval, LogStart, 210, Demo_ExecuteSetACL____________________);
	/* STEP 14 : SetAcl **********************************************************/
	if (realAcl.iCount == 0){
		Cmic_RequestSetACL(userAcl.Data, userAcl.iCount);

		CmicM_Inst.m_nBOOT = 210;

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st2_RES;
		CmicM_Inst.m_nTaskCnt=0;
	}
	else{
		/****216. SetAcl from realAcl ***************** */
		CmicM_Inst.m_nBOOT = 218;

		adi_wil_mode_t currentMode;
		
		Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
		if ( ADI_WIL_MODE_STANDBY != currentMode )
		{
			Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY );
			CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st2_RES;	
		}else { //STANDBY 
		
			CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st4_RES;
		}
		CmicM_Inst.m_nTaskCnt=0;  
	}

}

static void Cmic_Join_Step2_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 210, Demo_ExecuteSetACL____________________);

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st3_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Join_Step3_REQ(void)
{
	adk_debug_BootTimeLog(Interval, LogStart, 211, Demo_ExecuteGetACL_1__________________);
	Cmic_RequestGetACL(&packInstance);

	CmicM_Inst.m_nBOOT = 211;

	CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st3_RES;
	CmicM_Inst.m_nTaskCnt=0;
}


static void Cmic_Join_Step3_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 211, Demo_ExecuteGetACL_1__________________);

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st4_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Join_Step4_REQ(void)
{
	/* STEP 15 : Set Mode (COMMISSIONING) ****************************************/
	adi_wil_mode_t currentMode;

    Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode )
	{
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st5_REQ;
		CmicM_ControlBootState();	
		
	}else { //STANDBY -> COMMISSIONING 
	
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_COMMISSIONING );
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st4_RES;
	}
	CmicM_Inst.m_nTaskCnt=0;   
}


static void Cmic_Join_Step4_RES(void)
{
	adi_wil_mode_t currentMode;

	if (IsReleaseWilAPI(&packInstance)){

		Cmic_ExecuteGetMode(&packInstance, &currentMode);

		switch(currentMode) //STANDBY -> COMMISSIONING 
		{
			case  ADI_WIL_MODE_COMMISSIONING :
				CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st5_REQ;
				CmicM_ControlBootState();
				break;
			case  ADI_WIL_MODE_STANDBY :
				CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st7_REQ;
				CmicM_ControlBootState();
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

static void Cmic_Join_Step5_REQ(void)
{
	/* STEP 16 : Get Network Status **********************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 212, Demo_ExecuteGetNetworkStatus_0________);
    CmicM_Inst.m_bAllNodesJoined = Cmic_RequestGetNetworkStatus(&packInstance, &CmicM_Inst.m_networkStatus);

	CmicM_Inst.m_nBOOT = 212;

	CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st5_RES;
}

static void Cmic_Join_Step5_RES(void)
{
	adk_debug_BootTimeLog(Interval, LogEnd__, 212, Demo_ExecuteGetNetworkStatus_0________);
	if ( !CmicM_Inst.m_bAllNodesJoined )
	{
		/* CONDITION 4 : Check Node response *****************************************/
		adk_debug_BootTimeLog(Interval, LogStart, 215, Demo_bAllNodesJoined__________________);
		CmicM_Inst.m_nBOOT = 215;
		memset(&realAcl, 0, sizeof(adi_wil_acl_t));
		uint8_t l = 0;
		for(uint8 k = 0; k < CmicM_Inst.m_networkStatus.iCount; k++) 
		{
			if(CmicM_Inst.m_networkStatus.iConnectState & (1ULL << k ))
			{
				/* STEP 17 : realAcl generation from userAcl *********************/
				memcpy(realAcl.Data + (l * ADI_WIL_MAC_ADDR_SIZE), userAcl.Data + (k * ADI_WIL_MAC_ADDR_SIZE), ADI_WIL_MAC_ADDR_SIZE);
	   
				CmicM_Inst.m_nBOOT = 216;
				realAcl.iCount++;
				l++;
			}
		}
	 	CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st2_REQ;
	}
	else {
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st6_REQ;
	}
	CmicM_ControlBootState();
}

static void Cmic_Join_Step6_REQ(void)
{
	adk_debug_BootTimeLog(Interval, LogEnd__, 215, Demo_bAllNodesJoined__________________);
	/* CONDITION 5 : Check First trial for realAcl ***********************************/
	adk_debug_BootTimeLog(Interval, LogStart, 218, Demo_realAcl_generation_______________);

	if (realAcl.iCount == 0){
		/* STEP 18 : realAcl generation from userAcl *********************************/
		memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));

		CmicM_Inst.m_nBOOT = 218;
		adk_debug_BootTimeLog(Interval, LogEnd__, 218, Demo_realAcl_generation_______________);
	}
	/* STEP 19 : Set Mode (STANDBY) **************************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 220, Demo_ExecuteSetMode_STANDBY___________);
	adi_wil_mode_t currentMode;

    Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode )
	{
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY );	

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st6_RES;

	}else { //STANDBY

		CmicM_Inst.m_tSt.m_eBoot = eBOOT_LOAD;
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_stEND;
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st1_REQ;
		CmicM_ControlBootState();	
	}
	CmicM_Inst.m_nTaskCnt=0;   

	CmicM_Inst.m_nBOOT = 220;
}

static void Cmic_Join_Step6_RES(void)
{
	adk_debug_BootTimeLog(Interval, LogEnd__, 220, Demo_ExecuteSetMode_STANDBY___________);
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eBoot = eBOOT_LOAD;
		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_stEND;
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st1_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Join_Step7_REQ(void)
{
	/****209. should call ResetDevice ***************** */
	Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES);

	CmicM_Inst.m_nBOOT = 209;

	CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st7_RES;
	CmicM_Inst.m_nTaskCnt=0;

}

static void Cmic_Join_Step7_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st8_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Join_Step8_REQ(void)
{
	/****209. should call ResetDevice ***************** */
	Cmic_RequestSetACL(userAcl.Data, userAcl.iCount);

	CmicM_Inst.m_nBOOT = 210;

	CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st8_RES;
	CmicM_Inst.m_nTaskCnt=0;

}

static void Cmic_Join_Step8_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eSubJoin = eJOIN_st3_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Load_Step1_REQ(void)
{
	adi_wil_mode_t   currentMode;

	Cmic_ExecuteGetMode(&packInstance, &currentMode);

	if ( ADI_WIL_MODE_STANDBY != currentMode)
	{		 
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
	
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st1_RES;
		CmicM_Inst.m_nTaskCnt=0;
	}
	else { //currentMode == ADI_WIL_MODE_STANDBY
	 
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st2_REQ;
		CmicM_ControlBootState();
	}	
}

static void Cmic_Load_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		
		if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
		 	
			CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st1_REQ;
		}	
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st2_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
			//Fault
		}	
	}
}

static void Cmic_Load_Step2_REQ(void)
{
	/* STEP 21 : Load files (CONFIGURATION, ALL NODES) *******************************/
	/* STEP 22 : Load files (CONFIGURATION, ALL MANAGERS) ****************************/
	/* STEP 23 : Load files (CONTAINER, ALL NODES) ***********************************/
	adk_debug_BootTimeLog(Interval, LogStart, CmicM_Inst.m_LoadfileStep, CmicM_Inst.m_LoadfileStep);
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

	errorCode = Cmic_RequestLoadFileConfig(CmicM_Inst.m_eFileType, CmicM_Inst.m_DeviceType);

	if (errorCode != ADI_WIL_ERR_SUCCESS){
		// adk_debug_Report(DBG_wil_LoadFile, errorCode);
	}
	CmicM_Inst.m_nBOOT = CmicM_Inst.m_LoadfileStep;
	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st2_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Load_Step2_RES(void)
{
	adi_wil_err_t errorCode;

	if (CmicM_Inst.m_LoadfileStep == Demo_ExecuteLoadFile_config_allmngr___){
		CmicM_Inst.m_DeviceType = ADI_WIL_DEV_ALL_MANAGERS;
	}
	else if(CmicM_Inst.m_LoadfileStep == Demo_ExecuteLoadFile_cntain_allnode___){
		CmicM_Inst.m_DeviceType = ADI_WIL_DEV_ALL_NODES;
		CmicM_Inst.m_eFileType = ADI_WIL_FILE_TYPE_BMS_CONTAINER;
	}

	if (IsReleaseWilAPI(&packInstance)){

		switch(CmicM_Inst.m_notifyRC)
		{
			case  ADI_WIL_ERR_FAIL :
			case  ADI_WIL_ERR_TIMEOUT :
				CmicM_Inst.m_clientData.LoadFileStatus.iOffset = 0;
				
				errorCode = Cmic_RequestLoadFileConfig(CmicM_Inst.m_eFileType, CmicM_Inst.m_DeviceType);
				break;
				
			case  ADI_WIL_ERR_IN_PROGRESS :	
				
				errorCode = Cmic_RequestLoadFileConfig(CmicM_Inst.m_eFileType, CmicM_Inst.m_DeviceType);
				break;

			case  ADI_WIL_ERR_PARTIAL_SUCCESS :
			case  ADI_WIL_ERR_SUCCESS :
				adk_debug_BootTimeLog(Interval, LogEnd__, CmicM_Inst.m_LoadfileStep, CmicM_Inst.m_LoadfileStep);

				CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st1_REQ;

				if (CmicM_Inst.m_LoadfileStep == Demo_ExecuteLoadFile_cntain_allnode___){
					CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st3_REQ;
				}
				else{
					CmicM_Inst.m_LoadfileStep += 1;
				}
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

static void Cmic_Load_Step3_REQ(void)
{
	/* STEP 24 : Reset Device (ALL NODES) ********************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 250, Demo_ExecuteResetDevice_allnode_______);

	Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_NODES);

	CmicM_Inst.m_nBOOT = 250;
	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st3_RES;
	CmicM_Inst.m_nTaskCnt = 0;

}

static void Cmic_Load_Step3_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 250, Demo_ExecuteResetDevice_allnode_______);

		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st4_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
			//Fault
		}	
	}
}

static void Cmic_Load_Step4_REQ(void)
{
	/* STEP 25 : Reset Device (ALL MANAGERS) *****************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 251, Demo_ExecuteResetDevice_allmngr_______);

	Cmic_RequestResetDevice(&packInstance, ADI_WIL_DEV_ALL_MANAGERS);

	CmicM_Inst.m_nBOOT = 251;
	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st4_RES;
	CmicM_Inst.m_nTaskCnt = 0;

}

static void Cmic_Load_Step4_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 251, Demo_ExecuteResetDevice_allmngr_______);

		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st5_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
			//Fault
		}	
	}
}

static void Cmic_Load_Step5_REQ(void)
{
	/* STEP 26 : Disconnect **********************************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 260, Demo_ExecuteDisconnect_1______________);

	Cmic_ExecuteDisconnect(&packInstance);

	CmicM_Inst.m_nBOOT = 260;
	adk_debug_BootTimeLog(Interval, LogEnd__, 260, Demo_ExecuteDisconnect_1______________);

	/* STEP 27 : Connect *************************************************************/
	adk_debug_BootTimeLog(Interval, LogStart, 261, Demo_ExecuteConnect_2_________________);

	Cmic_RequestConnect(&packInstance, CmicM_Inst.m_wbmsSysSensorData, (BMS_DATA_PACKET_COUNT + PMS_DATA_PACKET_COUNT + EMS_DATA_PACKET_COUNT));
	
	CmicM_Inst.m_nBOOT = 261;
	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st5_RES;
	CmicM_Inst.m_nTaskCnt = 0;
}

static void Cmic_Load_Step5_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 261, Demo_ExecuteConnect_2_________________);
		
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st6_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
			//Fault
		}	
	}
}

static void Cmic_Load_Step6_REQ(void)
{
	/* STEP 28 : Set Mode (COMMISSIONING) ********************************************/
	adi_wil_mode_t currentMode;

    Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode ) // COMMISSIONING
	{
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st7_REQ;
		CmicM_ControlBootState();
		
	}else { //STANDBY -> COMMISSIONING 
	
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_COMMISSIONING );
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st6_RES;
	}

	CmicM_Inst.m_nTaskCnt=0; 
}

static void Cmic_Load_Step6_RES(void)
{
	adk_debug_BootTimeLog(Interval, LogEnd__, 220, Demo_ExecuteSetMode_STANDBY___________);
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st7_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Load_Step7_REQ(void)
{
	/* STEP 29 : Get Network Status **************************************************/
    adk_debug_BootTimeLog(Interval, LogStart, 270, Demo_ExecuteGetNetworkStatus_1________);
	
    adi_wil_example_ExecuteGetNetworkStatus(&packInstance, &CmicM_Inst.m_networkStatus, SET_MODE);
    CmicM_Inst.m_nBOOT = 270;

	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st7_RES;
	CmicM_Inst.m_nTaskCnt=0; 
}

static void Cmic_Load_Step7_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		adk_debug_BootTimeLog(Interval, LogEnd__, 270, Demo_ExecuteGetNetworkStatus_1________);

		CmicM_Inst.m_tSt.m_eSubJoin = eLOAD_st8_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Load_Step8_REQ(void)
{
    /* STEP 30 : Set Mode (STANDBY) **************************************************/
	adi_wil_mode_t currentMode;

    Cmic_ExecuteGetMode(&packInstance, &currentMode);
    	
	if ( ADI_WIL_MODE_STANDBY != currentMode )
	{
		Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY );	
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st8_RES;
	}else {
	
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st9_REQ;
		CmicM_ControlBootState();
	}
	CmicM_Inst.m_nTaskCnt=0; 

}

static void Cmic_Load_Step8_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st9_REQ;
		CmicM_ControlBootState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}	
	} 
}

static void Cmic_Load_Step9_REQ(void)
{
    /* STEP 31 : Get File CRC (Container) ********************************************/
	adi_wil_err_t errorCode;
	adk_debug_BootTimeLog(Interval, LogStart, 272, Demo_GetFileCRC_Container_____________);

	errorCode = Cmic_RequestGetFileCRC(&packInstance, ADI_WIL_DEV_ALL_NODES, ADI_WIL_FILE_TYPE_BMS_CONTAINER, SET_MODE);

	CmicM_Inst.m_nBOOT = 272;
	CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_st9_RES;

	CmicM_Inst.m_nTaskCnt = 0;
}

adi_wil_err_t Cmic_RequestGetFileCRC(adi_wil_pack_t * const pPack,
                                         adi_wil_device_t eDevice,
                                         adi_wil_file_type_t eFileType, 
                                         bool no_set_mode)
{
    adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;

    /* Call the adi_wil_GetFileCRC API */
    errorCode = adi_wil_GetFileCRC(pPack, eDevice, eFileType);
    if (errorCode != ADI_WIL_ERR_SUCCESS)
    {
        /* Handle error */
    }
    return errorCode;
}


static void Cmic_Load_Step9_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)) {
		adk_debug_BootTimeLog(Interval, LogEnd__, 272, 	Demo_GetFileCRC_Container_____________);

		CmicM_Inst.m_tSt.m_eBoot = eBOOT_ACTIVE;
		CmicM_Inst.m_tSt.m_eSubLoad = eLOAD_stEND;
		CmicM_Inst.m_tSt.m_eSubActive = eACTIVE_st8_REQ;
		CmicM_ControlBootState();
	}
	else{
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
			//Fault
		}
	}
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

		CmicM_Inst.m_tSt.m_eSensing = eSENSING_st0_IDLE;
		CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st0_IDLE;
			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}

}

static void Cmic_Sensing_Step0_IDLE(void)
{
	if ( eMAIN_SENSING != CmicM_Inst.m_tSt.m_ePrevMain ){
		CmicM_Inst.m_tSt.m_eSensing = eSENSING_st1_REQ;
		CmicM_ControlSensingState();
	}
}

static void Cmic_Sensing_Step1_REQ(void)
{

	Cmic_PresetModifyData(); //DCC default roll back

	CmicM_Inst.m_nNodeCount = 0;

	if ( !Cmic_CheckNode()){
		CmicM_Inst.m_tSt.m_eSensing = eSENSING_st0_IDLE;
		CmicM_Inst.m_tSt.m_ePrevMain = CmicM_Inst.m_tSt.m_eMain;
		return;
	}else {
		Cmic_RequestModifyScript_CFG_A();
	}
   	
	CmicM_Inst.m_tSt.m_eSensing = eSENSING_st1_RES;
	CmicM_Inst.m_nTaskCnt=0;
	
}


static void Cmic_Sensing_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC != ADI_WIL_ERR_SUCCESS) {
              CmicM_Inst.m_tScriptChange.bFailureFlag[CmicM_Inst.m_nNodeCount] = true;
		}        
				
		CmicM_Inst.m_tSt.m_eSensing = eSENSING_st2_REQ;
		CmicM_ControlSensingState();
			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}
}

static void Cmic_Sensing_Step2_REQ(void)
{
	Cmic_RequestModifyScript_CFG_B();
	CmicM_Inst.m_tSt.m_eSensing = eSENSING_st2_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Sensing_Step2_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC != ADI_WIL_ERR_SUCCESS) {
             CmicM_Inst.m_tScriptChange.bFailureFlag[CmicM_Inst.m_nNodeCount] = true;
		}  
		CmicM_Inst.m_NODE[CmicM_Inst.m_nNodeCount++].CB_STAT = 0;  //CB_STAT=0 & Next Node Set!
		
		if ( !Cmic_CheckNode()){ //실행할 다음노드가 없음.
			CmicM_Inst.m_tSt.m_eSensing = eBALANCING_st0_IDLE;
			CmicM_Inst.m_tSt.m_ePrevMain = CmicM_Inst.m_tSt.m_eMain;
			return;

		}else {
			Cmic_RequestModifyScript_CFG_A();
			CmicM_Inst.m_tSt.m_eSensing = eSENSING_st1_RES;
		}

			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}


static void Cmic_Balancing_Step0_IDLE(void)
{
	if ( CmicM_Inst.m_tSt.m_eMain != CmicM_Inst.m_tSt.m_ePrevMain ){
		CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st1_REQ;
		CmicM_ControlBalancingState();
	}
}

static void Cmic_Balancing_Step1_REQ(void)
{
	//Balancing 
	Cmic_PresetModifyData();

	CmicM_Inst.m_nNodeCount = 0;

	if ( !Cmic_CheckNode()){
		CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st0_IDLE;
		CmicM_Inst.m_tSt.m_ePrevMain = CmicM_Inst.m_tSt.m_eMain;
		return;
	}else {
		Cmic_RequestModifyScript_CFG_A();
	}
   	
	CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st1_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Balancing_Step1_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC != ADI_WIL_ERR_SUCCESS) {
              CmicM_Inst.m_tScriptChange.bFailureFlag[CmicM_Inst.m_nNodeCount] = true;
		}        
				
		CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st2_REQ;
		CmicM_ControlBalancingState();
			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}

static void Cmic_Balancing_Step2_REQ(void)
{
	Cmic_RequestModifyScript_CFG_B();
	CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st2_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_Balancing_Step2_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC != ADI_WIL_ERR_SUCCESS) {
             CmicM_Inst.m_tScriptChange.bFailureFlag[CmicM_Inst.m_nNodeCount] = true;
		}        
		 ++CmicM_Inst.m_nNodeCount;  //Next Node Set!
		
		if ( !Cmic_CheckNode()){ //실행할 다음노드가 없음.
			CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st0_IDLE;
			CmicM_Inst.m_tSt.m_ePrevMain = CmicM_Inst.m_tSt.m_eMain;
			return;

		}else {
			Cmic_RequestModifyScript_CFG_A();
			CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st1_RES;
		}

			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}

}

static void Cmic_KeyOn_Step1_REQ(void)
{
    adk_debug_BootTimeLog(Interval, LogStart, 703, Demo_TaskStart________________________);
	Cmic_ExecuteInitialize();
	Cmic_CallProcessTask();
	CmicM_Inst.m_nBOOT = 703;
	adk_debug_BootTimeLog(Interval, LogEnd__, 703, Demo_TaskStart________________________);
	
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st2_REQ; 

}

static void Cmic_KeyOn_Step1_RES(void)
{

}

static void Cmic_KeyOn_Step2_REQ(void)
{
	adk_debug_BootTimeLog(Interval, LogStart, 710, Demo_ExcuteConnect____________________);
   	
    Cmic_RequestConnect(&packInstance,  CmicM_Inst.m_wbmsSysSensorData,
                                       (BMS_DATA_PACKET_COUNT));
	CmicM_Inst.m_nBOOT = 710;
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st2_RES;		
	CmicM_Inst.m_nTaskCnt=0;						   
}

static void Cmic_KeyOn_Step2_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

	    adk_debug_BootTimeLog(Interval, LogEnd__, 710, Demo_ExcuteConnect____________________);

		CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st3_REQ;
		CmicM_ControlKeyOnState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}
}

static void Cmic_KeyOn_Step3_REQ(void)
{
	Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
		
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st3_RES;		
	CmicM_Inst.m_nTaskCnt=0;						   
}

static void Cmic_KeyOn_Step3_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st4_REQ;
		CmicM_ControlKeyOnState();
	}
	else {
		if( ++CmicM_Inst.m_nTaskCnt > 1000) {
				//Fault
		}	
	}
}

static void Cmic_KeyOn_Step4_REQ(void)
{
	if(realAcl.iCount == 0){          
        memcpy(&realAcl, &userAcl, sizeof(adi_wil_acl_t));
    }
	adk_debug_BootTimeLog(Interval, LogStart, 712, Demo_GetNetworkStatusACTmode__________);
	CmicM_Inst.m_bAllNodesJoined = Cmic_RequestGetNetworkStatus(&packInstance, &CmicM_Inst.m_networkStatus);
	 
    CmicM_Inst.m_nBOOT = 712;
    adk_debug_BootTimeLog(Interval, LogEnd__, 712, Demo_GetNetworkStatusACTmode__________);

	 CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st5_REQ;
	 CmicM_ControlKeyOnState();
}

static void Cmic_KeyOn_Step4_RES(void)
{
	
}

static void Cmic_KeyOn_Step5_REQ(void)
{
    adk_debug_BootTimeLog(Interval, LogStart, 720, Demo_TaskStartCB______________________);
    Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_ACTIVE);
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st5_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_KeyOn_Step5_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){
		CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st6_REQ;
		 CmicM_ControlKeyOnState();
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}	
}

static void Cmic_KeyOn_Step6_REQ(void)
{
      
	adi_wil_EnableNetworkDataCapture(&packInstance, CmicM_Inst.m_networkDataBuffer,
	  	                 (CmicM_Inst.m_networkStatus.iCount * ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL), true);

	/* STEP 38 : Init CB_CELL for demo ***************************************************/
	  for(uint8 i=0; i<12; i++){	 /* ADBMS6833 */
		  for(uint8 j=0; j<18; j++){
			  CmicM_Inst.m_bCB_CELL[i][j] = true;
		  }
	  }

    CmicM_Inst.m_nBOOT = 720;
    adk_debug_BootTimeLog(Interval, LogEnd__, 720, Demo_TaskStartCB______________________);
	

   	Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
		         			ADI_WIL_SENSOR_ID_BMS, ADI_BMS_INIT_ID); // INIT
	CmicM_Inst.m_tSt.m_eKeyOn = eKEY_ON_st6_RES;
}

static void Cmic_KeyOn_Step6_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){	

		adk_debug_BootTimeLog(Overall_, LogEnd__, 730, Demo_key_on_event_____________________);
		CmicM_Inst.m_fBOOT_TIME = BOOT_TIME[0].DURATION;

		Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
		         			ADI_WIL_SENSOR_ID_BMS, ADI_BMS_BASE_ID); // BASE

		CmicM_Inst.m_tSt.m_eMain = eMAIN_SENSING;
		CmicM_Inst.m_tSt.m_ePrevMain = eMAIN_SENSING;

		CmicM_Inst.m_tSt.m_eSensing = eSENSING_st0_IDLE;
		CmicM_Inst.m_tSt.m_eBalancing = eBALANCING_st0_IDLE;
			
	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 100) {
				//Fault
		}	
	}
}

/// @KEY OFF  ////////////////////////////////////////////////////////////////////////////////
////
static void Cmic_KeyOff_Step0_IDLE(void){

	if (eMAIN_KEY_OFF_EVENT != CmicM_Inst.m_tSt.m_ePrevMain){
		CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st1_REQ;
		CmicM_ControlKeyOffState();
	}
}

static void Cmic_KeyOff_Step1_REQ(void)
{
	CmicM_Inst.m_bLatent0_Notify = false;
	CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st1_RES;

	Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
							ADI_WIL_SENSOR_ID_BMS, ADI_BMS_LATENT0_ID);
            
}

static void Cmic_KeyOff_Step1_RES(void)
{
	if (CmicM_Inst.m_bLatent0_Notify) {

		CmicM_Inst.m_bLatent0_Notify = false;
		CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st2_REQ;
		CmicM_ControlKeyOffState();
	}
}

static void Cmic_KeyOff_Step2_REQ(void)
{
	CmicM_Inst.m_bLatent1_Notify = false;
	CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st2_RES;

	Cmic_RequestSelectScript(&packInstance, ADI_WIL_DEV_ALL_NODES, 
							ADI_WIL_SENSOR_ID_BMS, ADI_BMS_LATENT1_ID);	

}

static void Cmic_KeyOff_Step2_RES(void)
{
	if (CmicM_Inst.m_bLatent1_Notify) {

		CmicM_Inst.m_bLatent1_Notify = false;
		CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st3_REQ;
		CmicM_ControlKeyOffState();
	}
}

static void Cmic_KeyOff_Step3_REQ(void)
{
	Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_STANDBY);
		
	CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st3_RES;
	CmicM_Inst.m_nTaskCnt=0;
	
}

static void Cmic_KeyOff_Step3_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		if (CmicM_Inst.m_notifyRC == ADI_WIL_ERR_PARTIAL_SUCCESS) {
			CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st3_REQ;
			
		}else {
			CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st4_REQ;
		}
		CmicM_ControlKeyOffState();	

	}else {

		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}	
	}
}

static void Cmic_KeyOff_Step4_REQ(void)
{
	Cmic_RequestSetMode(&packInstance, ADI_WIL_MODE_SLEEP);
		
	CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st4_RES;
	CmicM_Inst.m_nTaskCnt=0;
}

static void Cmic_KeyOff_Step4_RES(void)
{
	if (IsReleaseWilAPI(&packInstance)){

		CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st5_REQ;
		CmicM_ControlKeyOffState();	

	}else {
		if( ++CmicM_Inst.m_nTaskCnt > 10000) {
							//Fault
		}		
	}

}

static void Cmic_KeyOff_Step5_REQ(void)
{
//    adi_wil_err_t  errorCode;

	if (ADI_WIL_ERR_SUCCESS != Cmic_ExecuteDisconnect(&packInstance)){
		//error		
	}

	if ( ADI_WIL_ERR_SUCCESS != adi_wil_Terminate()){
		//error
	}

	CmicM_Inst.m_tSt.m_ePrevMain = eMAIN_KEY_OFF_EVENT;
	CmicM_Inst.m_tSt.m_eKeyOff = eKEY_OFF_st0_IDLE;
}

static void Cmic_KeyOff_Step5_RES(void)
{

}

MAIN_STATE_E Cmic_GetMainState(void)
{
	return  CmicM_Inst.m_tSt.m_eMain;
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

			if (CmicM_Inst.m_tSt.m_eMain == eMAIN_KEY_ON_EVENT){ 
				Cmic_ReadInitPacket();
			}else{
				Cmic_ReadBMS();	
			}
			CmicM_Inst.m_nTotalPacketRcvd = ((adi_wil_sensor_data_buffer_t *)pData)->iCount;  /*  @remark Akash : Variable to store total no. of bms packets received */
  
		
			if (CmicM_Inst.m_tSt.m_eMain == eMAIN_KEY_OFF_EVENT){ 
			
				Cmic_SaveLatencyPkt();
			}
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
	sint16 	tempBuf[22]={0,};
	
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
			
            tempBuf[0]  = (signed short int)((pBMS_Pkt_0->Rdaca.iAc2v[1]   << 8) + pBMS_Pkt_0->Rdaca.iAc2v[0]);
            tempBuf[1]  = (signed short int)((pBMS_Pkt_0->Rdaca.iAc3v[1]   << 8) + pBMS_Pkt_0->Rdaca.iAc3v[0]);
            tempBuf[2]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc4v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc4v[0]);
            tempBuf[3]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc5v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc5v[0]);
            tempBuf[4]  = (signed short int)((pBMS_Pkt_0->Rdacb.iAc6v[1]   << 8) + pBMS_Pkt_0->Rdacb.iAc6v[0]);
            tempBuf[5]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc7v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc7v[0]);
            tempBuf[6]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc8v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc8v[0]);
            tempBuf[7]  = (signed short int)((pBMS_Pkt_0->Rdacc.iAc9v[1]   << 8) + pBMS_Pkt_0->Rdacc.iAc9v[0]);
            tempBuf[8]  = (signed short int)((pBMS_Pkt_0->Rdacd.iAc11v[1]  << 8) + pBMS_Pkt_0->Rdacd.iAc11v[0]);
            tempBuf[9]  = (signed short int)((pBMS_Pkt_0->Rdacd.iAc12v[1]  << 8) + pBMS_Pkt_0->Rdacd.iAc12v[0]);
            tempBuf[10] = (signed short int)((pBMS_Pkt_0->Rdace.iAc13v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc13v[0]);
            tempBuf[11] = (signed short int)((pBMS_Pkt_0->Rdace.iAc14v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc14v[0]);
            tempBuf[12] = (signed short int)((pBMS_Pkt_0->Rdace.iAc15v[1]  << 8) + pBMS_Pkt_0->Rdace.iAc15v[0]);
            tempBuf[13] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc16v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc16v[0]);
            tempBuf[14] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc17v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc17v[0]);
            tempBuf[15] = (signed short int)((pBMS_Pkt_0->Rdacf.iAc18v[1]  << 8) + pBMS_Pkt_0->Rdacf.iAc18v[0]);
            tempBuf[16] = (signed short int)((pBMS_Pkt_0->Rdauxa.iG1v[1]   << 8) + pBMS_Pkt_0->Rdauxa.iG1v[0]);
            tempBuf[17] = (signed short int)((pBMS_Pkt_0->Rdauxa.iG2v[1]   << 8) + pBMS_Pkt_0->Rdauxa.iG2v[0]);
            tempBuf[18] = (signed short int)((pBMS_Pkt_0->Rdauxa.iGa11v[1] << 8) + pBMS_Pkt_0->Rdauxa.iGa11v[0]);
            tempBuf[19] = (signed short int)((pBMS_Pkt_0->Rdauxe.iG3v[1]   << 8) + pBMS_Pkt_0->Rdauxe.iG3v[0]);

			for( i = 0; i < 16; i++)
			{
                /* @remark : Cell voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].CELL_V[i] = (float)(tempBuf[i] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : Cell voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].CELL_Vi[i] = tempBuf[i] + 10000;
            }

            for( i = 0; i < 4; i++)
			{
                /* @remark : AUX voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].TEMP_V[i] = (float)(tempBuf[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : AUX voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].TEMP_Vi[i] = tempBuf[i + 16] + 10000;  
            }

        }
    }
}

void Cmic_ReadBMS(void)
{
    uint8_t cnt, packetId, eNode, i;
	sint16 	tempBuf[22]={0,}; 
	
  
    for(cnt = 0; cnt < CmicM_Inst.m_nTotalPacketRcvd; cnt++){
		
        if (CmicM_Inst.m_userBMSBuf[cnt].iLength == 0) continue; // Adele: skip processing of empty packet

        eNode = Cmic_ConvertDeviceId(CmicM_Inst.m_userBMSBuf[cnt].eDeviceId);

		packetId = CmicM_Inst.m_userBMSBuf[cnt].Data[0];
                
        if( packetId == ADI_BMS_BASE_PKT_0_ID )
		{
			CmicM_Inst.m_nBMSNotifyCnt++; // for debug..
        
			CmicM_Inst.m_pBMS_Pkt_0 = (adi_bms_base_pkt_0_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;
            
            /* @remark : Read raw data (float) */
            tempBuf[0]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdaca.iAc2v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdaca.iAc2v[0]);
            tempBuf[1]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdaca.iAc3v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdaca.iAc3v[0]);
            tempBuf[2]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc4v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc4v[0]);
            tempBuf[3]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc5v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc5v[0]);
            tempBuf[4]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc6v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacb.iAc6v[0]);
            tempBuf[5]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc7v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc7v[0]);
            tempBuf[6]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc8v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc8v[0]);
            tempBuf[7]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc9v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacc.iAc9v[0]);
            tempBuf[8]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacd.iAc11v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacd.iAc11v[0]);
            tempBuf[9]  = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacd.iAc12v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacd.iAc12v[0]);
            tempBuf[10] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc13v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc13v[0]);
            tempBuf[11] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc14v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc14v[0]);
            tempBuf[12] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc15v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdace.iAc15v[0]);
            tempBuf[13] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc16v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc16v[0]);
            tempBuf[14] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc17v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc17v[0]);
            tempBuf[15] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc18v[1]  << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdacf.iAc18v[0]);
            tempBuf[16] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iG1v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iG1v[0]);
            tempBuf[17] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iG2v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iG2v[0]);
            tempBuf[18] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iGa11v[1] << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdauxa.iGa11v[0]);
            tempBuf[19] = (signed short int)((CmicM_Inst.m_pBMS_Pkt_0->Rdauxe.iG3v[1]   << 8) + CmicM_Inst.m_pBMS_Pkt_0->Rdauxe.iG3v[0]);

                  
            for( i = 0; i < 16; i++){
                /* @remark : Cell voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].CELL_V[i] = (float)(tempBuf[i]  * CELL_UNIT / 1000000.0f) + CELL_OFFSET;
                /* @remark : Cell voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].CELL_Vi[i] = tempBuf[i] + 10000;
            }

            for(i = 0; i < 4; i++){
                /* @remark : AUX voltage convert (float) */
                CmicM_Inst.m_NODE[eNode].TEMP_V[i]  = (tempBuf[i + 16] * CELL_UNIT / 1000000.0f) + CELL_OFFSET;

                /* @remark : AUX voltage convert (int16) */
                CmicM_Inst.m_NODE[eNode].TEMP_Vi[i]  = tempBuf[i + 16] + 10000;  
            }
  
            Cmic_ADBMS683x_Monitor_Base_Pkt0(&CmicM_Inst.m_userBMSBuf[cnt]);  
			
        }
        else if (packetId == ADI_BMS_BASE_PKT_1_ID)
		{
      
            CmicM_Inst.m_pBMS_Pkt_1 = (adi_bms_base_pkt_1_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;

            CmicM_Inst.m_NODE[eNode].CB_STAT = ((CmicM_Inst.m_pBMS_Pkt_1->Rdcfga.iCfgar2 & BMS_CELLS_1_TO_2_MASK) << 16) + (CmicM_Inst.m_pBMS_Pkt_1->Rdcfgb.iCfgbr5 << 8) + CmicM_Inst.m_pBMS_Pkt_1->Rdcfgb.iCfgbr4;

			Cmic_ADBMS683x_Monitor_Base_Pkt1(&CmicM_Inst.m_userBMSBuf[cnt]);       
        }
        else if (packetId == ADI_BMS_BASE_PKT_2_ID)
		{
         
            CmicM_Inst.m_pBMS_Pkt_2 = (adi_bms_base_pkt_2_t*) &CmicM_Inst.m_userBMSBuf[cnt].iLength;

            CmicM_Inst.m_NODE[eNode].OWD_CS_STAT = ((CmicM_Inst.m_pBMS_Pkt_2->Rdstatc.iStcr2 & 0xC0) << 10) + (CmicM_Inst.m_pBMS_Pkt_2->Rdstatc.iStcr1 << 8) + CmicM_Inst.m_pBMS_Pkt_2->Rdstatc.iStcr0;
           
        }
        else{
           
        }

        if(iOWDPcktsRcvd[eNode] == SM_ADBMS6833_ALL_PCKTS_FOR_OWD_RCVD)
        {
            Cmic_ADBMS683x_Monitor_Cell_OWD(eNode);
            iOWDPcktsRcvd[eNode] = 0U;
        }
    }
	
	for(cnt = 0; cnt < realAcl.iCount; cnt++)
    {
        // Adele
        uint8_t* bms_pkt_map = CmicM_Inst.m_NODE[cnt].BMS_PKT_MAP;
        //if(iBMSNotificationCount < 5)
        //    bms_pkt_map[1] = 0xFF;
        //bms_pkt_map[0] = tempBuf[cnt];
        if (adi_cnt1_8bit(bms_pkt_map[0]) < adi_cnt1_8bit(bms_pkt_map[1]))
            bms_pkt_map[1] = bms_pkt_map[0];
    }
	
}

adi_bms_latent0_pkt_0_t Latency0_Pkt_0;
adi_bms_latent0_pkt_1_t Latency0_Pkt_1;
adi_bms_latent0_pkt_2_t Latency0_Pkt_2;
adi_bms_latent1_pkt_0_t Latency1_Pkt_0;
adi_bms_latent1_pkt_1_t Latency1_Pkt_1;

void Cmic_SaveLatencyPkt(void)
{
	static  uint8  latent0_received[ADK_MAX_node]= {0,};
	static  uint8  latent1_received[ADK_MAX_node]= {0,};
	
	bool      latent0_result=true, latent1_result=true;
	uint8	  eNode, count; 


	 /* @remark  : check BASE packet header */
	 for(count = 0; count < CmicM_Inst.m_nTotalPacketRcvd; count++){

		 eNode = Cmic_ConvertDeviceId(CmicM_Inst.m_userBMSBuf[count].eDeviceId);
		 
		 if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_0_ID)
		 {
			CmicM_Inst.m_nLatentNotifyCnt++;  //for debug.
			 latent0_received[eNode] |= 0x01;
			 memcpy(&Latency0_Pkt_0, (adi_bms_latent0_pkt_0_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_0.Rdstatc, (adi_bms_latent0_pkt_0_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_0_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_1_ID)
		 {
			 latent0_received[eNode] |= 0x02;
			 memcpy(&Latency0_Pkt_1, (adi_bms_latent0_pkt_1_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_1.Rdpwma_2, (adi_bms_latent0_pkt_1_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_1_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT0_PKT_2_ID)
		 {					
			 latent0_received[eNode] |= 0x04;
			 memcpy(&Latency0_Pkt_2, (adi_bms_latent0_pkt_2_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency0_Pkt_2.Rdace, (adi_bms_latent0_pkt_2_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent0_pkt_2_t) - 8);
		 		 	
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT1_PKT_0_ID)
		 {					 
			 latent1_received[eNode] |= 0x01;
			 memcpy(&Latency1_Pkt_0, (adi_bms_latent1_pkt_0_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency1_Pkt_0.Rdaca, (adi_bms_latent1_pkt_0_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_0_t) - 8);
		 }
		 else if (CmicM_Inst.m_userBMSBuf[count].Data[0] == ADI_BMS_LATENT1_PKT_1_ID)
		 {					 
			 latent1_received[eNode] |= 0x02;
			 memcpy(&Latency1_Pkt_1, (adi_bms_latent1_pkt_1_t*)CmicM_Inst.m_userBMSBuf[count].Data, sizeof(adi_bms_packetheader_t));
			 memcpy(&Latency1_Pkt_1.Rdsvd, (adi_bms_latent1_pkt_1_t*)&CmicM_Inst.m_userBMSBuf[count].Data[sizeof(adi_bms_packetheader_t)], sizeof(adi_bms_latent1_pkt_1_t) - 8);
		 }
 		
	 }
	
	 for(count = 0; count < realAcl.iCount ; count++){

	 	if (0x07 != latent0_received[count]) //LATENT PACKET 0 not all received ..
	 	{
			latent0_result = false;			
		}
		if (0x03 != latent1_received[count]) //LATENT PACKET 0 not all received ..
	 	{
			latent1_result = false;			
		}		
	 }		

	 if (latent0_result) {//모든 노드에 대한 latent0를 수신했을 경우.
	 	
		CmicM_Inst.m_bLatent0_Notify = true;
		memset(latent0_received, 0, ADK_MAX_node);      
	 }
	 if (latent1_result) {//모든 노드에 대한 latent1를 수신했을 경우.

	 	CmicM_Inst.m_bLatent1_Notify = true;	
		memset(latent1_received, 0, ADK_MAX_node);
	 }
}

void Cmic_PresetModifyData(void)
{
	uint8 nodeCnt, cellCnt;

	memset(&CmicM_Inst.m_tScriptChange, 0, sizeof(CmicM_Inst.m_tScriptChange));

	// 
	for( nodeCnt = 0; nodeCnt < realAcl.iCount; nodeCnt++){
        CmicM_Inst.m_bCB_NODE[nodeCnt] = false;
        CmicM_Inst.m_iDecTemp[nodeCnt] = 0;
        CmicM_Inst.m_iDecFinal[nodeCnt] = 0;
    }

	for(nodeCnt = 0; nodeCnt < realAcl.iCount; nodeCnt++){
        for(cellCnt = 0; cellCnt < ADK_MAX_cell; cellCnt++){
            if(CmicM_Inst.m_bCB_CELL[nodeCnt][cellCnt] == true){
                CmicM_Inst.m_iDecTemp[nodeCnt] += (1 << cellCnt);
                CmicM_Inst.m_bCB_NODE[nodeCnt] = true;
            }
        }

        /**
         * @remark : Cell balancing user input goes here
         */    
        if(eMAIN_BALANCING_EVEN == CmicM_Inst.m_tSt.m_eMain){
            CmicM_Inst.m_iDecFinal[nodeCnt] = CmicM_Inst.m_iDecTemp[nodeCnt];
            CmicM_Inst.m_iDecFinal[nodeCnt] &= 0x2A954; //EVEN for [6833]
        }
        else if(eMAIN_BALANCING_ODD == CmicM_Inst.m_tSt.m_eMain){
            CmicM_Inst.m_iDecFinal[nodeCnt] = CmicM_Inst.m_iDecTemp[nodeCnt];
            CmicM_Inst.m_iDecFinal[nodeCnt] &= 0x154AA; //ODD for  [6833]
        }
        else{ //sensing
            CmicM_Inst.m_iDecFinal[nodeCnt] = 0; //DCC default roll back
        }          
    }

	CmicM_Inst.m_tScriptChange.iCount = realAcl.iCount; //node count
	memcpy(CmicM_Inst.m_tScriptChange.iDeviceList, realAcl.Data, sizeof(uint8_t) * 8 * realAcl.iCount); //MAC address

}


bool Cmic_CheckNode(void)
{
	bool   bExecute=false;	//밸런싱할 노드를 찾았을 때=true

	for(uint8 cnt = CmicM_Inst.m_nNodeCount; cnt < CmicM_Inst.m_tScriptChange.iCount; cnt++){
		if(CmicM_Inst.m_bCB_NODE[cnt] == true){
		    bExecute=true;  
			CmicM_Inst.m_nNodeCount = cnt;
			break;
		}
	}
	
	return bExecute;
}


void Cmic_RequestModifyScript_CFG_A(void)
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
	uint16 currPktTimestamp=0, calPktTimestamp=0;
	uint16 pecVal = 0;
	uint8  devID = 0;
	uint8  nodeCnt = CmicM_Inst.m_nNodeCount;

	//Setting WRCFGA 6Bytes 
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[0] = 0x80; //REFON=1, CTH[2:0]=0
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[1] = 0x00;  //FLAG_D[7:0]=0      
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_DCC_OFFSET] = 0x40; //OWRNG=1 (?)
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_DCC_OFFSET] |= (uint8_t)((CmicM_Inst.m_iDecFinal[nodeCnt] / 0x10000 ) & BMS_CELLS_1_TO_2_MASK); //WRCFGA DCC[18,17]
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_DCC_OFFSET+1] = 0xFF; //GPIO[8~1] pull-down off
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_DCC_OFFSET+2] = 0x03; //GPIO[10,9] pull-down off
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_DCC_OFFSET+3] = 0x07; //FC[2:0]:IIR Filter
 
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeDataLength = BMS_SCRIPT_WRCFGA_DATA_LENGTH;    /* Length of the data to change in the script at the node */
    CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iEntryOffset = BMS_SCRIPT_WRCFGA_OFFSET;              /* Offset to the DCC bits to be changed in Config B register */

   	/* iActivationTime needs to be calculated from latest BMS packet timestamp and ACTIVATION_DELAY specified. 
	   Latest BMS packet will be retrieved from the timestamp field from the latest BMS data packet */
	if(nodeCnt == 0)
	{
		currPktTimestamp = CmicM_Inst.m_userBMSBuf[0].Data[PACKET_HEADER_TIMESTAMP_OFFSET] << 8;
		currPktTimestamp |= CmicM_Inst.m_userBMSBuf[0].Data[PACKET_HEADER_TIMESTAMP_OFFSET+1];
		currPktTimestamp = (currPktTimestamp & 0x7FFF) + ACTIVATION_DELAY;

		calPktTimestamp = (currPktTimestamp & BMS_CELLS_9_TO_16_MASK) >> 8;
		calPktTimestamp |= (currPktTimestamp & BMS_CELLS_1_TO_8_MASK) << 8;
	}
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iActivationTime = calPktTimestamp;

	/* Calculating the PEC for updated configuration A register to be set on the node */
	pecVal = Cmic_PEC10_Calc(true, 0,
							BMS_SCRIPT_WRCFGA_DATA_LENGTH - 2,
							CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData);

	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_PEC_OFFSET] = (uint8_t)(pecVal >> 8);
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGA_PEC_OFFSET+1] = (uint8_t)(pecVal >> 0);

	
	Cmic_MAC_DeviceID_Return(&packInstance, false, &CmicM_Inst.m_tScriptChange.iDeviceList[ADI_WIL_MAC_ADDR_SIZE * nodeCnt], &devID);

	/* Calling ModifyScript node API */
	errorCode = adi_wil_ModifyScript(&packInstance, (adi_wil_device_t)(ADI_WIL_DEV_NODE_0 << devID),
									ADI_WIL_SENSOR_ID_BMS, 	&CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt]);
										
	if (errorCode != ADI_WIL_ERR_SUCCESS) {
		CmicM_Inst.m_tScriptChange.bFailureFlag[nodeCnt] = true;

	}
}

void Cmic_RequestModifyScript_CFG_B(void)
{
	adi_wil_err_t errorCode = ADI_WIL_ERR_SUCCESS;
	uint16 pecVal = 0;
	uint8  devID = 0;
	uint8  nodeCnt = CmicM_Inst.m_nNodeCount;

	//WRCFGB DCC설정
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGB_DCC_OFFSET] = (uint8_t)(CmicM_Inst.m_iDecFinal[nodeCnt] & BMS_CELLS_1_TO_8_MASK);
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGB_DCC_OFFSET+1] = (uint8_t)((CmicM_Inst.m_iDecFinal[nodeCnt] & BMS_CELLS_9_TO_16_MASK) >> 8);
	
	/* Calculating the PEC for updated configuration B register to be set on the node */
	pecVal = Cmic_PEC10_Calc(true, WRCFGB_PEC_OFFSET - WRCFGA_PEC_OFFSET,
							BMS_SCRIPT_WRCFGB_DATA_LENGTH - 2,
							CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData);

	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGB_PEC_OFFSET] = (uint8_t)(pecVal >> 8);
	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[WRCFGB_PEC_OFFSET+1] = (uint8_t)(pecVal >> 0);

	Cmic_MAC_DeviceID_Return(&packInstance, false, &CmicM_Inst.m_tScriptChange.iDeviceList[ADI_WIL_MAC_ADDR_SIZE * nodeCnt], &devID);

    for (int k = 0; k < BMS_SCRIPT_WRCFGB_DATA_LENGTH; k++)
	{
		CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[k] = 
		       	CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeData[k+BMS_SCRIPT_WRCFGA_DATA_LENGTH];
	}
    CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iChangeDataLength = BMS_SCRIPT_WRCFGB_DATA_LENGTH;    /* Length of the data to change in the script at the node */
    CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt].iEntryOffset = BMS_SCRIPT_WRCFGB_OFFSET;
	
	/* Calling ModifyScript node API */
	errorCode = adi_wil_ModifyScript(&packInstance, (adi_wil_device_t)(ADI_WIL_DEV_NODE_0 << devID),
									ADI_WIL_SENSOR_ID_BMS, 	&CmicM_Inst.m_tScriptChange.iDeviceChangeScriptInfo[nodeCnt]);
										
	if (errorCode != ADI_WIL_ERR_SUCCESS) {
		CmicM_Inst.m_tScriptChange.bFailureFlag[nodeCnt] = true;

	}
}


uint16_t Cmic_PEC10_Calc(bool rx_cmd, int offset, int len, uint8_t *data)
{
    uint16_t remainder = 16; /* PEC_SEED;   0000010000 */
    uint16_t polynom = 0x8F; /* x10 + x7 + x3 + x2 + x + 1 <- the CRC15 polynomial         100 1000 1111   48F */

    /* Perform modulo-2 division, a byte at a time. */
    for (int pbyte = offset; pbyte < (offset + len); ++pbyte)
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
        remainder ^= (uint16_t)((data[offset + len] & 0xFC) << 2);

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

/******************************************************************************
 * MAC <-> index retrieval function
 * This is a helper function that will return to the user either the MAC address or
 * the device ID of a node in the network
 *****************************************************************************/
void Cmic_MAC_DeviceID_Return(adi_wil_pack_t *pPack, bool bMacReturn, uint8_t *pMacPtr, uint8_t *pDeviceID)
{
    uint8_t iDeviceID = *pDeviceID;
    /* Check MAC return flag to ascertain if MAC address of the node is to be returned or the device ID */
    if(bMacReturn) {
        for(uint8_t i=0;i<ADI_WIL_MAC_ADDR_SIZE;i++) {
            /*Retrieving the MAC address */
            *(pMacPtr+i) = CmicM_Inst.m_sysAcl.Data[(ADI_WIL_MAC_ADDR_SIZE*iDeviceID)+i];
        }
    }
    else {
        for(uint8_t i=0;i<ADI_WIL_MAX_NODES;i++) {
            bool bMacFound = true;
            for(uint8_t j=0;j<ADI_WIL_MAC_ADDR_SIZE && bMacFound;j++) {
                bMacFound = (CmicM_Inst.m_sysAcl.Data[(ADI_WIL_MAC_ADDR_SIZE*i)+j] == *(pMacPtr+j));
            }
            if(bMacFound == true) {
                /*Retrieving the Device ID */
                *pDeviceID = i;
                break;
            }
        }
    }
}


void Cmic_ADBMS683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr)
{
    uint8_t eNode   = 100;
    adi_bms_base_pkt_0_t* pPacketPtr;

    /* Get Node ID from the BMS packet */
    eNode       = Cmic_ConvertDeviceId(BMSBufferPtr->eDeviceId);

    /* Use corresponding packet structure to easily parse data from the BMS packet */
    pPacketPtr  = (adi_bms_base_pkt_0_t*) &BMSBufferPtr->iLength;
    
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
   
}

void Cmic_ADBMS683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr)
{
    uint8_t eNode   = 100;
    adi_bms_base_pkt_1_t* pPacketPtr;

    /* Get Node ID from the BMS packet */
    eNode       = Cmic_ConvertDeviceId(BMSBufferPtr->eDeviceId);

    /* Use corresponding packet structure to easily parse data from the BMS packet */
    pPacketPtr  = (adi_bms_base_pkt_1_t*) &BMSBufferPtr->iLength;

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
}

void Cmic_ADBMS683x_Monitor_Cell_OWD(adi_wil_device_t eNode)
{
    uint8_t iCellIndx       = 0U;
    int32_t iCellData       = 0U;
    int32_t iCellOWData     = 0U;
    int32_t iCellDrop       = 0U;
    int32_t iCellOWThres_p    = 0U;
    int32_t iCellOWThres_m    = 0U;

  
    aiOWStatus[eNode] = SM_ADBMS6833_STATUS_PASS;
    CmicM_Inst.m_NODE[eNode].OWD_STAT = SM_ADBMS6833_STATUS_PASS;

    for(iCellIndx = 0U; iCellIndx < SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE; iCellIndx++)
    {
        iCellData    = (aiNominalCellData[eNode][iCellIndx] * SM_ADBMS6833_VTG_REG_RESOLUTION) + SM_ADBMS6833_VTG_REG_OFFSET;
        iCellOWData  = (aiOWCellData[eNode][iCellIndx]      * SM_ADBMS6833_VTG_REG_RESOLUTION) + SM_ADBMS6833_VTG_REG_OFFSET;
        iCellDrop    = iCellData - iCellOWData;

        iCellOWThres_p = ((iCellData * SM_ADBMS6833_OW_THRESHOLD_PERCENT) / 100);
        iCellOWThres_m = -((iCellData * SM_ADBMS6833_OW_THRESHOLD_PERCENT) / 100);

        if(aiNominalCellData[eNode][iCellIndx] < SM_ADBMS6833_OW_ZERO_VOL)
        {
            CmicM_Inst.m_NODE[eNode].OWD_FAIL[iCellIndx]++;
        }
        else
        {
            if ((iCellDrop > iCellOWThres_p) || (iCellDrop < iCellOWThres_m))
            {
                CmicM_Inst.m_NODE[eNode].OWD_FAIL[iCellIndx]++;
                aiOWStatus[eNode] = SM_ADBMS6833_STATUS_FAIL;
                CmicM_Inst.m_NODE[eNode].OWD_STAT = SM_ADBMS6833_STATUS_FAIL;
            }
            else{ /* DEBUG */
                CmicM_Inst.m_NODE[eNode].OWD_SUCCESS[iCellIndx]++;
            }
        }
    
    }
  
}

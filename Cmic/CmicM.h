/*
 * CmicM.h
 *
 *  Created on: 2023. 10. 12.
 *      Author: sure
 */

#ifndef CMICM_H_
#define CMICM_H_

#include "Platform_Types.h"
//#include "adi_wil_types.h"
#include "adi_bms_types.h"
#include "adi_bms_container.h"

#include "adi_wil_hal_ticker.h"
#include "adi_wil_example_debug_functions.h"
#include "adi_wil_example_functions.h"
#include "adi_wil_example_config.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_acl.h"
#include "adi_wil_example_cell_balance.h"
#include "adi_wil_example_owd.h"
#include "adi_wil_example_cfg_profiles.h"

/*******************************************************************************
 * Enumerations
 *******************************************************************************/
typedef enum
{
	eMAIN_BOOT,
	eMAIN_SENSING,
	eMAIN_BALANCING_EVEN,
	eMAIN_BALANCING_ODD,
	eMAIN_KEY_ON_EVENT,
	eMAIN_KEY_OFF_EVENT,
}MAIN_STATE_E;

typedef enum
{
    eBOOT_INIT      ,
    eBOOT_CONNECT   ,    
    eBOOT_JOIN      ,
	eBOOT_LOAD      ,
    eBOOT_ACTIVE    ,
    eBOOT_OTA       ,
  
}BOOT_STATE_E;


typedef enum
{
	eINIT_st1_REQ, // REQUEST WBMS INIT & QUERY DEVICE 
	eINIT_st1_RES, // RESPONSE QUERY DEVICE 
	eINIT_stEND,
}BOOTSUB_INIT_E;

typedef enum
{
	eCONNECT_st1_REQ, //(NOT DUAL) REQUEST CONNECT 
	eCONNECT_st1_RES, //RESPONSE   & DUAL CHECK 
	
	eCONNECT_st2_REQ, // REQUEST RESET DEVICE(ALL MANAGER)
	eCONNECT_st2_RES, // RESPONSE 

	eCONNECT_st3_REQ, // REQUEST SET MODE(STANDBY)
	eCONNECT_st3_RES, //	 RESPONSE 

	eCONNECT_st4_REQ, //  REQUEST LOAD FILE(ALL MANAGERS)
	eCONNECT_st4_RES, // RESPONSE

	eCONNECT_st5_REQ, // REQUEST  RESET DEVICE(ALL MANAGER)
	eCONNECT_st5_RES, // RESPONSE & DISCONNECT 
	
	eCONNECT_st6_REQ, //(DUAL ) REQUEST CONNECT 
	eCONNECT_st6_RES, // RESPONSE CONNECT 
	eCONNECT_st7_REQ, // REQUEST SET MODE(STANDBY)
	eCONNECT_st7_RES, // RESPONSE
	eCONNECT_st8_REQ, // REQUEST GET ACL 
	eCONNECT_st8_RES, // RESPONSE & COMPARE ACL 
	eCONNECT_stEND,

}BOOTSUB_CONNECT_E;

typedef enum
{
	eJOIN_st1_REQ, //
	eJOIN_st1_RES, //
	eJOIN_st2_REQ, //
	eJOIN_st2_RES, //
	eJOIN_st3_REQ, //
	eJOIN_st3_RES, //
	eJOIN_st4_REQ, //
	eJOIN_st4_RES, //
	eJOIN_st5_REQ, //
	eJOIN_st5_RES, //
	eJOIN_st6_REQ, //
	eJOIN_st6_RES, //
	eJOIN_st7_REQ, //
	eJOIN_st7_RES, //
	eJOIN_st8_REQ, //
	eJOIN_st8_RES, //
	
	eJOIN_stEND,

}BOOTSUB_JOIN_E;

typedef enum
{
	eLOAD_st1_REQ, //
	eLOAD_st1_RES, //
	eLOAD_st2_REQ, //
	eLOAD_st2_RES, //
	eLOAD_st3_REQ, //
	eLOAD_st3_RES, //
	eLOAD_st4_REQ, //
	eLOAD_st4_RES, //
	eLOAD_st5_REQ, //
	eLOAD_st5_RES, //
	eLOAD_st6_REQ, //
	eLOAD_st6_RES, //
	eLOAD_st7_REQ, //
	eLOAD_st7_RES, //
	eLOAD_st8_REQ, //
	eLOAD_st8_RES, //
	eLOAD_st9_REQ, //
	eLOAD_st9_RES, //
		
	eLOAD_stEND,

}BOOTSUB_LOAD_E;

typedef enum
{
	eACTIVE_st1_REQ, // REAL ACL COPY & REQUEST SET MODE(COMMISSIONNG)
	eACTIVE_st1_RES, // RESPONSE 
	eACTIVE_st2_REQ, // REQUEST GET NETWORK STATUS
	eACTIVE_st2_RES, // RESPONSE 	
	eACTIVE_st3_REQ, //( NOT ALL NODE JOIN) SET MODE (STANDBY)
	eACTIVE_st3_RES, //	RESPONSE
	eACTIVE_st4_REQ, //  REQUEST RESET DEVICE(ALL NODES)
	eACTIVE_st4_RES, //	RESPONSE
	eACTIVE_st5_REQ, //  REQUEST SET ACL 
	eACTIVE_st5_RES, //	RESPONSE
	eACTIVE_st6_REQ, //  REQUEST GET ACL
	eACTIVE_st6_RES, //	RESPONSE 
	eACTIVE_st7_REQ, //  (ALL NODE JOIN) SET MODE (STANDBY) 
	eACTIVE_st7_RES, //	RESPONSE
	eACTIVE_st8_REQ, // REQUEST SET MODE (ACTIVE)
	eACTIVE_st8_RES, //	RESPONSE
	eACTIVE_st9_REQ, //  ENABLE NETWORK DATA CAPTURE
	eACTIVE_st9_RES, //	NO ACTION
	eACTIVE_st10_REQ, // REQUEST SELECT SCRIPT
	eACTIVE_st10_RES, //	 RESPONSE 
	eACTIVE_stEND,

}BOOTSUB_ACTIVE_E;

typedef enum
{
	eSENSING_st0_IDLE,
	eSENSING_st1_REQ, //DCC default rollback
	eSENSING_st1_RES, //RESPONSE
	eSENSING_st2_REQ, //
    eSENSING_st2_RES, //RESPONSE
}SENSING_STATE_E;

typedef enum
{
	eBALANCING_st0_IDLE,
	eBALANCING_st1_REQ, //REQUEST BALANCING-Write CFGA
	eBALANCING_st1_RES, //RESPONSE
	eBALANCING_st2_REQ, //REQUEST BALANCING-Write CFGB
	eBALANCING_st2_RES, //RESPONSE
}BALANCING_STATE_E;

typedef enum
{
	eKEY_ON_st1_REQ, //Initialize & TaskStart
	eKEY_ON_st1_RES, 
	eKEY_ON_st2_REQ, //Connect
	eKEY_ON_st2_RES,
	eKEY_ON_st3_REQ, //SetMode(Standby)
	eKEY_ON_st3_RES,
	eKEY_ON_st4_REQ, //Get NetworkStatus
	eKEY_ON_st4_RES,
	eKEY_ON_st5_REQ, //SetMode(Active)
	eKEY_ON_st5_RES,
	eKEY_ON_st6_REQ, //EnableNetworkData & Select script
	eKEY_ON_st6_RES,	
}KEY_ON_STATE_E;

typedef enum
{
	eKEY_OFF_st0_IDLE,
	eKEY_OFF_st1_REQ, //Latent0 Script start
	eKEY_OFF_st1_RES,
	eKEY_OFF_st2_REQ, //Latent1 Script start
	eKEY_OFF_st2_RES,
	eKEY_OFF_st3_REQ, //SetMode(Standby)
	eKEY_OFF_st3_RES,
	eKEY_OFF_st4_REQ, //SetMode(Sleep)
	eKEY_OFF_st4_RES,
	eKEY_OFF_st5_REQ, //Disconnet&Terminate
	eKEY_OFF_st5_RES,
	
}KEY_OFF_STATE_E;

void CmicM_Init(void);
void CmicM_IG_Init(void);
void CmicM_Handler(void);


MAIN_STATE_E Cmic_GetMainState(void);

adi_wil_err_t Cmic_RequestLoadFileConfig(adi_wil_file_type_t eFileType, adi_wil_device_t eDevice);
adi_wil_err_t Cmic_RequestGetFileCRC(adi_wil_pack_t * const pPack,
                                         adi_wil_device_t eDevice,
                                         adi_wil_file_type_t eFileType, 
                                         bool no_set_mode);
bool Cmic_IsDualConfig(adi_wil_configuration_t * C1, adi_wil_configuration_t * C2);
bool Cmic_CompareACL(void);
uint8 Cmic_ConvertDeviceId (adi_wil_device_t WilDeviceId);
void Cmic_ReadInitPacket(void);
void Cmic_ReadBMS(void);
void Cmic_SaveLatencyPkt(void);
void Cmic_PresetModifyData(void);
bool Cmic_CheckNode(void);
void Cmic_RequestModifyScript_CFG_A(void);
void Cmic_RequestModifyScript_CFG_B(void);
uint16_t Cmic_PEC10_Calc(bool rx_cmd, int offset, int len, uint8_t *data);
void Cmic_MAC_DeviceID_Return(adi_wil_pack_t *pPack, bool bMacReturn, uint8_t *pMacPtr, uint8_t *pDeviceID);
void Cmic_ADBMS683x_Monitor_Base_Pkt0(adi_wil_sensor_data_t* BMSBufferPtr);
void Cmic_ADBMS683x_Monitor_Base_Pkt1(adi_wil_sensor_data_t* BMSBufferPtr);
void Cmic_ADBMS683x_Monitor_Base_Pkt2(adi_wil_sensor_data_t* BMSBufferPtr);
void Cmic_ADBMS683x_Monitor_Cell_OWD(adi_wil_device_t eNode);
#endif /* CMICM_H_ */

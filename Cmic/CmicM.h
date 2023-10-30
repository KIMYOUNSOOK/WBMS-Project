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
}MAIN_STATE_E;

typedef enum
{
    eBOOT_INIT , //BOOT START ~ QUERY DEVICE
    eBOOT_CONNECT , // ~ COMPARE ACL    
    eBOOT_JOIN , // ~ New Node Joining Case 
    eBOOT_ACTIVE , // ~  (RUN )
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
	
	eJOIN_stEND,

}BOOTSUB_JOIN_E;

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
	eSENSING_st1_REQ, //REQUEST SENSING
	eSENSING_st1_RES, //RESPONSE
}MAINSUB_SENSING_E;

typedef enum
{
	eBALANCING_st0_IDLE,
	eBALANCING_st1_REQ, //REQUEST BALANCING
	eBALANCING_st1_RES, //RESPONSE
}MAINSUB_BALANCING_E;



void CmicM_Init(void);
void CmicM_Handler(void);


adi_wil_err_t Cmic_RequestLoadFileConfig(adi_wil_device_t eDevice);

bool Cmic_IsDualConfig(adi_wil_configuration_t * C1, adi_wil_configuration_t * C2);
bool Cmic_CompareACL(void);
uint8 Cmic_ConvertDeviceId (adi_wil_device_t WilDeviceId);
void Cmic_ReadInitPacket(void);
void Cmic_ReadBMS(void);
void Cmic_SaveLatencyPkt(void);

#endif /* CMICM_H_ */

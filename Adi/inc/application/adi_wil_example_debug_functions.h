/**
 * @brief   Custom functions for debugging targeted for IFX AURIX TC387
 * @author  ADK SW team
 * @date    2022-06-17
 */

#include <stdint.h>
#include <stdbool.h>
#include "Ifx_Types.h"
#include "IfxPort.h"

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/
#define DBG_GPIO_0              &MODULE_P34,1                   /* @remark : GPIO for debugging purpose */ 
#define DBG_GPIO_1              &MODULE_P34,3                   /* @remark : GPIO for debugging purpose */ 
#define DBG_GPIO_2              &MODULE_P34,2                   /* @remark : GPIO for debugging purpose */ 
#define DBG_GPIO_3              &MODULE_P34,4                   /* @remark : GPIO for debugging purpose */ 
#define DBG_GPIO_4              &MODULE_P15,2                   /* @remark : GPIO for debugging purpose */ 
#define DBG_GPIO_5              &MODULE_P15,8                   /* @remark : GPIO for debugging purpose */ 

#define DBG_TEMP_TEST           /* TEMP raw data 0 test */
#define DBG_ACL_MAP_TEST        /* Monitoring userAcl and realAcl mapping information (userAcl_map, realAcl_map) */
#define DBG_INIT_SCRIPT_TEST    /* Test for Init script */
//#define DBG_PACKET_ID_TEST      /* collect packet ID when readBMS */
//#define DBG_GET_FILE_TEST       /* Test for GetFile API */

#define Overall_ true
#define Interval false
#define LogStart true
#define LogEnd__ false
#define ON true
#define OFF false

/*******************************************************************************
 * Externs                                                                     *
 *******************************************************************************/
extern void DBG_GPIO_INIT(void);
extern void DBG_GPIO_HIGH(uint8_t ch);
extern void DBG_GPIO_LOW(uint8_t ch);
extern void DBG_GPIO_SPIKE_UP(uint8_t ch);
extern void DBG_GPIO_SPIKE_DOWN(uint8_t ch);

/* Calling examples */
// DBG_GPIO_HIGH(ch);       /* @remark  : debugging purpose */
// DBG_GPIO_LOW(ch);        /* @remark  : debugging purpose */
// DBG_GPIO_SPIKE_UP(ch);   /* @remark  : debugging purpose */
// DBG_GPIO_SPIKE_DOWN(ch); /* @remark  : debugging purpose */

/*******************************************************************************
 * Project configuration                                                       *
 *******************************************************************************/
#define ADK_SPI_0       4 //1   /* SPI0_MASTER : 4 = QSPI4, Others = QSPI1 */
#define ADK_SPI_1       0 //3   /* SPI1_MASTER : 0 = QSPI0, Others = QSPI3 */
#define ADK_SPI_SPEED   1   /* Reserved, Now SPI speed has been fixed to 1Mhz */
#define ADK_ADBMS683x   3 //0   /* 0 = 6830, 3 = 6833, Others = Build errors */
#define ADK_SPI_P20     OFF /* OFF = default(TC387), ON = change SPI config to P20 */

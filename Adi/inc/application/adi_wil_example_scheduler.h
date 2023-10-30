/*******************************************************************************
* @file     adi_wil_example_scheduler.h
*
* @brief    Header for scheduler tasks 
*
* @details  Contains exported function prototypes
*
* Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef __ADI_EXAMPLE_SCHDLR__
#define __ADI_EXAMPLE_SCHDLR__

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/
/**
 * @remark : Scheduler select
 */
#define SCHDLR_1MS_TABLE_SIZE           0U
#define SCHDLR_2MS_TABLE_SIZE           0U
#define SCHDLR_5MS_TABLE_SIZE           0U
#define SCHDLR_10MS_TABLE_SIZE          0U
#define SCHDLR_25MS_TABLE_SIZE          0U
#define SCHDLR_50MS_TABLE_SIZE          0U
#define SCHDLR_100MS_TABLE_SIZE         0U
#define SCHDLR_1000MS_TABLE_SIZE        0U
#define SCHDLR_60000MS_TABLE_SIZE       1U

#define SCHDLR_1MS_INTERVAL             1U
#define SCHDLR_2MS_INTERVAL             2U
#define SCHDLR_5MS_INTERVAL             5U
#define SCHDLR_10MS_INTERVAL            10U
#define SCHDLR_25MS_INTERVAL            25U
#define SCHDLR_50MS_INTERVAL            50U
#define SCHDLR_100MS_INTERVAL           100U
#define SCHDLR_1000MS_INTERVAL          1000U
#define SCHDLR_60000MS_INTERVAL         60000U

#define SCHDLR_MAXSLOTS                 10              /* Maximum number of time based schedules */
#define SCHDLR_TIMER_1MS_INTERVAL       1000U           /* 1 millisecond */

/*******************************************************************************
 * Structures
 *******************************************************************************/
typedef struct
{
    uint8_t TickDelay;
    void (*TaskPtr)(void);
}SchedulerEntry_t;

typedef struct
{
    uint32_t timeSlot;
    SchedulerEntry_t *timeSlotEntry;
    uint32_t currentTableSize;
}SchedulerSlots_t;


/*******************************************************************************
 * Functions
 *******************************************************************************/
/************************************************************************************
* adi_wil_example_scheduleTasks
*
* Scheduler Function (with an infinite While loop).
* Traverses the available scheduler table and invokes the function when the respective period expires.
*
* @arguments :None
*
* @return none
 ***********************************************************************************/
void adi_wil_example_scheduleTasks(void);

/************************************************************************************
 * adi_task_bmsDataRetrieval
 *
 * Task for BMS Data retrieval
 *
* @arguments : none
*
* @return none
 ***********************************************************************************/
void adi_task_bmsDataRetrieval(void);

/************************************************************************************
 * adi_task_pmsDataRetrieval
 *
 * Task for PMS Data retrieval
 *
* @arguments : none
*
* @return none
 ***********************************************************************************/
void adi_task_pmsDataRetrieval(void);

/************************************************************************************
 * adi_task_emsDataRetrieval
 *
 * Task for EMS Data retrieval
 *
* @arguments : none
*
* @return none
 ***********************************************************************************/
void adi_task_emsDataRetrieval(void);

/************************************************************************************
 * adi_task_processEvent
 *
 * Task for processing the BMS / PMS / EMS events
 *
* @arguments : none
*
* @return none
 ***********************************************************************************/
void adi_task_processEvent(void);

/************************************************************************************
 * adi_task_spiPortStatisticsRetrieval
 *
 * Task for printing SPI port statistics over V71 debug UART
 *
* @arguments : none
*
* @return none
 ***********************************************************************************/
void adi_task_spiPortStatisticsRetrieval(void);

/************************************************************************************
 * adi_wil_example_SchedulerInit
 *
 * Timer_channel 0  Initialization for scheduler
 *
* @arguments : none
*
* @return Scheduler Init Status : true or false 
 ***********************************************************************************/
bool adi_wil_example_SchedulerInit(void);


/**
 * @remark : select script example state machine
 */
void adi_example_select_script_statemachine(void);


/**
 * @remark : Key on event process
 */
void adi_example_key_on_event(void);


/**
 * @remark: null function for test
 */

void adi_example_null_function(void);

#endif

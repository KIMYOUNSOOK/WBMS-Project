/*******************************************************************************
* @file     adi_wil_example_scheduler.c
*
* @brief    Non-preemptive scheduler
*
* @details  Contains scheduler table and scheduling mechanism.
*
* Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#include <stdint.h>
#include "adi_wil_example_scheduler.h"
#include "adi_wil_example_utilities.h"
#include "adi_wil_example_owd.h"
#include "STM_Interrupt.h"
#include "Bsp.h"
#include "IfxStm.h"

#include "adi_wil_example_debug_functions.h"

#define STM                     &MODULE_STM0                    /* STM0 is used in this example                     */

/*******************************************************************************/
/* Local MACRO Declarations                                                    */
/*******************************************************************************/

/*******************************************************************************/
/* User Defined Type                                                           */
/*******************************************************************************/

/*******************************************************************************/
/* Global Variable Declarations                                                */
/*******************************************************************************/

extern IfxStm_CompareConfig g_STMConf;                               /* STM configuration structure                      */
extern Ifx_TickTime g_ticksFor1ms;                                   /* Variable to store the number of ticks to wait    */
extern unsigned int Schdlr_CurrentMsCount;                           /* @remark  : static removed */  
extern bool Schdlr_1ms_tick;                                         /* @remark  : static removed */  
uint32_t DBG_main_idle = 0;

/*******************************************************************************/
/* Local Variable Declarations                                                 */
/*******************************************************************************/
static void Scheduler_ExecuteTasks(uint32_t iTaskSlot);
extern void Scheduler_1ms_Trigger(void);

/* Scheduler 1ms Tasks */
#if SCHDLR_1MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_1ms[SCHDLR_1MS_TABLE_SIZE] =
{
    {0,adi_wil_example_w2canBmsMsgHandler},
    /* add more tasks here */
};
#endif

/* Scheduler 2ms Tasks */
#if SCHDLR_2MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_2ms[SCHDLR_2MS_TABLE_SIZE] =
{
    /* add more tasks here */
};
#endif

/* Scheduler 5ms Tasks */
#if SCHDLR_5MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_5ms[SCHDLR_5MS_TABLE_SIZE] =
{
    /* add more tasks here */
};
#endif

/* Scheduler 10ms Tasks */
#if SCHDLR_10MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_10ms[SCHDLR_10MS_TABLE_SIZE] =
{
    {0,adi_task_pmsDataRetrieval},
    {1,adi_wil_example_w2canHealthReportHandler}
    /* add more tasks here */
};
#endif

/* Scheduler 25ms Tasks */
#if SCHDLR_25MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_25ms[SCHDLR_25MS_TABLE_SIZE] =
{
    {0,adi_wil_example_w2canPmsMsgHandler},
    /* add more tasks here */
};
#endif

/* Scheduler 50ms Tasks */
#if SCHDLR_50MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_50ms[SCHDLR_50MS_TABLE_SIZE] =
{
    /* add more tasks here */
};
#endif

/* Scheduler 100ms Tasks */
#if SCHDLR_100MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_100ms[SCHDLR_100MS_TABLE_SIZE] =
{
    {0, adi_example_select_script_statemachine},
    /* add more tasks here */
};
#endif

/* Scheduler 1000ms Tasks */
#if SCHDLR_1000MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_1000ms[SCHDLR_1000MS_TABLE_SIZE] =
{
    /* add more tasks here */
};
#endif

/* Scheduler 60000ms Tasks */
#if SCHDLR_60000MS_TABLE_SIZE > 0
SchedulerEntry_t SchedulerTable_60000ms[SCHDLR_60000MS_TABLE_SIZE] =
{
    {0, adi_example_null_function},
    /* add more tasks here */
};
#endif

/* 
 * Struct array -> {arrayindex1{timeslot(uint32), {tickdelay(uint8), ptr(void*)}, currenttablesize(uint32)}, arrayindex2{....},...upto SCHDLR_MAXSLOTS}
*/
SchedulerSlots_t SchedulerTickSlots[SCHDLR_MAXSLOTS] = 
{
    #if SCHDLR_1MS_TABLE_SIZE > 0
    {SCHDLR_1MS_INTERVAL, &SchedulerTable_1ms[0], SCHDLR_1MS_TABLE_SIZE},
    #endif

    #if SCHDLR_2MS_TABLE_SIZE > 0
    {SCHDLR_2MS_INTERVAL, &SchedulerTable_2ms[0], SCHDLR_2MS_TABLE_SIZE},
    #endif

    #if SCHDLR_5MS_TABLE_SIZE > 0
    {SCHDLR_5MS_INTERVAL, &SchedulerTable_5ms[0], SCHDLR_5MS_TABLE_SIZE},
    #endif

    #if SCHDLR_10MS_TABLE_SIZE > 0
    {SCHDLR_10MS_INTERVAL, &SchedulerTable_10ms[0], SCHDLR_10MS_TABLE_SIZE},
    #endif

    #if SCHDLR_25MS_TABLE_SIZE > 0
    {SCHDLR_25MS_INTERVAL, &SchedulerTable_25ms[0], SCHDLR_25MS_TABLE_SIZE},
    #endif
    
     #if SCHDLR_50MS_TABLE_SIZE > 0
    {SCHDLR_50MS_INTERVAL, &SchedulerTable_50ms[0], SCHDLR_50MS_TABLE_SIZE},
    #endif

    #if SCHDLR_100MS_TABLE_SIZE > 0
    {SCHDLR_100MS_INTERVAL, &SchedulerTable_100ms[0], SCHDLR_100MS_TABLE_SIZE},
    #endif

    #if SCHDLR_1000MS_TABLE_SIZE > 0
    {SCHDLR_1000MS_INTERVAL, &SchedulerTable_1000ms[0], SCHDLR_1000MS_TABLE_SIZE},
    #endif

    #if SCHDLR_60000MS_TABLE_SIZE > 0
    {SCHDLR_60000MS_INTERVAL, &SchedulerTable_60000ms[0], SCHDLR_60000MS_TABLE_SIZE},
    #endif
};

/*******************************************************************************/
/* Functions Definitions                                                       */
/*******************************************************************************/

/************************************************************************************
 * adi_wil_example_scheduleTasks
 *
 * Scheduler Function (with an infinite While loop).
 * Traverses the Available Tables and invokes the function when the respective period expires.
 *
 * @arguments :None
 *
 * @return none
***********************************************************************************/
void adi_wil_example_scheduleTasks(void)
{
    while(1)
    {
        DBG_main_idle++; /* @remark: test purpose */
        adi_example_select_script_statemachine();
        if(true == Schdlr_1ms_tick)
        {
            Schdlr_1ms_tick = false;

            /* 1ms Task : start */
            #if SCHDLR_1MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_1MS_INTERVAL);
            #endif
            /* 1ms Task : end */

            /*2ms Task : Start */
            #if SCHDLR_2MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_2MS_INTERVAL);
            #endif
            /*2ms Task : end */

            /*5ms Task : Start */
            #if SCHDLR_5MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_5MS_INTERVAL);
            #endif
            /*5ms Task : end */

            /*10ms Task : Start */
            #if SCHDLR_10MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_10MS_INTERVAL);
            #endif
            /*10ms Task : end */

            /*25ms Task : Start */
            #if SCHDLR_25MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_25MS_INTERVAL);
            #endif
            /*25ms Task : end */
            
            /*50ms Task : Start */
            #if SCHDLR_50MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_50MS_INTERVAL);
            #endif
            /*50ms Task : end */
            
            /* 100ms Task : start*/
            #if SCHDLR_100MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_100MS_INTERVAL);
            #endif
            /* 100ms Task : end*/

            /* 1000ms Task : start*/
            #if SCHDLR_1000MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_1000MS_INTERVAL);
            #endif
            /* 1000ms Task : end*/

            /* 60000ms Task : start*/
            #if SCHDLR_60000MS_TABLE_SIZE > 0
            Scheduler_ExecuteTasks(SCHDLR_60000MS_INTERVAL);
            #endif
            /* 60000ms Task : end*/
        }
        else {
            /* Do nothing till timer triggered */
        }
    }
}
/************************************************************************************
 * Scheduler_ExecuteTasks
 *
 * Generic function to execute all the tasks of provided Slot
 *
 * @arguments : Time Slot : 1,2,5,100ms .. etc.
 *
 * @return none
 ***********************************************************************************/
static void Scheduler_ExecuteTasks(uint32_t iTaskInterval)
{
    uint32_t iCurrentDelayMillisec = 0;
    uint32_t iTaskTableIdx;
    uint32_t iSlotIdx = 0;

    iCurrentDelayMillisec = Schdlr_CurrentMsCount%iTaskInterval;

    /* find the scheduler slot index , eg:- 1ms or 10ms or 100ms.... */
    for(iTaskTableIdx = 0; iTaskTableIdx < SCHDLR_MAXSLOTS; iTaskTableIdx++)
    {
        if(iTaskInterval == SchedulerTickSlots[iTaskTableIdx].timeSlot)
        {
            iSlotIdx = iTaskTableIdx;
            for(iTaskTableIdx = 0; iTaskTableIdx < SchedulerTickSlots[iSlotIdx].currentTableSize; iTaskTableIdx++)
            {
                if(iCurrentDelayMillisec == SchedulerTickSlots[iSlotIdx].timeSlotEntry[iTaskTableIdx].TickDelay)
                    {
                        /* time to invoke the task, if the function pointer is not NULL */
                        if(NULL != SchedulerTickSlots[iSlotIdx].timeSlotEntry[iTaskTableIdx].TaskPtr)
                            {
                                SchedulerTickSlots[iSlotIdx].timeSlotEntry[iTaskTableIdx].TaskPtr();
                                }
                            }
                    }
            break;
        }
    }
}

/************************************************************************************
 * adi_wil_example_SchedulerInit
 *
 * Initializes the Scheduler Tick (timer channel 0, with 1ms timer callback)
 *
 * @arguments : none
 *
 * @return Scheduler Init Status : true or false 
 ***********************************************************************************/
bool adi_wil_example_SchedulerInit(void)
{
    initPeripherals();
    return true;
}

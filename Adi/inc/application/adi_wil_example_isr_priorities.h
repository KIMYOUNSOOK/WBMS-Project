/*******************************************************************************
* @brief    Priorities for all application ISRs
*
* @details  On TC275, every ISR which will be serviced by a CPU must be assigned a priority. 
*           Priority is an 8-bit integer, higher number assigns higher priority.
*           Typically, each ISR should have a unique priority. To facilitate organizing
*           priorities for whole application, priorities are defined in one location
*           to make cross-reference easier.
* 
*           Also note that documentation warns to not surround definitions with brackets.
*           That is,  #define PRIORITY 10  is ok but  #define PRIORITY (10)  is not.
*           (The brackets break the platform macros)    
*  
*           For more information, see online documentation at
*           https://www.infineon.com/dgdl/Infineon-AURIX_Interrupt_Router-Training-v01_00-EN.pdf?fileId=5546d46269bda8df0169ca77743b2558
*
* Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#include "adi_wil_example_debug_functions.h"

#ifndef ADI_WIL_EXAMPLE_ISR_PRIORITIES_H
#define ADI_WIL_EXAMPLE_ISR_PRIORITIES_H

/*******************************************************************************
 * #defines                                                                    *
 *******************************************************************************/

#define ISR_PRIORITY_ASCLIN3_RX  1  /* Interrupt priority for ASCLIN Receive ISR */
#define ISR_PRIORITY_ASCLIN3_TX  2  /* Interrupt priority for ASCLIN Transmit ISR */
#define ISR_PRIORITY_ASCLIN3_ER  3  /* Interrupt priority for ASCLIN Error ISR */

#if(ADK_SPI_0 == 4)
#define ISR_PRIORITY_QSPI4_TX    12 /* Define the QSPI1 transmit interrupt priority */
#define ISR_PRIORITY_QSPI4_RX    13 /* Define the QSPI1 receive interrupt priority */
#define ISR_PRIORITY_QSPI4_ER    10 /* Define the QSPI1 error interrupt priority */
#else
#define ISR_PRIORITY_QSPI1_TX    12 /* Define the QSPI1 transmit interrupt priority */
#define ISR_PRIORITY_QSPI1_RX    13 /* Define the QSPI1 receive interrupt priority */
#define ISR_PRIORITY_QSPI1_ER    10 /* Define the QSPI1 error interrupt priority */
#endif

#if(ADK_SPI_1 == 0)
#define ISR_PRIORITY_QSPI0_TX    14 /* Define the QSPI0 transmit interrupt priority */
#define ISR_PRIORITY_QSPI0_RX    15 /* Define the QSPI0 receive interrupt priority */
#define ISR_PRIORITY_QSPI0_ER    11 /* Define the QSPI0 error interrupt priority */
#else
#define ISR_PRIORITY_QSPI3_TX    14 /* Define the QSPI3 transmit interrupt priority */
#define ISR_PRIORITY_QSPI3_RX    15 /* Define the QSPI3 receive interrupt priority */
#define ISR_PRIORITY_QSPI3_ER    11 /* Define the QSPI3 error interrupt priority */
#endif

#define ISR_PRIORITY_DMA_CH1     50 /* Define the DMA channel1 interrupt priority */
#define ISR_PRIORITY_DMA_CH2     51 /* Define the DMA channel2 interrupt priority */
#define ISR_PRIORITY_DMA_CH3     52 /* Define the DMA channel3 interrupt priority */
#define ISR_PRIORITY_DMA_CH4     53 /* Define the DMA channel4 interrupt priority */

#define ISR_PRIORITY_HAL_TASK    46 /* Priority for HAL_TASK interrupt */
#define ISR_PRIORITY_HAL_TASK_CB 35 /* Priority for HAL_TASK_CB interrupt */

#define ISR_PRIORITY_HAL_TMR     41 /* Priority for HAL TMR interrupt */


#endif  /*  ADI_WIL_EXAMPLE_ISR_PRIORITIES_H  */

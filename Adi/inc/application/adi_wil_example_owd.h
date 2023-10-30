/*******************************************************************************
 * @file adi_wil_example_owd.h
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef ADI_WIL_EXAMPLE_OWD_H_
#define ADI_WIL_EXAMPLE_OWD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adi_wil.h"
#include "adi_wil_types.h"
#include "adi_wil_example_debug_functions.h"

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
/* Total scripts for ADBMS6830 Saftey Measures */
#define SM_ADBMS6830_TOTAL_SCRIPTS                          (2U)
/* Max retries for adi_wil_SelectScript API incase of failure */
#define SM_ADBMS6830_SELECT_SCRIPT_MAX_RETRIES              (2U)
/* Macros for SM status */
#define SM_ADBMS6830_STATUS_PASS                            (0U)
#define SM_ADBMS6830_STATUS_FAIL                            (1U)
/* Macros to define total Cell data per device */
#define SM_ADBMS6830_TOTAL_CELLS_PER_DEVICE                 (16U)
/* Macros for voltage resolution and offset */
#define SM_ADBMS6830_VTG_REG_RESOLUTION                     (150U)
#define SM_ADBMS6830_VTG_REG_OFFSET                         (1500000U)
/* Macros to identify the packets received for open wire detection */
#define SM_ADBMS6830_C_CH_EVEN_RCVD                         (0x01U)
#define SM_ADBMS6830_S_CH_EVEN_RCVD                         (0x02U)
#define SM_ADBMS6830_C_CH_ODD_RCVD                          (0x04U)
#define SM_ADBMS6830_S_CH_ODD_RCVD                          (0x08U)
/* Macro to indicate that all packets for open wire detection are received */
#define SM_ADBMS6830_ALL_PCKTS_FOR_OWD_RCVD                 (0x7U)
/* Threshold for Open Wire detection. It indicates the percentage of allowable drop in cell voltage */
#define SM_ADBMS6830_OW_THRESHOLD_PERCENT                   (25)
/* Open Wire detection under -0.3 vol(-12000) */
#define SM_ADBMS6830_OW_ZERO_VOL                            (-12000)

#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
/* Total scripts for ADBMS6833 Saftey Measures */
#define SM_ADBMS6833_TOTAL_SCRIPTS                          (2U)
/* Max retries for adi_wil_SelectScript API incase of failure */
#define SM_ADBMS6833_SELECT_SCRIPT_MAX_RETRIES              (2U)
/* Macros for SM status */
#define SM_ADBMS6833_STATUS_PASS                            (0U)
#define SM_ADBMS6833_STATUS_FAIL                            (1U)
/* Macros to define total Cell data per device */
#define SM_ADBMS6833_TOTAL_CELLS_PER_DEVICE                 (16U)
/* Macros for voltage resolution and offset */
#define SM_ADBMS6833_VTG_REG_RESOLUTION                     (150U)
#define SM_ADBMS6833_VTG_REG_OFFSET                         (1500000U)
/* Macros to identify the packets received for open wire detection */
#define SM_ADBMS6833_C_CH_EVEN_RCVD                         (0x01U)
#define SM_ADBMS6833_S_CH_EVEN_RCVD                         (0x02U)
#define SM_ADBMS6833_C_CH_ODD_RCVD                          (0x04U)
#define SM_ADBMS6833_S_CH_ODD_RCVD                          (0x08U)
/* Macro to indicate that all packets for open wire detection are received */
#define SM_ADBMS6833_ALL_PCKTS_FOR_OWD_RCVD                 (0x7U)
/* Threshold for Open Wire detection. It indicates the percentage of allowable drop in cell voltage */
#define SM_ADBMS6833_OW_THRESHOLD_PERCENT                   (30)
/* Open Wire detection under -0.3 vol(-12000) */
#define SM_ADBMS6833_OW_ZERO_VOL                            (-12000)

#else   /* Not supported */
#endif


/******************************************************************************
 * Function Declarations
 *****************************************************************************/
extern void adi_wil_example_ExecuteOWD(void);


#endif /* ADI_WIL_EXAMPLE_OWD_H_ */


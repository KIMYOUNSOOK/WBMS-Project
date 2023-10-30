/*******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_WIL_CFG_ADK_H_
#define ADI_WIL_CFG_ADK_H_

#include <stdint.h>
#include "adi_wil_example_debug_functions.h"

#if     (ADK_ADBMS683x == 0) /* ADBMS6830 */
#define ADI_WIL_CFG_ADK_CRC                        2966953970u
#elif   (ADK_ADBMS683x == 3) /* ADBMS6833 */
#define ADI_WIL_CFG_ADK_CRC                        3299112295u
#else   /* Not supported */
#endif


void adi_wil_cfg_ADK_GetConfigurationPtr(uint8_t **pConfiguration, uint32_t *pConfiguration_len);

extern const uint8_t configuration_file_configuration[];
extern const unsigned long configuration_file_configuration_termination;
extern const unsigned long configuration_file_configuration_start;
extern const unsigned long configuration_file_configuration_finish;
extern const unsigned long configuration_file_configuration_length;

#endif  /* ADI_WIL_CFG_ADK_H_ */


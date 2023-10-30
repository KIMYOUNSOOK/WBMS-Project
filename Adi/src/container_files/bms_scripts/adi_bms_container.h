/*******************************************************************************
* Copyright (c) 2020 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_BMS_CONTAINER_H_
#define ADI_BMS_CONTAINER_H_

#include <stdint.h>

void adi_bms_GetContainerPtr(uint8_t **pContainer, uint32_t *pContainer_len);

void adi_bms_GetMonitorParamsPtr(uint8_t **pMonitorParams, uint32_t *pMonitorParams_len);

#endif  /* ADI_BMS_CONTAINER_H_ */

/******************************************************************************
* @file    wb_wil_xms_type.h
*
* @brief   Definition of adi_wil_xms_type_t enum
*
* Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef WB_XMS_TYPE__H
#define WB_XMS_TYPE__H

#include <stdint.h>

/******************************************************************************
 * Typedefs
 *****************************************************************************/

/**
 * @brief   Types of XMS data.
 */
typedef enum
{
    ADI_WIL_XMS_BMS,
    ADI_WIL_XMS_PMS,
    ADI_WIL_XMS_EMS
} adi_wil_xms_type_t;

/**
 * @brief   Stages of XMS measurement.
 */
typedef enum
{
    ADI_WIL_XMS_START_MEASUREMENT,
    ADI_WIL_XMS_SUPPLEMENTAL_MEASUREMENT
} adi_wil_xms_measurement_cmd_id_t;

/**
 * @brief   XMS metadata.
 */
typedef struct
{
    adi_wil_xms_type_t eType;
    adi_wil_xms_measurement_cmd_id_t eCmdId;
} wb_xms_metadata_t;

#endif  //WB_XMS_TYPE__H

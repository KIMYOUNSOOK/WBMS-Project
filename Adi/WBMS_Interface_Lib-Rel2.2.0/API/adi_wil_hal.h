/******************************************************************************
 * @file     adi_wil_hal.h
 *
 * @brief    WBMS Interface Library common HAL definitions.
 *
 * @details  Contains API definitions that is common to all HAL functions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_HAL__H
#define ADI_WIL_HAL__H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Type defines
 *****************************************************************************/
/**
 * @brief   HAL error codes
 */
typedef enum {
    ADI_WIL_HAL_ERR_SUCCESS,		    /*!< Operation successful */
    ADI_WIL_HAL_ERR_INVALID_PARAM,      /*!< Invalid parameter given */
    ADI_WIL_HAL_ERR_NO_RESOURCES,       /*!< No resources to carry out the operation */
    ADI_WIL_HAL_ERR_FAILURE             /*!< Operation failed */
} adi_wil_hal_err_t;

#ifdef __cplusplus
}
#endif
#endif   // #ifndef ADI_WIL_HAL__H

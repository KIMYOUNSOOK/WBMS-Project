/******************************************************************************
 * @file     adi_wil_safety_internals.h
 *
 * @brief    WIL FE internals structure definitions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_SAFETY_INTERNALS__H
#define ADI_WIL_SAFETY_INTERNALS__H

#include "adi_wil_types.h"
#include "adi_wil_api_internals.h"
#include "adi_wil_assl_internals.h"
#include "adi_wil_ui_internals.h"
#include "adi_wil_xms_internals.h"
#include "adi_wil_xms_parameters.h"

/******************************************************************************
 * Structure Definitions
 *****************************************************************************/

/**
 * @brief   ASSL Internals structure
 */
struct adi_wil_safety_internals_t {
    adi_wil_api_internals_t API;                                       /*!< Structure for storing parameters used by API modules */
    adi_wil_assl_internals_t ASSL;                                     /*!< Internals structure for ASSL module */
    adi_wil_ui_internals_t UI;                                         /*!< Internals structure for UI module */
    adi_wil_xms_internals_t XMS;                                       /*!< Internals structure for XMS module */
    adi_wil_xms_parameters_t XMSParameters;                            /*!< Structure containing non-FuSa XMS buffer parameters */
    adi_wil_xms_parameters_t FuSaXMSParameters;                        /*!< Structure containing FuSa XMS buffer parameters */    
    adi_wil_pack_t const * pPack;                                      /*!< Pointer to pack structure */
};

#endif //ADI_WIL_SAFETY_INTERNALS__H

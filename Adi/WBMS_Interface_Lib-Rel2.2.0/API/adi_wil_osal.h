/******************************************************************************
 * @file     adi_wil_osal.h
 *
 * @brief    WBMS Interface Library OSAL definitions and declarations.
 *
 * @details  Contains API declarations for the OSAL functions.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_OSAL__H
#define ADI_WIL_OSAL__H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Type defines
 *****************************************************************************/

/**
 * @brief   OSAL error codes
 */
typedef enum {
    ADI_WIL_OSAL_ERR_SUCCESS,           /*!< OSAL operation successful */
    ADI_WIL_OSAL_ERR_FAIL,              /*!< OSAL operation failed */
    ADI_WIL_OSAL_ERR_NO_RESOURCE        /*!< Requested resource does not exist */
} adi_wil_osal_err_t;


/******************************************************************************
 * Function Declarations
 *****************************************************************************/

/**
 * @brief   Create and initialize a locking resource.
  *
 * @details This function creates and initializes a locking resource. The resource
 *          is identified by the argument pPack.
 *
 * @param  pPack                The address of the pack is creating the resource.
 *                              This argument is used to identify the resource as
 *                              well as the pack that is creating the resource.
 *
 * @return adi_wil_osal_err_t   Error code of the creation/initialization.
 *                              Return ADI_WIL_OSAL_ERR_SUCCESS if creation successful,
 *                              ADI_WIL_OSAL_ERR_FAIL otherwise.
 */
adi_wil_osal_err_t adi_wil_osal_CreateResource(void const * const pPack);


/**
 * @brief   Acquire a locking resource.
  *
 * @details This function acquires a resource with the ID pPack.
 *
 * @param  pPack                The address of the pack is acquiring the resource.
 *                              This argument is used to identify the resource as
 *                              well as the pack that is acquiring the resource.
 *
 * @return adi_wil_osal_err_t   Result from the resource acquisition.
 *                              Return ADI_WIL_OSAL_ERR_SUCCESS if resource acquired
 *                              within the given waiting period, ADI_WIL_OSAL_ERR_FAIL
 *                              otherwise. If the requested resource does not exist,
 *                              ADI_WIL_OSAL_ERR_NO_RESOURCE is returned.
 */
adi_wil_osal_err_t adi_wil_osal_AcquireResource(void const * const pPack);


/**
 * @brief   Release a locking resource.
  *
 * @details This function release a resource with the ID pPack that was acquired by
 *          calling adi_wil_osal_AcquireResource.
 *
 * @param  pPack                The address of the pack is releasing the resource.
 *                              This argument is used to identify the resource as
 *                              well as the pack that is releasing the resource.
 *
 * @return adi_wil_osal_err_t   Result from the resource release.
 *                              Return ADI_WIL_OSAL_ERR_SUCCESS if the resource is
 *                              released ADI_WIL_OSAL_ERR_FAIL otherwise.
 *                              If the referred resource does not exist,
 *                              ADI_WIL_OSAL_ERR_NO_RESOURCE is returned.
 */
 adi_wil_osal_err_t adi_wil_osal_ReleaseResource(void const * const pPack);


/**
 * @brief   Destroy a resource.
  *
 * @details This function destroys the resource with ID specified by pPack.
 *
 * @param  pPack                The ID of the resource indicating the resource to destroy.
 *                              This argument is used to identify the resource as
 *                              well as the pack that is requesting to destroy the resource.
 *
 * @return adi_wil_osal_err_t   Result from the resource destruction.
 *                              Return ADI_WIL_OSAL_ERR_SUCCESS if the resource has
 *                              been destroyed, ADI_WIL_OSAL_ERR_FAIL otherwise.
 *                              If the referred the resource does not exist,
 *                              ADI_WIL_OSAL_ERR_NO_RESOURCE is returned.
 */
adi_wil_osal_err_t adi_wil_osal_DestroyResource(void const * const pPack);

#ifdef __cplusplus
}
#endif
#endif   // #ifndef ADI_WIL_OSAL__H

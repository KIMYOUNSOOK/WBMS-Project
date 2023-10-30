/******************************************************************************
 * @file    wb_wil_configure_cell_balancing.c
 *
 * @brief   Configure cell balancing on a safety MCU
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_configure_cell_balancing.h"
#include "wb_wil_ui_fusa.h"
#include "wb_assl_fusa.h"
#include "wb_wil_device.h"
#include "wb_scl_msg_defs.h"
#include <string.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

/* Number of retries before the request is aborted */
#define ADI_WIL_CELL_BALANCING_RETRIES (2u)

/* Time in milliseconds before request is retried */
#define ADI_WIL_CELL_BALANCING_TIMEOUT (300u)

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static bool adi_wil_ValidateConfigureCellBalancingParams (adi_wil_safety_internals_t const * const pInternals,
                                                          uint64_t iDeviceId,
                                                          adi_wil_ddc_t const * const pDischargeDutyCycle);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_ConfigureCellBalancingAPI (adi_wil_safety_internals_t * const pInternals,
                                                uint64_t iDeviceId,
                                                adi_wil_ddc_t const * const pDischargeDutyCycle,
                                                uint16_t iDuration,
                                                uint32_t iUVThreshold)
{
    /* Return value of the function */
    adi_wil_err_t rc;

    /* TODO: Remove this when 64-bit support is enabled in ASSL */
    uint8_t iASSLDeviceId;

    /* Initialize 8-bit Device ID variable to 0 */
    iASSLDeviceId = 0u;

    /* Validate the input parameter */
    if (!adi_wil_ValidateConfigureCellBalancingParams (pInternals,
                                                       iDeviceId,
                                                       pDischargeDutyCycle))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Attempt to acquire the lock */
        rc = wb_wil_ui_AcquireFuSaLock (pInternals->pPack, pInternals->pPack);

        /* If we were able to acquire the lock, proceed... */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Store the requested node in the API requested node map */
            pInternals->API.iRequestedNodes = iDeviceId;

            /* TODO: Remove this when 64-bit support is enabled in ASSL */
            while (((iDeviceId >> iASSLDeviceId) & 1ULL) != 1ULL)
            {
                iASSLDeviceId++;
            }

            /* Attempt to write the request to the buffer */
            rc = wb_assl_WriteConfigureCellBalancingFrame (pInternals,
                                                           iASSLDeviceId,
                                                           pDischargeDutyCycle,
                                                           iDuration,
                                                           iUVThreshold);

            /* If we were able to write the request it is ready to go out so
             * mark as active */
            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                /* Activate the request with the defined number of retries and
                 * timeout values */
                rc = wb_assl_ActivateRequest (pInternals,
                                              ADI_WIL_API_CONFIGURE_CELL_BALANCING,
                                              ADI_WIL_CELL_BALANCING_RETRIES,
                                              ADI_WIL_CELL_BALANCING_TIMEOUT);
            }

            /* Release the lock if either the writing or activation steps
             * failed */
            if (ADI_WIL_ERR_SUCCESS != rc)
            {
                wb_wil_ui_ReleaseFuSaLock (pInternals->pPack, pInternals->pPack);
            }
        }
    }

    return rc;
}

void wb_wil_HandleConfigureCellBalancingResponse (adi_wil_safety_internals_t * const pInternals,
                                                  uint8_t iDeviceId,
                                                  adi_wil_err_t rc)
{
    /* Validate pointer before dereferencing */
    if (pInternals != NULL)
    {
        /* Validate device ID is within range */
        if (iDeviceId < ADI_WIL_MAX_NODES)
        {
            /* Clear the responding node from the requested node bitmap */
            pInternals->API.iRequestedNodes &= ~(1ULL << iDeviceId);
        }

        /* Check if we've received all responses from all requested nodes */
        if (pInternals->API.iRequestedNodes == 0u)
        {
            /* Deactivate the request when all responses have been received */
            wb_assl_DeactivateRequest (pInternals);

            /* Generate callback and release lock */
            wb_wil_ui_GenerateFuSaCb (pInternals->pPack, ADI_WIL_API_CONFIGURE_CELL_BALANCING, rc, NULL);
            wb_wil_ui_ReleaseFuSaLock (pInternals->pPack, pInternals->pPack);
        }
    }
}

void wb_wil_TimeoutConfigureCellBalancing (adi_wil_safety_internals_t const * const pInternals)
{
    /* Validate pointer before dereferencing */
    if (pInternals != NULL)
    {
        /* Generate callback and release lock */
        wb_wil_ui_GenerateFuSaCb (pInternals->pPack, ADI_WIL_API_CONFIGURE_CELL_BALANCING, ADI_WIL_ERR_TIMEOUT, NULL);
        wb_wil_ui_ReleaseFuSaLock (pInternals->pPack, pInternals->pPack);
    }
}

static bool adi_wil_ValidateConfigureCellBalancingParams (adi_wil_safety_internals_t const * const pInternals,
                                                          uint64_t iDeviceId,
                                                          adi_wil_ddc_t const * const pDischargeDutyCycle)
{
    /* Validate the following clauses:
     * - pInternals is not NULL
     * - pDischargeDutyCycle is not NULL
     * - The 64-bit Device ID is not 0
     * - The 64-bit Device ID is a valid node
     * - There is only one bit set in the Device ID */
    return ((pInternals != NULL) &&
            (pDischargeDutyCycle != NULL) &&
            (iDeviceId != 0u) &&
            (iDeviceId <= ADI_WIL_DEV_NODE_61) &&
            ((iDeviceId & (iDeviceId - 1u)) == 0u));
}

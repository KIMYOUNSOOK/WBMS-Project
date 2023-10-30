/******************************************************************************
 * @file    wb_wil_get_cell_balancing_status.c
 *
 * @brief   Get the cell balancing status of a safety MCU
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_get_cell_balancing_status.h"
#include "wb_wil_ui_fusa.h"
#include "wb_assl_fusa.h"
#include "wb_wil_device.h"
#include "wb_scl_msg_defs.h"
#include "adi_wil_cell_balancing_status.h"
#include <string.h>

 /******************************************************************************
  * #defines
  *****************************************************************************/

  /* Number of retries before the request is aborted */
#define ADI_WIL_GET_CELL_BALANCING_STATUS_RETRIES (2u)

/* Time in milliseconds before request is retried */
#define ADI_WIL_GET_CELL_BALANCING_STATUS_TIMEOUT (300u)

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetCellBalancingStatusAPI (adi_wil_safety_internals_t * const pInternals)
{
    /* Return value of the function */
    adi_wil_err_t rc;

    /* Validate the input parameter */
    if (NULL == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Attempt to acquire the lock */
        rc = wb_wil_ui_AcquireFuSaLock (pInternals->pPack,
                                        pInternals->pPack);

        /* If we were able to acquire the lock, proceed... */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Store the currently connected nodes in the API requested node map */
            pInternals->API.iRequestedNodes = pInternals->ASSL.iConnectedNodes;

            /* Attempt to write the request to the buffer */
            rc = wb_assl_WriteGetCellBalancingStatusFrame (pInternals);

            /* If we were able to write the request it is ready to go out so
             * mark as active */
            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                /* Activate the request with the defined number of retries and
                 * timeout values */
                rc = wb_assl_ActivateRequest (pInternals,
                                              ADI_WIL_API_GET_CELL_BALANCING_STATUS,
                                              ADI_WIL_GET_CELL_BALANCING_STATUS_RETRIES,
                                              ADI_WIL_GET_CELL_BALANCING_STATUS_TIMEOUT);
            }

            /* Release the lock if either the writing or activation steps
             * failed */
            if (ADI_WIL_ERR_SUCCESS != rc)
            {
                wb_wil_ui_ReleaseFuSaLock (pInternals->pPack,
                                           pInternals->pPack);
            }
        }
    }

    return rc;
}

void wb_wil_HandleGetCellBalancingStatusResponse (adi_wil_safety_internals_t * const pInternals,
                                                  adi_wil_cell_balancing_status_t const * const pStatus,
                                                  adi_wil_err_t rc)
{
    /* Validate pointer before dereferencing */
    if ((NULL != pInternals) &&
        (NULL != pStatus))
    {
        /* Validate the response code was success before generating event */
        if (rc == ADI_WIL_ERR_SUCCESS)
        {
            /* If we haven't already generated an event for this node, do so */
            if ((pInternals->API.iRequestedNodes & pStatus->eDeviceId) != 0ULL)
            {
                /* Generate event */
                wb_wil_ui_GenerateFuSaEvent (pInternals->pPack,
                                             ADI_WIL_EVENT_CELL_BALANCING_STATUS,
                                             pStatus);

                /* Clear the responding node from the requested node bitmap */
                pInternals->API.iRequestedNodes &= ~pStatus->eDeviceId;
            }
        }

        /* Check if we've received all responses from all requested nodes or
         * a node had a failure */
        if ((pInternals->API.iRequestedNodes == 0u) ||
            (rc != ADI_WIL_ERR_SUCCESS))
        {
            /* Deactivate the request when all responses have been received */
            wb_assl_DeactivateRequest (pInternals);

            /* Generate callback and release lock */
            wb_wil_ui_GenerateFuSaCb (pInternals->pPack,
                                      ADI_WIL_API_GET_CELL_BALANCING_STATUS,
                                      rc,
                                      NULL);

            wb_wil_ui_ReleaseFuSaLock (pInternals->pPack,
                                       pInternals->pPack);
        }
    }
}

void wb_wil_TimeoutGetCellBalancingStatus (adi_wil_safety_internals_t const * const pInternals)
{
    /* Validate pointer before dereferencing */
    if (pInternals != NULL)
    {
        /* Generate callback and release lock */
        wb_wil_ui_GenerateFuSaCb (pInternals->pPack,
                                  ADI_WIL_API_GET_CELL_BALANCING_STATUS,
                                  ADI_WIL_ERR_TIMEOUT,
                                  NULL);

        wb_wil_ui_ReleaseFuSaLock (pInternals->pPack,
                                   pInternals->pPack);
    }
}

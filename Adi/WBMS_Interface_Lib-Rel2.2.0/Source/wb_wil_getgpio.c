/******************************************************************************
 * @file    wb_wil_getgpio.c
 *
 * @brief   Get the status of a supported GPIO pin
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_getgpio.h"
#include "wb_wil_request.h"
#include "wb_nil.h"
#include "wb_req_get_gpio.h"
#include "wb_rsp_get_gpio.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_setgpio.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetGPIOFunc (adi_wil_pack_internals_t * const pInternals);

static void wb_wil_GetGPIOComplete (adi_wil_pack_internals_t * const pInternals,
                                    adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_GetGPIOAPI (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 adi_wil_gpio_id_t eGPIOId)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_ACTIVE, ADI_WIL_MODE_STANDBY, ADI_WIL_MODE_MONITORING };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE };
    bool bReleaseLock = false;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Validate system mode */
        rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = wb_wil_SetGPIOCheckPin (eGPIOId);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Set-up the correct bit-mask to prepare for a new packet request
             * generation */
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_GetGPIOFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            pInternals->GpioState.GpioId = eGPIOId;
            /* Send 'get GPIO' request packet */
            wb_wil_GetGPIOFunc (pInternals);
        }

        /* Release lock if any of the above steps returns a failure */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;
        }
    }

    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    return rc;
}

void wb_wil_HandleGetGPIOResponse (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   wbms_cmd_resp_get_gpio_t const * const pResponse)
{
    (void) iDeviceId;

    if (wb_wil_api_CheckToken (pInternals, pResponse->iToken, true) != ADI_WIL_ERR_SUCCESS)
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
    {
        /* Response packet from end device mentions that 'Get GPIO' API has not
         * completed successfully.Generate call back to the host application
         * with a return code from end device (that has responded with this
         * response packet) */
        wb_wil_GetGPIOComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GetGpioValueFromUint (pResponse->iValue, &pInternals->GpioState.GpioValue))
    {
        /* Response packet from end device does not have a valid GPIO value.
         * Generate call back to the host application with a return code
         * of FAIL */
        wb_wil_GetGPIOComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* Get GPIO API has been completed successfully.
         * Generate call back to the host application with the GPIO value and
         * return code of SUCCESS */
        wb_wil_GetGPIOComplete (pInternals, ADI_WIL_ERR_SUCCESS);
    }
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_GetGPIOFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_get_gpio_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iGPIOId = wb_wil_GPIOtoUint (pInternals->GpioState.GpioId);

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_GetGPIOComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GetGPIORequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_GetGPIOComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_GetGPIOComplete (adi_wil_pack_internals_t * const pInternals,
                                    adi_wil_err_t rc)
{
    adi_wil_gpio_value_t GpioVal;

    /* Get GPIO API has reached a completion. Reset all the internal
     * variables */
    (void) memcpy (&GpioVal, &pInternals->GpioState.GpioValue, sizeof (adi_wil_gpio_value_t));
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Generate callback with return structure that contains the GPIO state
         * and a return code of SUCCESS */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_GPIO, rc, (void *) &GpioVal);
    }
    else
    {
        /* Because the Get GPIO API has failed, generate callback with a null
         * return structure and a return code of FAIL */
        wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_GET_GPIO, rc, (void *) 0);
    }

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

/******************************************************************************
 * @file    wb_wil_setgpio.c
 *
 * @brief   Set a supported GPIO pin on a node
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_setgpio.h"
#include "wb_wil_request.h"
#include "wb_req_set_gpio.h"
#include "wb_rsp_generic.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include <string.h>

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetGPIOFunc (adi_wil_pack_internals_t * const pInternals);
static void wb_wil_SetGPIOComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetGPIOAPI (adi_wil_pack_internals_t * const pInternals,
                                 uint64_t iDeviceId,
                                 adi_wil_gpio_id_t eGPIOId,
                                 adi_wil_gpio_value_t eGPIOValue)
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
            rc = wb_wil_SetupRequest (pInternals, iDeviceId, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, &wb_wil_SetGPIOFunc);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Populate the structure which would be used in creating the
             * request packet */
            pInternals->GpioState.GpioId = eGPIOId;
            pInternals->GpioState.GpioValue = eGPIOValue;
            /* Send 'set GPIO' request packet */
            wb_wil_SetGPIOFunc (pInternals);
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

void wb_wil_HandleSetGPIOResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    (void) iDeviceId;

    if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true))
    {
        /* Do nothing, there is no active request with this token */
    }
    else
    {
        wb_wil_SetGPIOComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
    }
}

adi_wil_err_t wb_wil_SetGPIOCheckPin (adi_wil_gpio_id_t gpio_pin)
{
    /* Only the following GPIO pin numbers are supported */
    const adi_wil_gpio_id_t valid_pins[] = {
                                ADI_WIL_GPIO_0,
                                ADI_WIL_GPIO_2,
                                ADI_WIL_GPIO_7,
                                ADI_WIL_GPIO_8,
                                ADI_WIL_GPIO_9,
                                ADI_WIL_GPIO_10 };

    adi_wil_err_t rc;

    /* Initialize return count to INVALID PARAMETER to start with */
    rc = ADI_WIL_ERR_INVALID_PARAMETER;

    /* Cycle through the max valid GPIO pins until the correct match is found*/
    for (uint8_t i = 0u; i < (sizeof (valid_pins) / sizeof (adi_wil_gpio_id_t)); i++)
    {
        if (gpio_pin == valid_pins [i]) {
            /* GPIO pin match found */
            rc = ADI_WIL_ERR_SUCCESS;
            break;
        }
    }

    /* When GPIO match is not found, return INVALID PARAMETER */
    return rc;
}

/******************************************************************************
 * Static functions
 *****************************************************************************/

static void wb_wil_SetGPIOFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_set_gpio_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iGPIOId = wb_wil_GPIOtoUint (pInternals->GpioState.GpioId);
    Request.iValue = (pInternals->GpioState.GpioValue == ADI_WIL_GPIO_LOW) ? 0u : 1u;

    if (pInternals->UserRequestState.iRetries >= ADI_WIL_RETRIES)
    {
        wb_wil_SetGPIOComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_SetGPIORequest (pInternals, &Request, ADI_WIL_RESPONSE_TIMEOUT_MS))
    {
        wb_wil_SetGPIOComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_SetGPIOComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = ((void *) 0);

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_SET_GPIO, rc, (void *) 0);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

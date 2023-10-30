/******************************************************************************
 * @file    wb_wil_api.h
 *
 * @brief   Functions required to setup, validate and log an API invocation
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_api.h"
#include "adi_wil_types.h"
#include "adi_wil_port.h"
#include "adi_wil_hal_ticker.h"
#include "wbms_cmd_mgr_defs.h"
#include "wb_wil_utils.h"
#include "adi_wil_osal.h"
#include "adi_wil_pack_internals.h"
#include <stdint.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

#define WB_WIL_TIMER_JITTER_TOLERANCE (5u)

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static adi_wil_err_t adi_wil_CheckConnected (adi_wil_pack_internals_t const * const pInternals);

/******************************************************************************
 * Static functions
 *****************************************************************************/

static adi_wil_err_t adi_wil_CheckConnected (adi_wil_pack_internals_t const * const pInternals)
{
    adi_wil_err_t rc;
    bool bConnected;

    /* Validate input */
    if ((void *) 0 == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else if (pInternals->pManager0Port == (void *) 0)
    {
        /* Return not connected as pManager0Port is zeroed after disconnect */
        rc = ADI_WIL_ERR_NOT_CONNECTED;
    }
    else
    {
        bConnected = pInternals->pManager0Port->Internals.bConnected;

        if (pInternals->pManager1Port != (void *) 0)
        {
            bConnected = bConnected || pInternals->pManager1Port->Internals.bConnected;
        }

        if (!bConnected)
        {
            rc = ADI_WIL_ERR_NOT_CONNECTED;
        }
        else
        {
            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    return rc;
}

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t adi_wil_ValidateInstance (adi_wil_pack_internals_t const * const pInternals, bool bCheckConnected)
{
    adi_wil_err_t rc;

    /* Validate input parameter */
    if ((void *) 0 == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else if (bCheckConnected)
    {
        /* Check if the required device is still connected to the host */
        rc = adi_wil_CheckConnected (pInternals);
    }
    else
    {
        /* All good so far, return a SUCCESS */
        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

adi_wil_err_t adi_wil_CheckNullableParams (uint8_t iParamCount, void const * const Params[])
{
    adi_wil_err_t rc;

    /* Initialize return code to SUCCESS to start with */
    rc = ADI_WIL_ERR_SUCCESS;

    for (uint8_t i = 0u; i < iParamCount; i++)
    {
        /* Validate input parameter */
        if (Params [i] == (void *) 0)
        {
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
            break;
        }
    }

    return rc;
}

adi_wil_err_t adi_wil_CheckSystemMode (adi_wil_pack_internals_t const * const pInternals, uint8_t iModeCount, adi_wil_mode_t const ValidModes[])
{
    adi_wil_err_t rc;

    /* Initialize return code with INVALID MODE */
    rc = ADI_WIL_ERR_INVALID_MODE;

    for (uint8_t i = 0u; i < iModeCount; i++)
    {
        if (ValidModes [i] == pInternals->eMode)
        {
            rc = ADI_WIL_ERR_SUCCESS;
            break;
        }
    }

    /* When mode match is not found, return INVALID MODE */
    return rc;
}

adi_wil_err_t adi_wil_CheckFileType (adi_wil_pack_internals_t const * const pInternals, adi_wil_file_type_t eFileType, uint8_t iCount, adi_wil_file_type_t const ValidTypes[])
{
    (void) pInternals;

    adi_wil_err_t rc;

    /* Initialize return code to INVALID PARAMETER to start with */
    rc = ADI_WIL_ERR_INVALID_PARAMETER;

    for (uint8_t i = 0u; i < iCount; i++)
    {
        if (ValidTypes [i] == eFileType)
        {
            /* Found match for the file type, return with SUCCESS */
            rc = ADI_WIL_ERR_SUCCESS;
            break;
        }
    }

    return rc;
}

adi_wil_err_t wb_wil_api_StartTimer (adi_wil_pack_internals_t * const pInternals, uint16_t * const pToken, uint32_t iTimeout)
{
    adi_wil_err_t rc;

    /* Validate input params */
    if (((void *) 0 == pInternals) || ((void *) 0 == pToken))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Don't allow a timeout value of 0 */
    /* Don't allow a job to be posted that falls within our jitter tolerance */
    else if ((iTimeout == 0u) ||
             (iTimeout >= (UINT32_MAX - WB_WIL_TIMER_JITTER_TOLERANCE)))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        pInternals->UserRequestState.iStartTime = adi_wil_hal_TickerGetTimestamp ();
        pInternals->UserRequestState.iTimeout = iTimeout;
        pInternals->UserRequestState.bValid = true;

        *pToken = pInternals->UserRequestState.iToken;

        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

adi_wil_err_t wb_wil_api_CheckToken (adi_wil_pack_internals_t * const pInternals, uint16_t iToken, bool bClear)
{
    adi_wil_err_t rc;

    /* Validate input parameter */
    if ((void *) 0 == pInternals)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else if (pInternals->UserRequestState.bValid && (iToken == pInternals->UserRequestState.iToken))
    {
        /* Invert the logic of bClear to invalidate/not invalidate the current
         * request */
        pInternals->UserRequestState.bValid = !bClear;

        rc = ADI_WIL_ERR_SUCCESS;
    }
    else
    {
        /* If the requested state is not valid and the current token does not
         * match the requested one, return with a return code of FAIL */
        rc = ADI_WIL_ERR_FAIL;
    }

    return rc;
}

void wb_wil_api_CheckForTimeout (adi_wil_pack_internals_t * const pInternals,
                                 uint32_t iCurrentTime)
{
    uint32_t iElapsedTime;

    if ((void *) 0 != pInternals)
    {
        if (pInternals->UserRequestState.bValid)
        {
            iElapsedTime = (iCurrentTime - pInternals->UserRequestState.iStartTime);

            if (iElapsedTime < iCurrentTime)
            {
                /* CERT-C - Allow rollover as timers are expected to wrap */
            }

            /* Check if the job has elapsed,
             * ...but don't trigger a timeout if we've potentially underflowed
             * by 5ms to allow for timer jitter */
            if ((iElapsedTime > pInternals->UserRequestState.iTimeout) &&
                (iElapsedTime < (UINT32_MAX - WB_WIL_TIMER_JITTER_TOLERANCE)))
            {
                /* Timeout occurred, mark as invalid */
                pInternals->UserRequestState.bValid = false;
                /* Increment current retry count */
                wb_wil_IncrementWithRollover8 (&pInternals->UserRequestState.iRetries);

                /* Check our function pointer hasn't been cleared before trying
                 * to execute it */
                if ((void *) 0 != pInternals->UserRequestState.pfRequestFunc)
                {
                    pInternals->UserRequestState.pfRequestFunc (pInternals);
                }
            }
        }
    }
}

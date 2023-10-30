/******************************************************************************
 * @file    wb_wil_disconnect.c
 *
 * @brief   Disconnect a pack from the BMS application
 *
 * Copyright(c) 2022 Analog Devices, Inc.All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.and its licensors.
 *****************************************************************************/

#include "wb_wil_disconnect.h"
#include "wb_nil.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_DisconnectAPI (adi_wil_pack_internals_t * const pInternals)
{
    adi_wil_err_t rc;

    rc = ADI_WIL_ERR_SUCCESS;

    /* Validate input parameter */
    if (pInternals == (void *) 0)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* As we are disconnecting API, no point in maintaining connection */
        pInternals->bMaintainConnection = false;

        if ((void *) 0 != pInternals->pManager0Port)
        {
            /* Logout, close port and update internal parameters for
             * manager 0 */
            wb_nil_Logout (pInternals->pManager0Port);
            rc = wb_nil_ClosePort (pInternals->pManager0Port);
            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                pInternals->pManager0Port->Internals.bConnected = false;
                pInternals->pManager0Port = (void *) 0;
            }
        }

        if ((ADI_WIL_ERR_SUCCESS == rc) && ((void *) 0 != pInternals->pManager1Port))
        {
            /* Logout, close port and update internal parameters for
             * manager 1 */
            wb_nil_Logout (pInternals->pManager1Port);
            rc = wb_nil_ClosePort (pInternals->pManager1Port);
            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                pInternals->pManager1Port->Internals.bConnected = false;
                pInternals->pManager1Port = (void *) 0;
            }
        }
    }

    return rc;
}

/******************************************************************************
 * @file     wb_wil_initialize.c
 *
 * @brief    WBMS initialization source
 *
 * @details  Contains API definitions for the WBMS Interface Library init routines.
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_initialize.h"
#include "wb_wil_ui.h"
#include "wb_wil_api.h"
#include "wb_nil.h"
#include "adi_wil_osal.h"
#include "wb_wil_utils.h"
#include "adi_wil_pack_internals.h"
#include <string.h>

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

adi_wil_err_t wb_wil_InitializeAPI (void)
{
    return wb_nil_Initialize ();
}

adi_wil_err_t adi_wil_InitializePack (adi_wil_pack_t const * const pPack,
                                      adi_wil_pack_internals_t * const pInternals,
                                      adi_wil_port_t * const pManager0Port,
                                      adi_wil_port_t * const pManager1Port)
{
    adi_wil_err_t rc;
    adi_wil_pack_t * pLockId;
    bool bLockAcquired = false;

    void const * const NullableParams[] = { pPack, pInternals, pManager0Port };

    /* Validate input parameters */
    rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);

    if (rc == ADI_WIL_ERR_SUCCESS)
    {
        (void) memset (&pLockId, 0xFF, sizeof (void *));

        /* Acquire lock to make sure processTask isn't running */
        rc = wb_wil_ui_AcquireLock ((void *)0, pLockId);

        if (rc == ADI_WIL_ERR_SUCCESS)
        {
            bLockAcquired = true;
        }
        else
        {
            /* Update lock flag that is used to release the lock that was
             * acquired */
            bLockAcquired = false;
            rc = ADI_WIL_ERR_FAIL;
        }

        /* Initialize manager 0's port */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = wb_nil_InitPort (pInternals, pManager0Port);
        }

        /* Initialize manager 1's port */
        if ((ADI_WIL_ERR_SUCCESS == rc) && ((void *) 0 != pManager1Port))
        {
            rc = wb_nil_InitPort (pInternals, pManager1Port);
        }

        if (rc == ADI_WIL_ERR_SUCCESS)
        {
            /* Initialize the required internal variables */
            (void) memset (pInternals, 0, sizeof (adi_wil_pack_internals_t));
            pInternals->pPack = pPack;
            pInternals->pManager0Port = pManager0Port;
            pInternals->pManager1Port = pManager1Port;
            pInternals->pCurrentPort = pInternals->pManager0Port;
            pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER;
            pInternals->UserRequestState.pfRequestFunc = (void *) 0;
            pInternals->bMaintainConnection = false;

            /* Initialize BMS packet count to 0 for both managers */
            pInternals->Stats.BmsPktStats.iManager0PktCount = 0u;
            pInternals->Stats.BmsPktStats.iManager1PktCount = 0u;
        }

        if (bLockAcquired)
        {
            wb_wil_ui_ReleaseLock ((void *)0, pLockId);
        }
    }

    return rc;
}

adi_wil_err_t wb_wil_TerminateAPI (void)
{
    /* Call the core function from this wrapper function */
    return wb_nil_Terminate ();
}


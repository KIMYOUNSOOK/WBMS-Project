/******************************************************************************
 * @file    wb_wil_get_network_status.c
 *
 * @brief   Retrieve the network status of a given pack
 *
 * Copyright(c) 2022 Analog Devices, Inc.All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc.and its licensors.
 *****************************************************************************/

#include "wb_wil_get_network_status.h"
#include "wb_wil_request.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_ui.h"
#include "adi_wil_network_status.h"
#include "wb_wil_api.h"

adi_wil_err_t wb_wil_GetNetworkStatusAPI (adi_wil_pack_internals_t const * const pInternals,
                                          adi_wil_network_status_t * const pStatus)
{
    adi_wil_err_t rc;
    void const * const NullableParams[] = { pStatus };

    /* Validate input parameter */
    rc = adi_wil_ValidateInstance (pInternals, true);

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Update network status statistics with stored internal status. */
        pStatus->iCount = pInternals->NodeState.iCount;
        pStatus->iConnectState = pInternals->NodeState.iConnectState;
    }
 
    return rc;
}

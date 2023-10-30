/******************************************************************************
 * @file    wb_wil_error_notify.c
 *
 * @brief   Functions to handle error notifications.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_error_notify.h"
#include "wb_wil_reset.h"
#include "wb_nil.h"
#include "adi_wil_pack_internals.h"
#include "adi_wil_port.h"
#include "wb_wil_connect.h"
#include "wb_wil_ui.h"
#include <string.h>

/******************************************************************************
 * Public functions
 *****************************************************************************/

void wb_wil_HandlePortDownEvent (adi_wil_pack_internals_t * const pInternals, adi_wil_port_t * const pPort)
{
    adi_wil_device_t eDeviceId;

    if ((void *) 0 != pInternals)
    {
        (void) wb_nil_Logout (pPort);
        pPort->Internals.bConnected = false;

        eDeviceId = (pPort == pPort->Internals.pPackInternals->pManager0Port) ? ADI_WIL_DEV_MANAGER_0 : ADI_WIL_DEV_MANAGER_1;

        /* If ResetMgr was not in progress */
        if (ADI_WIL_ERR_FAIL == wb_wil_ResetMgrNotifyConnectionLost (pInternals, eDeviceId))
        {
            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_COMM_MGR_DISCONNECTED, &eDeviceId);
        }
    }
}

void wb_wil_HandlePortAvailableEvent (adi_wil_pack_internals_t * const pInternals, adi_wil_port_t * const pPort)
{
    /* Validate the input parameter */
    if ((void *) 0 != pPort)
    {
        pPort->Internals.bLinkAvailable = true;

        if (!pPort->Internals.bConnected)
        {
            /* Validate the input parameter */
            if ((void *) 0 != pInternals)
            {
                if (pInternals->bMaintainConnection)
                {
                    pInternals->bReconnectPending = true;
                }
            }
        }
    }
}

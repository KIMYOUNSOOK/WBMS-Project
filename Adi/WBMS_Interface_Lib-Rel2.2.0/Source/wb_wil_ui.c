/******************************************************************************
 * @file     wb_wil_ui.c
 *
 * @brief    WBMS Interface Library - User Interface source
 *
 * @details  Contains API definitions for the WBMS Interface Library init routines.
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "adi_wil_api.h"
#include "adi_wil_port.h"
#include "wb_wil_ui.h"
#include "wb_wil_ui_fusa.h"
#include "adi_wil_ui_internals.h"
#include "adi_wil_osal.h"
#include "adi_wil_pack.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_app_interface.h"
#include "wb_assl_fusa.h"

/* API implementation interface headers */

#include "wb_wil_initialize.h"
#include "wb_wil_connect.h"
#include "wb_wil_disconnect.h"
#include "wb_wil_setmode.h"
#include "wb_wil_set_acl.h"
#include "wb_wil_get_acl.h"
#include "wb_wil_query_device.h"
#include "wb_wil_get_network_status.h"
#include "wb_wil_load_file.h"
#include "wb_wil_erase_file.h"
#include "wb_wil_get_version.h"
#include "wb_wil_get_file_crc.h"
#include "wb_wil_setgpio.h"
#include "wb_wil_getgpio.h"
#include "wb_wil_selectscript.h"
#include "wb_wil_modifyscript.h"
#include "wb_wil_inventory_transition.h"
#include "wb_wil_get_file.h"
#include "wb_wil_process_task.h"
#include "wb_wil_set_contextual.h"
#include "wb_wil_get_contextual.h"
#include "wb_wil_reset.h"
#include "wb_wil_set_state_of_health.h"
#include "wb_wil_get_state_of_health.h"
#include "wb_wil_fault_service.h"
#include "wb_wil_rotate_key.h"
#include "wb_wil_set_customer_identifier.h"
#include "wb_wil_capture_network_data.h"
#include "wb_wil_update_monitor_params.h"
#include "wb_wil_get_monitor_params_crc.h"
#include "wb_wil_dmh.h"
#include "wb_wil_configure_cell_balancing.h"
#include "wb_wil_get_cell_balancing_status.h"
#include "wb_wil_device.h"

#include <string.h>

/******************************************************************************
 * #defines
 *****************************************************************************/

/**
 * @brief   Maximum time in milliseconds that the Ticker HAL can jitter
 */
#define ADI_WIL_UI_TIMER_JITTER (2u)

/**
 * @brief   Maximum time in milliseconds for a lock to be held beyond expected
 *          limit before a fault is declared. Include the timer jitter
 *          for true FTDI value.
 */
#define ADI_WIL_UI_LOCK_HOLD_TIMEOUT_FTDI (100u + ADI_WIL_UI_TIMER_JITTER)

/******************************************************************************
 * Function Declarations
 *****************************************************************************/

static void wb_wil_ui_Initialize (adi_wil_ui_internals_t * const pInternals);

static void wb_wil_ui_Process (adi_wil_pack_t const * const pPack,
                               uint32_t iCurrentTicks);

static bool wb_wil_ui_CheckTimer (adi_wil_ui_lock_state_t * const pState,
                                  uint32_t iCurrentTime);

static adi_wil_err_t wb_wil_ui_AcquireLockCommon (adi_wil_pack_t const * const pPack,
                                                  void const * const pLockID,
                                                  bool bFusaContext);

static void wb_wil_ui_ReleaseLockCommon (adi_wil_pack_t const * const pPack,
                                         void const * const pLockID,
                                         bool bFusaContext);

static void wb_wil_ui_ActivateLock (adi_wil_ui_lock_state_t * const pState,
                                    bool bFusaContext);

static bool wb_wil_ui_DeactivateLock (adi_wil_ui_lock_state_t * const pState,
                                      bool bFusaContext);

static adi_wil_safety_internals_t * wb_ui_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack);

/******************************************************************************
 * Static variables
 *****************************************************************************/

/* Global lock state variable - used for storage of global lock acquired,
 * timestamp, etc */
static adi_wil_ui_lock_state_t GlobalLockState;

/* Global lock ID variable - used for storage of all 0xFF lock ID */
static adi_wil_pack_t * pGlobalLockID;

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

adi_wil_err_t adi_wil_Initialize (void)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Generate global lock ID - pointer to void with all 0xFF bytes */
    (void) memset (&pGlobalLockID, 0xFF, sizeof (void *));

    /* Initialize global lock timer variables */
    GlobalLockState.bAcquired = false;
    GlobalLockState.bFusaContext = false;
    GlobalLockState.iTimestamp = 0u;

    /* Create a single locking resource for global use */
    if (adi_wil_osal_CreateResource (pGlobalLockID) != ADI_WIL_OSAL_ERR_SUCCESS)
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else
    {
        /* This method does not use a pack pointer - no need to dereference so
         * just return the rc from the API */
        rc = wb_wil_InitializeAPI ();
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_Terminate (void)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Destroy the lock regardless of if it is acquired and ignore RC as we are
     * tearing it all down */
    (void) adi_wil_osal_DestroyResource (pGlobalLockID);

    /* This method does not use a pack pointer - no need to dereference so just
        * return the rc from the API */
    rc = wb_wil_TerminateAPI ();

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_QueryDevice (adi_wil_port_t * const pPort)
{
    /* This method does not use a pack pointer - no need to dereference so just
     * return the rc from the API */
    return wb_wil_QueryDeviceAPI (pPort);
}

adi_wil_err_t adi_wil_GetWILSoftwareVersion (adi_wil_version_t * const pVersion)
{
    /* This method does not use a pack pointer - no need to dereference so just
     * return the rc from the API */
    return wb_wil_GetWILSoftwareVersionAPI (pVersion);
}

adi_wil_err_t adi_wil_Connect (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_ui_GetSafetyInternalsPointer (pPack);

    /* Validate pack instance before dereferencing. Ensure global lock ID has
     * been created and pack pointer is not the same as global lock ID */
    if (((void *) 0 == pInternals) ||
        ((void *) 0 == pGlobalLockID) ||
        (pPack == pGlobalLockID))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Initialize the UI internals for this Pack */
        wb_wil_ui_Initialize (&pInternals->UI);

        /* Initialize the ASSL + XMS internals for this Pack */
        rc = wb_assl_Initialize (pPack, pPack->pDataBuffer, pPack->iDataBufferCount);

        /* If we were able to initialize the ASSL + XMS, continue... */
        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            /* Try and create a lock for this pack and set RC accordingly */
            if (adi_wil_osal_CreateResource (pPack) != ADI_WIL_OSAL_ERR_SUCCESS)
            {
                rc = ADI_WIL_ERR_FAIL;
            }
            /* If valid, invoke API and set rc to return value */
            else
            {
                rc = wb_wil_ConnectAPI (pPack, pPack->pInternals, pPack->pManager0Port, pPack->pManager1Port);
            }
        }
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_Disconnect (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else 
    {
        rc = wb_wil_ui_AcquireLock (pPack, pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_DisconnectAPI (pPack->pInternals);

        /* Destroy the lock */
        if (adi_wil_osal_DestroyResource (pPack) != ADI_WIL_OSAL_ERR_SUCCESS)
        {
            /* If we failed to destroy the lock, release it so the user can try
             * again */
            (void) adi_wil_osal_ReleaseResource (pPack);

            /* Update the return code to indicate failure */
            rc = ADI_WIL_ERR_FAIL;
        }
        /* ... else reset the timer associated with this pack */
        else
        {
            /* Call ASSL Initialize to reset ASSL internal state */
            (void) wb_assl_Initialize (pPack, pPack->pDataBuffer, pPack->iDataBufferCount);

            /* Validate pSafetyInternals is still set before de-referencing. */
            if ((void *) 0 != pPack->pSafetyInternals)
            {
                /* Call UI Initialize to reset UI internal state and stop
                 * timers */
                wb_wil_ui_Initialize (&pPack->pSafetyInternals->UI);
            }
        }
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetMode (adi_wil_pack_t const * const pPack,
                               adi_wil_mode_t eMode)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetModeAPI (pPack->pInternals, eMode);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetMode (adi_wil_pack_t const * const pPack,
                               adi_wil_mode_t * const pMode)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetModeAPI (pPack->pInternals, pMode);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetACL (adi_wil_pack_t const * const pPack,
                              uint8_t const * const pData,
                              uint8_t iCount)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetACLAPI (pPack->pInternals, pData, iCount);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetACL (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetACLAPI (pPack->pInternals);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetNetworkStatus (adi_wil_pack_t const * const pPack,
                                        adi_wil_network_status_t * const pStatus)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetNetworkStatusAPI (pPack->pInternals, pStatus);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_LoadFile (adi_wil_pack_t const * const pPack,
                                adi_wil_device_t eDeviceId,
                                adi_wil_file_type_t eFileType,
                                uint8_t const * const pData)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {   
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_LoadFileAPI (pPack->pInternals, eDeviceId, eFileType, pData);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_EraseFile (adi_wil_pack_t const * const pPack,
                                 adi_wil_device_t eDeviceId,
                                 adi_wil_file_type_t eFileType)
{
    /* Method return code variable */
    adi_wil_err_t rc;
    
    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_EraseFileAPI (pPack->pInternals, eDeviceId, eFileType);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetDeviceVersion (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetDeviceVersionAPI (pPack->pInternals, eDeviceId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetFileCRC (adi_wil_pack_t const * const pPack,
                                 adi_wil_device_t eDeviceId,
                                 adi_wil_file_type_t eFileType)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetFileCRCAPI (pPack->pInternals, eDeviceId, eFileType);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetGPIO (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_gpio_id_t eGPIOId,
                               adi_wil_gpio_value_t eGPIOValue)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetGPIOAPI (pPack->pInternals, eDeviceId, eGPIOId, eGPIOValue);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetGPIO (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_gpio_id_t eGPIOId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetGPIOAPI (pPack->pInternals, eDeviceId, eGPIOId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SelectScript (adi_wil_pack_t const * const pPack,
                                    adi_wil_device_t eDeviceId,
                                    adi_wil_sensor_id_t eSensorId,
                                    uint8_t iScriptId)
{
    /* Method return code variable */
    adi_wil_err_t rc;



    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SelectScriptAPI (pPack->pInternals, eDeviceId, eSensorId, iScriptId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_ModifyScript (adi_wil_pack_t const * const pPack,
                                    adi_wil_device_t eDeviceId,
                                    adi_wil_sensor_id_t eSensorId,
                                    adi_wil_script_change_t const * const pData)
{
    /* Method return code variable */
    adi_wil_err_t rc;

     /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_ModifyScriptAPI (pPack->pInternals, eDeviceId, eSensorId, pData);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_EnterInventoryState (adi_wil_pack_t const * const pPack,
                                           adi_wil_device_t eDeviceId,
                                           bool bEnable,
                                           uint64_t iCurrentTime)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_EnterInventoryStateAPI (pPack->pInternals, eDeviceId, bEnable, iCurrentTime);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetFile (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_file_type_t eFileType)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetFileAPI (pPack->pInternals, eDeviceId, eFileType);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_ProcessTask (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Current timestamp value */
    uint32_t iCurrentTicks;

    /* Retrieve current timestamp from ticker HAL */
    iCurrentTicks = adi_wil_hal_TickerGetTimestamp ();

    /* Retrieve safety internals pointer */
    pInternals = wb_ui_GetSafetyInternalsPointer (pPack);

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        /* If invalid, invoke API with NULL pointer and return RC */
        rc = wb_wil_ProcessTaskAPI (((void *) 0), iCurrentTicks);
    }
    else
    {
        /* If valid, process ASSL timers and tasks */
        wb_assl_Process (pInternals, iCurrentTicks);

        /* Invoke API and set rc to return value */
        rc = wb_wil_ProcessTaskAPI (pPack->pInternals, iCurrentTicks);
    }

    /* Process UI timers - NULL pointer is allowed */
    wb_wil_ui_Process (pPack, iCurrentTicks);

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetContextualData (adi_wil_pack_t const * const pPack,
                                         adi_wil_device_t eDeviceId,
                                         adi_wil_contextual_id_t eContextualDataId,
                                         adi_wil_contextual_data_t const * const pData)
{
    /* Method return code variable */
    adi_wil_err_t rc;



    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetContextualDataAPI (pPack->pInternals, eDeviceId, eContextualDataId, pData);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_ResetDevice (adi_wil_pack_t const * const pPack,
                                   adi_wil_device_t eDeviceId)
{
    /* Method return code variable */
    adi_wil_err_t rc;


    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_ResetDeviceAPI (pPack->pInternals, eDeviceId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetContextualData (adi_wil_pack_t const * const pPack,
                                         adi_wil_device_t eDeviceId,
                                         adi_wil_contextual_id_t eContextualDataId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetContextualDataAPI (pPack->pInternals, eDeviceId, eContextualDataId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetStateOfHealth (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId,
                                        uint8_t iPercentage)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetStateOfHealthAPI (pPack->pInternals, eDeviceId, iPercentage);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetStateOfHealth (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetStateOfHealthAPI (pPack->pInternals, eDeviceId);
    }

    /* Return error code to caller */
    return rc;

}

adi_wil_err_t adi_wil_EnableFaultServicing (adi_wil_pack_t const * const pPack,
                                            adi_wil_device_t eDeviceId,
                                            bool bEnable)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_EnableFaultServicingAPI (pPack->pInternals, eDeviceId, bEnable);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_RotateKey (adi_wil_pack_t const * const pPack,
                                 adi_wil_key_t eKeyType,
                                 adi_wil_device_t eDeviceId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_RotateKeyAPI (pPack->pInternals, eKeyType, eDeviceId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_SetCustomerIdentifierData (adi_wil_pack_t const * const pPack,
                                                 adi_wil_customer_identifier_t eCustomerIdentifier,
                                                 uint8_t const * const pData,
                                                 uint8_t iLength)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_SetCustomerIdentifierDataAPI (pPack->pInternals, eCustomerIdentifier, pData, iLength);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_EnableNetworkDataCapture (adi_wil_pack_t const * const pPack,
                                                adi_wil_network_data_t * pDataBuffer,
                                                uint16_t iDataBufferCount,
                                                bool bEnable)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_EnableNetworkDataCaptureAPI (pPack->pInternals, pDataBuffer, iDataBufferCount, bEnable);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_UpdateMonitorParameters (adi_wil_pack_t const * const pPack,
                                               adi_wil_device_t eDeviceId,
                                               uint8_t * const pData,
                                               uint16_t iLength)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_UpdateMonitorParametersAPI (pPack->pInternals, eDeviceId, pData, iLength);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetMonitorParametersCRC (adi_wil_pack_t const * const pPack,
                                               adi_wil_device_t eDeviceId)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetMonitorParametersCRCAPI (pPack->pInternals, eDeviceId);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_AssessNetworkTopology (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_AssessNetworkTopologyAPI (pPack->pInternals);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_ApplyNetworkTopology (adi_wil_pack_t const * const pPack,
                                            adi_wil_topology_t eTopology)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_ApplyNetworkTopologyAPI (pPack->pInternals,
                                             eTopology);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_ConfigureCellBalancing (adi_wil_pack_t const * const pPack,
                                              adi_wil_device_t eDeviceId,
                                              adi_wil_ddc_t const * const pDischargeDutyCycle,
                                              uint16_t iDuration,
                                              uint32_t iUVThreshold)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_ConfigureCellBalancingAPI (pPack->pSafetyInternals,
                                               eDeviceId,
                                               pDischargeDutyCycle,
                                               iDuration,
                                               iUVThreshold);
    }

    /* Return error code to caller */
    return rc;
}

adi_wil_err_t adi_wil_GetCellBalancingStatus (adi_wil_pack_t const * const pPack)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* Validate pack instance before dereferencing */
    if ((void *) 0 == pPack)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* If valid, invoke API and set rc to return value */
        rc = wb_wil_GetCellBalancingStatusAPI (pPack->pSafetyInternals);
    }

    /* Return error code to caller */
    return rc;
}

void wb_wil_ui_GenerateCb (adi_wil_pack_t const * const pPack,
                           adi_wil_api_t eNonSafetyAPI,
                           adi_wil_err_t rc,
                           void const * const pData)
{
    /* List of APIs allowed to perform callbacks from a non-safety component */
    static const adi_wil_api_t eValidNonSafetyAPIs [] = { ADI_WIL_API_CONNECT,
                                                          ADI_WIL_API_SET_MODE,
                                                          ADI_WIL_API_SET_ACL,
                                                          ADI_WIL_API_GET_ACL,
                                                          ADI_WIL_API_LOAD_FILE,
                                                          ADI_WIL_API_ERASE_FILE,
                                                          ADI_WIL_API_GET_DEVICE_VERSION,
                                                          ADI_WIL_API_GET_FILE_CRC,
                                                          ADI_WIL_API_SET_GPIO,
                                                          ADI_WIL_API_GET_GPIO,
                                                          ADI_WIL_API_SELECT_SCRIPT,
                                                          ADI_WIL_API_MODIFY_SCRIPT,
                                                          ADI_WIL_API_ENTER_INVENTORY_STATE,
                                                          ADI_WIL_API_GET_FILE,
                                                          ADI_WIL_API_SET_CONTEXTUAL_DATA,
                                                          ADI_WIL_API_GET_CONTEXTUAL_DATA,
                                                          ADI_WIL_API_RESET_DEVICE,
                                                          ADI_WIL_API_SET_STATE_OF_HEALTH,
                                                          ADI_WIL_API_GET_STATE_OF_HEALTH,
                                                          ADI_WIL_API_ENABLE_FAULT_SERVICING,
                                                          ADI_WIL_API_ROTATE_KEY,
                                                          ADI_WIL_API_SET_CUSTOMER_IDENTIFIER_DATA,
                                                          ADI_WIL_API_UPDATE_MONITOR_PARAMETERS,
                                                          ADI_WIL_API_GET_MONITOR_PARAMETERS_CRC,
                                                          ADI_WIL_API_ASSESS_NETWORK_TOPOLOGY,
                                                          ADI_WIL_API_APPLY_NETWORK_TOPOLOGY};

    /* Boolean value tracking if eNonSafetyAPI exists in eValidNonSafetyAPIs */
    bool bValid;

    /* Initialize local variables */
    bValid = false;

    /* Loop through all eValidNonSafetyAPIs looking for input eNonSafetyAPI */
    for (uint8_t i = 0u; i < (sizeof (eValidNonSafetyAPIs) / sizeof (eValidNonSafetyAPIs [0])); i++)
    {
        /* If found, mark as valid and break */
        if (eValidNonSafetyAPIs [i] == eNonSafetyAPI)
        {
            bValid = true;
            break;
        }
    }

    /* Check validity and pPack before dereferencing */
    if (bValid && ((void *) 0 != pPack))
    {
        /* Invoke API Callback */
        adi_wil_HandleCallback (pPack, pPack->pClientData, eNonSafetyAPI, rc, pData);
    }
}

void wb_wil_ui_GeneratePortCb (adi_wil_port_t const * const pPort,
                               adi_wil_api_t eNonSafetyAPI,
                               adi_wil_err_t rc,
                               void const * const pData)
{
    /* List of port-based APIs allowed to perform callbacks from a non-safety
     * component */
    static const adi_wil_api_t eValidNonSafetyAPIs[] = { ADI_WIL_API_QUERY_DEVICE };

    /* Boolean value tracking if eNonSafetyAPI exists in eValidNonSafetyAPIs */
    bool bValid;

    /* Initialize local variables */
    bValid = false;

    /* Loop through all eValidNonSafetyAPIs looking for input eNonSafetyAPI */
    for (uint8_t i = 0u; i < (sizeof (eValidNonSafetyAPIs) / sizeof (eValidNonSafetyAPIs [0])); i++)
    {
        /* If found, mark as valid and break */
        if (eValidNonSafetyAPIs [i] == eNonSafetyAPI)
        {
            bValid = true;
            break;
        }
    }

    /* Check validity and pPack before dereferencing */
    if (bValid && ((void *) 0 != pPort))
    {
        /* Invoke API Callback */
        adi_wil_HandlePortCallback (pPort, eNonSafetyAPI, rc, pData);
    }
}

void wb_wil_ui_GenerateEvent (adi_wil_pack_t const * const pPack,
                              adi_wil_event_id_t eNonSafetyEvent,
                              void const * const pData)
{
    /* List of events allowed from a non-safety component */
    static const adi_wil_event_id_t eValidNonSafetyEvents [] = { ADI_WIL_EVENT_COMM_NODE_CONNECTED,
                                                                 ADI_WIL_EVENT_COMM_NODE_DISCONNECTED,
                                                                 ADI_WIL_EVENT_COMM_MGR_CONNECTED,
                                                                 ADI_WIL_EVENT_COMM_MGR_DISCONNECTED,
                                                                 ADI_WIL_EVENT_COMM_MGR_TO_MGR_ERROR,
                                                                 ADI_WIL_EVENT_DATA_READY_HEALTH_REPORT,
                                                                 ADI_WIL_EVENT_DATA_READY_NETWORK_DATA,
                                                                 ADI_WIL_EVENT_FAULT_SOURCES,
                                                                 ADI_WIL_EVENT_FAULT_REPORT,
                                                                 ADI_WIL_EVENT_SEC_NODE_NOT_IN_ACL,
                                                                 ADI_WIL_EVENT_SEC_CERTIFICATE_CALCULATION_ERROR,
                                                                 ADI_WIL_EVENT_SEC_JOIN_NO_KEY,
                                                                 ADI_WIL_EVENT_SEC_JOIN_DUPLICATE_JOIN_COUNTER,
                                                                 ADI_WIL_EVENT_SEC_JOIN_MIC_FAILED,
                                                                 ADI_WIL_EVENT_SEC_UNKNOWN_ERROR,
                                                                 ADI_WIL_EVENT_XFER_DEVICE_REMOVED,
                                                                 ADI_WIL_EVENT_SEC_SESSION_MIC_FAILED,
                                                                 ADI_WIL_EVENT_SEC_M2M_JOIN_CNTR_ERROR,
                                                                 ADI_WIL_EVENT_SEC_M2M_SESSION_CNTR_ERROR,
                                                                 ADI_WIL_EVENT_SEC_CERTIFICATE_EXCHANGE_LOCK_ERROR,
                                                                 ADI_WIL_EVENT_MGR_QUEUE_OVERFLOW,
                                                                 ADI_WIL_EVENT_TOPOLOGY_ASSESSMENT_COMPLETE,
                                                                 ADI_WIL_EVENT_TOPOLOGY_APPLICATION_COMPLETE,
                                                                 ADI_WIL_EVENT_NODE_MODE_MISMATCH };

    /* Boolean value tracking if eNonSafetyEvent exists in
     * eValidNonSafetyEvents */
    bool bValid;

    /* Initialize local variables */
    bValid = false;

    /* Loop through all eValidNonSafetyEvents looking for input
     * eNonSafetyEvent */
    for (uint8_t i = 0u; i < (sizeof (eValidNonSafetyEvents) /  sizeof (eValidNonSafetyEvents [0])); i++)
    {
        /* If found, mark as valid and break */
        if (eValidNonSafetyEvents [i] == eNonSafetyEvent)
        {
            bValid = true;
            break;
        }
    }

    /* Check validity and pPack before dereferencing */
    if (bValid && ((void *) 0 != pPack))
    {
        /* Invoke event callback */
        adi_wil_HandleEvent (pPack, pPack->pClientData, eNonSafetyEvent, pData);
    }
}

void wb_wil_ui_GenerateFuSaCb (adi_wil_pack_t const * const pPack,
                               adi_wil_api_t eSafetyAPI,
                               adi_wil_err_t rc,
                               void const * const pData)
{
    /* List of APIs allowed to perform callbacks from a safety component */
    static const adi_wil_api_t eValidSafetyAPIs [] = { ADI_WIL_API_CONFIGURE_CELL_BALANCING,
                                                       ADI_WIL_API_GET_CELL_BALANCING_STATUS };

    /* Boolean value tracking if eSafetyAPI exists in eValidSafetyAPIs */
    bool bValid;

    /* Initialize local variables */
    bValid = false;

    /* Loop through all eValidSafetyAPIs looking for input eSafetyAPI */
    for (uint8_t i = 0u; i < (sizeof (eValidSafetyAPIs) / sizeof (eValidSafetyAPIs [0])); i++)
    {
        /* If found, mark as valid and break */
        if (eValidSafetyAPIs [i] == eSafetyAPI)
        {
            bValid = true;
            break;
        }
    }

    /* Check validity and pPack before dereferencing */
    if (bValid && ((void *) 0 != pPack))
    {
        /* Invoke API callback */
        adi_wil_HandleCallback (pPack, pPack->pClientData, eSafetyAPI, rc, pData);
    }
}

void wb_wil_ui_GenerateFuSaEvent (adi_wil_pack_t const * const pPack,
                                  adi_wil_event_id_t eSafetyEvent,
                                  void const * const pData)
{
    /* List of events allowed from a safety component */
    static const adi_wil_event_id_t eValidSafetyEvents [] = { ADI_WIL_EVENT_DATA_READY_BMS,
                                                              ADI_WIL_EVENT_DATA_READY_PMS,
                                                              ADI_WIL_EVENT_DATA_READY_EMS,
                                                              ADI_WIL_EVENT_CELL_BALANCING_STATUS,
                                                              ADI_WIL_EVENT_COMM_SAFETY_CPU_CONNECTED,
                                                              ADI_WIL_EVENT_COMM_SAFETY_CPU_DISCONNECTED,
                                                              ADI_WIL_EVENT_XMS_DUPLICATE,
                                                              ADI_WIL_EVENT_XMS_INSERTION,
                                                              ADI_WIL_EVENT_XMS_DELAY,
                                                              ADI_WIL_EVENT_XMS_DROPPED,
                                                              ADI_WIL_EVENT_FAULT_SAFETY_CPU,
                                                              ADI_WIL_EVENT_FAULT_SCL_TX_MESSAGE_EXPIRED,
                                                              ADI_WIL_EVENT_FAULT_SCL_VALIDATION,
                                                              ADI_WIL_EVENT_FAULT_LOCK_RELEASE,
                                                              ADI_WIL_EVENT_INSUFFICIENT_BUFFER };

    /* Boolean value tracking if eNonSafetyEvent exists in
     * eValidNonSafetyEvents */
    bool bValid;

    /* Initialize local variables */
    bValid = false;

    /* Loop through all eValidNonSafetyEvents looking for input
    * eNonSafetyEvent */
    for (uint8_t i = 0u; i < (sizeof (eValidSafetyEvents) / sizeof (eValidSafetyEvents [0])); i++)
    {
        /* If found, mark as valid and break */
        if (eValidSafetyEvents [i] == eSafetyEvent)
        {
            bValid = true;
            break;
        }
    }

    /* Check validity and pPack before dereferencing */
    if (bValid && ((void *) 0 != pPack))
    {
        /* Invoke event callback */
        adi_wil_HandleEvent (pPack, pPack->pClientData, eSafetyEvent, pData);
    }
}

adi_wil_err_t wb_wil_ui_AcquireLock (adi_wil_pack_t const * const pPack,
                                     void const * const pLockID)
{
    /* Call the common lock acquire indicating we are in non-fusa context */
    return wb_wil_ui_AcquireLockCommon (pPack, pLockID, false);
}

adi_wil_err_t wb_wil_ui_AcquireFuSaLock (adi_wil_pack_t const * const pPack,
                                         void const * const pLockID)
{
    /* Call the common lock acquire indicating we are in fusa context */
    return wb_wil_ui_AcquireLockCommon (pPack, pLockID, true);
}

void wb_wil_ui_ReleaseLock (adi_wil_pack_t const * const pPack,
                            void const * const pLockID)
{
    /* Call the common lock release indicating we are in non-fusa context */
    wb_wil_ui_ReleaseLockCommon (pPack, pLockID, false);
}

void wb_wil_ui_ReleaseFuSaLock (adi_wil_pack_t const * const pPack,
                                void const * const pLockID)
{
    /* Call the common lock release indicating we are in fusa context */
    wb_wil_ui_ReleaseLockCommon (pPack, pLockID, true);
}

/******************************************************************************
 * Local Function Definitions
 *****************************************************************************/

static void wb_wil_ui_Initialize (adi_wil_ui_internals_t * const pInternals)
{
    /* Initialize pack lock timer variables */
    pInternals->LockState.bAcquired = false;
    pInternals->LockState.bFusaContext = false;
    pInternals->LockState.iTimestamp = 0u;
}

static void wb_wil_ui_ActivateLock (adi_wil_ui_lock_state_t * const pState,
                                    bool bFusaContext)
{
    /* Store value indicating if the timer was started from the FuSa context */
    pState->bFusaContext = bFusaContext;

    /* Store current timestamp from the Ticker HAL */
    pState->iTimestamp = adi_wil_hal_TickerGetTimestamp ();

    /* Set timer state to acquired */
    pState->bAcquired = true;
}

static bool wb_wil_ui_DeactivateLock (adi_wil_ui_lock_state_t * const pState,
                                      bool bFusaContext)
{
    /* Storage for return code value indicating if the context of the lock
     * allowed for unlocking */
    bool bValid;

    /* Initialize local variables */
    bValid = true;

    /* Check that the context we are releasing from matches the context the
     * lock was acquired in */
    if (bFusaContext == pState->bFusaContext)
    {
        /* Set lock state to not acquired */
        pState->bAcquired = false;
    }
    /* ... Otherwise return failure */
    else
    {
        bValid = false;
    }

    return bValid;
}

static bool wb_wil_ui_CheckTimer (adi_wil_ui_lock_state_t * const pState,
                                  uint32_t iCurrentTime)
{
    /* Store elapsed time in ms from calculation */
    uint32_t iElapsedTime;

    /* Storage for return code value indicating if a lock timer is within
     * limit */
    bool bValid;

    /* Initialize local variables */
    bValid = true;

    /* Check if the lock is acquired before proceeding */
    if (pState->bAcquired)
    {
        /* Compute elapsed time in ms from the timer timestamp */
        iElapsedTime = (iCurrentTime - pState->iTimestamp);

        if (iElapsedTime < iCurrentTime)
        {
            /* CERT-C - Allow rollover as timers are expected to wrap */
        }

        /* Check if the lock has been held for longer than expected + FTDI.
         * Don't trigger a timeout if we've potentially underflowed by allowed
         * timer jitter */
        if ((iElapsedTime > (ADI_WIL_MAX_API_TIMEOUT + ADI_WIL_UI_LOCK_HOLD_TIMEOUT_FTDI)) &&
            (iElapsedTime < (UINT32_MAX - ADI_WIL_UI_TIMER_JITTER)))
        {
            bValid = false;

            /* Restart the timer by setting the timestamp to the current
             * time */
            pState->iTimestamp = iCurrentTime;
        }
    }

    /* Return status variable indicating if timer has exceeded limit */
    return bValid;
}

static adi_wil_err_t wb_wil_ui_AcquireLockCommon (adi_wil_pack_t const * const pPack,
                                                  void const * const pLockID,
                                                  bool bFusaContext)
{
    /* Method return code variable */
    adi_wil_err_t rc;

    /* OSAL API return code variable */
    adi_wil_osal_err_t eOSALrc;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_ui_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters. Clauses:
     * - pLockID cannot be NULL.
     * - If not acquiring global lock, pPack/safety internals can not be NULL
     * - If not acquiring global lock, pLockID must match pPack */
    if (((void *) 0 == pLockID) ||
        ((pLockID != pGlobalLockID) && (((void *) 0 == pInternals) || (pLockID != pPack))))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Attempt to acquire the lock from the OSAL */
        eOSALrc = adi_wil_osal_AcquireResource (pLockID);

        /* If we were able to acquire, set rc to success */
        if (ADI_WIL_OSAL_ERR_SUCCESS == eOSALrc)
        {
            rc = ADI_WIL_ERR_SUCCESS;

            /* If we acquired the pack lock, start the pack lock timer */
            if (pLockID != pGlobalLockID)
            {
                wb_wil_ui_ActivateLock (&pInternals->UI.LockState, bFusaContext);
            }
            /* ..else we acquired the global lock, start the global lock
             * timer */
            else
            {
                wb_wil_ui_ActivateLock (&GlobalLockState, bFusaContext);
            }
        }
        /* ...else if the lock has not been created, set rc to not connected */
        else if (ADI_WIL_OSAL_ERR_NO_RESOURCE == eOSALrc)
        {
            rc = ADI_WIL_ERR_NOT_CONNECTED;
        }
        /* ...else another API already owns the lock, set rc to in progress */
        else
        {
            rc = ADI_WIL_ERR_API_IN_PROGRESS;
        }
    }

    /* Return error code to caller */
    return rc;
}

static void wb_wil_ui_ReleaseLockCommon (adi_wil_pack_t const * const pPack,
                                         void const * const pLockID,
                                         bool bFusaContext)
{
    /* Storage for return code value indicating if a lock was able to be
     * released */
    bool bValid;

    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Initialize local variables */
    bValid = false;

    /* Retrieve safety internals pointer */
    pInternals = wb_ui_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters - must either match pack or global lock */
    if ((pLockID != pPack) &&
        (pLockID != pGlobalLockID))
    {
        /* Do nothing - invalid release request */
    }
    /* If we are releasing the pack lock, stop the pack lock timer */
    else if (pLockID == pPack)
    {
        /* Validate pack instance before dereferencing */
        if ((void *) 0 != pInternals)
        {
            /* Attempt to deactivate the pack lock */
            bValid = wb_wil_ui_DeactivateLock (&pInternals->UI.LockState, bFusaContext);
        }
    }
    /* If we are releasing the global lock, stop the global lock timer */
    else
    {
        /* Attempt to deactivate the global lock */
        bValid = wb_wil_ui_DeactivateLock (&GlobalLockState, bFusaContext);
    }

    /* If we're in a valid state, try and release the resource */
    if (bValid)
    {
        /* Call for OSAL lock release and check for success */
        bValid = (adi_wil_osal_ReleaseResource (pLockID) == ADI_WIL_OSAL_ERR_SUCCESS);
    }

    /* If we're no longer in a valid state, generate a fault notification */
    if (!bValid)
    {
        wb_wil_ui_GenerateFuSaEvent (pPack, ADI_WIL_EVENT_FAULT_LOCK_RELEASE, (void *) 0);
    }
}

static void wb_wil_ui_Process (adi_wil_pack_t const * const pPack,
                               uint32_t iCurrentTicks)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Retrieve safety internals pointer */
    pInternals = wb_ui_GetSafetyInternalsPointer (pPack);

    /* Validate internals instance before dereferencing */
    if ((void *) 0 != pInternals)
    {
        /* Check pack lock state */
        if (!wb_wil_ui_CheckTimer (&pInternals->UI.LockState, iCurrentTicks))
        {
            /* Generate event to user to signal the lock release has timed
             * out */
            wb_wil_ui_GenerateFuSaEvent (pPack, ADI_WIL_EVENT_FAULT_LOCK_RELEASE, (void *) 0);
        }
    }

    /* Check global lock state */
    if (!wb_wil_ui_CheckTimer (&GlobalLockState, iCurrentTicks))
    {
        /* Generate event to user to signal the lock release has timed out */
        wb_wil_ui_GenerateFuSaEvent ((void *) 0, ADI_WIL_EVENT_FAULT_LOCK_RELEASE, (void *) 0);
    }
}

static adi_wil_safety_internals_t * wb_ui_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack)
{
    /* Return value of this method */
    adi_wil_safety_internals_t * pInternals;

    /* Initialize local variable to NULL */
    pInternals = (void *) 0;

    /* Check pack instance before dereferencing and ensure we're not
     * dereferencing the global lock ID */
    if (((void *) 0 != pPack) &&
        ((void *) 0 != pGlobalLockID) &&
        (pPack != pGlobalLockID))
    {
        /* Assign safety internals pointer to local variable */
        pInternals = pPack->pSafetyInternals;
    }

    /* Return local variable pointer to safety internals */
    return pInternals;
}

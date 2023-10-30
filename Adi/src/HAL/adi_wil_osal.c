/*******************************************************************************
 * @brief    Operating System Abstraction Layer (OSAL)
 *
 * @details  Contains HAL OSAL functions, making use of global variables
 *           instead of mutexes or semaphores to manage shared resources.
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include "adi_wil_osal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "adi_wil_example_debug_functions.h"

// Maxinum number of semaphores to be supported.
// Equivalent to max number of packs + 1 semaphore for QueryDevice. */
#define MAX_NUM_SEM 2u

// Define a structure to hold the WIL semaphore ID and corresponding semaphore handle.
typedef struct {
    void const * pPack;             /* WIL resource ID and pack handle              */
    bool bResourceAcquiredEn;       /* Boolean flag to avoid using semaphores       */
} adi_wil_sem_t;

static adi_wil_sem_t osal_sem[MAX_NUM_SEM];

volatile bool bgResourceAcquired[MAX_NUM_SEM] = {false};


adi_wil_osal_err_t adi_wil_osal_CreateResource(void const * const pPack)
{
    bool bSemExist = false;
    uint8_t i, iFreeSlot = 0;
    adi_wil_osal_err_t err = ADI_WIL_OSAL_ERR_FAIL;

    /* Find whether the given resource already exists */
    for(i = 0; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn == true) && ( osal_sem[i].pPack == pPack))
        {
            /* The semaphore already exists */
            bSemExist = true;
            err = ADI_WIL_OSAL_ERR_SUCCESS;
            break;
        }
        else if(osal_sem[i].bResourceAcquiredEn == false)
        {
            /* Pick a free slot if the resource hasn't already been acquired */
            iFreeSlot = i;
            break;
        }
    }

    if((bSemExist == false) && (i != MAX_NUM_SEM))
    {
        /* No need to create a semaphore as boolean*/
        /* Need to create a semaphore or equivalent artefact    */
        /* Save the semaphore handle and the Res ID             */
        osal_sem[iFreeSlot].pPack = pPack;
        osal_sem[iFreeSlot].bResourceAcquiredEn = true;
        bgResourceAcquired[iFreeSlot] = false;

        err = ADI_WIL_OSAL_ERR_SUCCESS;
    }
    return err;
}


adi_wil_osal_err_t adi_wil_osal_AcquireResource(void const * const pPack)
{
    bool bFoundId = false;

    /* Find the semaphore handle with the given Res Id */
    for(uint8_t i = 0; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn == true) && (osal_sem[i].pPack == pPack))
        {
           bgResourceAcquired[i] = true;
           bFoundId = true;
           break;
        }
    }

    return (bFoundId == true) ? ADI_WIL_OSAL_ERR_SUCCESS : ADI_WIL_OSAL_ERR_FAIL;
}


adi_wil_osal_err_t adi_wil_osal_ReleaseResource(void const * const pPack)
{
    bool bFoundId = false;

    /* Find the semaphore handle with the given Res Id */
    for(uint8_t i = 0; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn == true) && (osal_sem[i].pPack == pPack))
        {
           bgResourceAcquired[i] = false;
           bFoundId = true;
           break;
        }
    }
    return (bFoundId == true) ? ADI_WIL_OSAL_ERR_SUCCESS : ADI_WIL_OSAL_ERR_FAIL;
}


adi_wil_osal_err_t adi_wil_osal_DestroyResource(void const * const pPack)
{
    bool bFoundId = false;

    /* Find the semaphore handle with the given Res Id */
    for(uint8_t i = 0; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn == true) && (osal_sem[i].pPack == pPack))
        {
           osal_sem[i].bResourceAcquiredEn = false;
           bgResourceAcquired[i] = false;
           bFoundId = true;
           break;
        }
    }
    return (bFoundId == true) ? ADI_WIL_OSAL_ERR_SUCCESS : ADI_WIL_OSAL_ERR_FAIL;
}

/**
 * @brief   Wait for resource to be available.
  *
 * @details This function waits after invoking a WIL blocking call.
 *
 * @param  pPack                The ID of the resource indicating the resource to destroy.
 *                              This argument is used to identify the resource as
 *                              well as the pack that is requesting to destory the resource.
 *
 * @return adi_wil_osal_err_t   Result from the resource destruction.
 *                              Return ADI_WIL_OSAL_ERR_SUCCESS if the resource has
 *                              been destroyed, ADI_WIL_OSAL_ERR_FAIL otherwise.
 */
void WaitForWilAPI(void const * const pPack)
{
    /* As this is a single threaded baremetal implementation there is not need
       to implement a mechanism to acquire and release a semaphore. Instead
       a global variable is used.
    */

    for(uint8_t i = 0u; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn) && (osal_sem[i].pPack == pPack))
        {
            while(bgResourceAcquired[i])
            {
                /* spin wait */
            };
        }
    }
}

boolean IsReleaseWilAPI(void const * const pPack)
{
    /* As this is a single threaded baremetal implementation there is not need
       to implement a mechanism to acquire and release a semaphore. Instead
       a global variable is used.
    */
    boolean		bRetValue = FALSE;

    for(uint8_t i = 0u; i < MAX_NUM_SEM; i++)
    {
        if((osal_sem[i].bResourceAcquiredEn) && (osal_sem[i].pPack == pPack))
        {
        	if (!bgResourceAcquired[i])
				bRetValue = TRUE;       
        }
    }
	return bRetValue;
}


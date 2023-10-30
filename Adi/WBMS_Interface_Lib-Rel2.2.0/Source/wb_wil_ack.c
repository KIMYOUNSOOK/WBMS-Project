/******************************************************************************
 * @file     wb_wil_ack.c
 *
 * @brief    Queue to add pending acknowledgments to be sent to the wBMS
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_ack.h"
#include "wb_wil_utils.h"

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static uint8_t wb_wil_ack_Mask (uint8_t val);

/******************************************************************************
 * Function definitions
 *****************************************************************************/

adi_wil_err_t wb_wil_ack_Put (adi_wil_ack_queue_t * const pQueue,
                              uint16_t iValue,
                              uint8_t iCommandId)
{
    adi_wil_err_t rc;

    /* Validate input parameters */
    if (pQueue == (void *) 0)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Check for buffer fullness */

    else if (ADI_WIL_ACK_QUEUE_COUNT == ((pQueue->iHead - pQueue->iTail) & (uint8_t) 0xFFu))
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else
    {
        /* Buffer not full, store the node at the head */
        pQueue->iValue [wb_wil_ack_Mask (pQueue->iHead)] = iValue;

        /* Buffer not full, store the node at the head */
        pQueue->iCommandId [wb_wil_ack_Mask (pQueue->iHead)] = iCommandId;

        /* CERT-C Precondition check on parameters */
        if (0xFFu == pQueue->iHead)
        {
            /* Do nothing - expect rollover */
        }

        /* Increment count of queued messages */
        ++pQueue->iHead;

        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

adi_wil_err_t wb_wil_ack_Get (adi_wil_ack_queue_t * const pQueue,
                              uint16_t * const pValue,
                              uint8_t * const pCommandId)
{
    adi_wil_err_t rc;

    /* Validate input parameters */
    if ((pQueue == (void *) 0) ||
        (pValue == (void *) 0) ||
        (pCommandId == (void *) 0))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Check for buffer emptiness */
    else if (((pQueue->iHead - pQueue->iTail) & (uint8_t) 0xFFu) == 0u)
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    /* Buffer not empty retrieve the node at the head and increment tail pointer */
    else
    {
        *pValue = pQueue->iValue [wb_wil_ack_Mask (pQueue->iTail)];
        *pCommandId = pQueue->iCommandId [wb_wil_ack_Mask (pQueue->iTail)];

        /* CERT-C Precondition check on parameters */
        if (0xFFu == pQueue->iTail)
        {
            /* Do nothing - expect rollover */
        }
        
        /* Increment count of retrieved messages */
        ++pQueue->iTail;

        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

static uint8_t wb_wil_ack_Mask (uint8_t val)
{
    /* Return the correct value after masking out */
    return val & (ADI_WIL_ACK_QUEUE_COUNT - 1u);
}

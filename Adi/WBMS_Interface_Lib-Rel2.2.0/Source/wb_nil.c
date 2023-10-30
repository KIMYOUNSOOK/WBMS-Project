/******************************************************************************
 * @file     wb_nil.c
 *
 * @brief    NIL source code
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

/* Function declarations header */
#include "wb_nil.h"

/* External module headers */
#include "adi_wil_hal_spi.h"
#include "adi_wil_hal_ticker.h"
#include "adi_wil_hal_tmr.h"
#include "wb_nil_packet.h"
#include "wb_wil_utils.h"
#include "wb_wil_query_device.h"
#include "wb_wil_ack.h"
#include "wb_wil_error_notify.h"
#include "wb_crc_32.h"
#include "wb_crc_config.h"

/* Structure definition headers */
#include "adi_wil_port.h"

/* Standard library headers */
#include <string.h>

/******************************************************************************
 *  Defines
 *****************************************************************************/

/* Number of consecutive frames in error before logging out of a session with manager */
#define WB_NIL_LINK_LOST_DETECTION_THRESHOLD    (200u)

/* Number of logged out frames received from manager, before signaling a link up event */
#define WB_NIL_LINK_UP_DETECTION_THRESHOLD      (20u)

/* Number of ports that a user can add to the WIL */
#define ADI_WIL_MAX_PORTS                       (16u)

/******************************************************************************
 * Static variables
 *****************************************************************************/

/* List of ports in the system */
static adi_wil_port_t * DeviceList [ADI_WIL_MAX_PORTS];

/* List of booleans indicating which DeviceList entries are in-use in the system */
static bool bInUseList [ADI_WIL_MAX_PORTS];

/* Volatile boolean used to switch on/off ISR activity */
static volatile bool bIsrDisabled;

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

/* Application context functions */

static adi_wil_err_t wb_nil_AddPort (adi_wil_port_t * const pPort);

static bool wb_nil_RemovePort (adi_wil_port_t const * const pPort);

/* Timer Callback context functions */

static void wb_nil_TimerCb (void);

static void wb_nil_TransmitOnAvailableDevices (void);

static bool wb_nil_CheckTransmittedList (uint8_t const * const pTransmittedList,
                                         uint32_t iTransmittedCount,
                                         uint8_t iSPIDevice);

static void wb_nil_Transmit (adi_wil_port_t * const pPort);

static void wb_wil_SetupTxTransmission (adi_wil_port_internal_t * const pInternals);

static void wb_wil_SetupRxTransmission (adi_wil_port_internal_t * const pInternals);

static void wb_nil_CheckTransmissibleStatus (void);

static void wb_nil_CheckSPIForTransmissibleStatusReset (uint8_t iSPIDevice);

/* SPI Callback context functions */

static void wb_nil_TxDoneCb (uint8_t iSPIDevice,
                             uint8_t iChipSelect);

static void wb_nil_MarkTxFrameAsTransmitted (adi_wil_port_internal_t * const pInternals);

static void wb_nil_MarkRxFrameForProcessing (adi_wil_port_internal_t * const pInternals);

/* Process Task context functions */

static void wb_nil_ReadFrame (adi_wil_port_t * const pPort,
                              uint8_t * const pData);

static bool wb_nil_ValidateFrameMetadata (uint8_t const * const pData,
                                          uint8_t iPayloadLength);

static void wb_nil_ProcessFramePayload (adi_wil_port_t * const pPort,
                                        uint8_t iPayloadLength,
                                        uint8_t * const pData,
                                        bool bLoggedInFrame);

static void wb_nil_ReleaseFrame (adi_wil_port_t * const pPort,
                                 uint8_t const * const pData);

static void wb_nil_CheckLinkStatus (adi_wil_port_t * const pPort,
                                    adi_wil_port_internal_t * const pInternals,
                                    adi_wil_port_stats_t * const pStatistics);

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

adi_wil_err_t wb_nil_Initialize (void)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Set the ISR status to disabled as we don't want to */
    bIsrDisabled = true;

    /* Initialize the timer */
    if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TickerInit ())
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    /* Initialize the ticker */
    else if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TmrInit ())
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else
    {
        /* Initialize the list of in-use flags */
        (void) memset (&bInUseList [0], 0, sizeof (bInUseList));

        /* Start the timer */
        if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TmrStart (&wb_nil_TimerCb))
        {
            rc = ADI_WIL_ERR_FAIL;
        }
        /* Start the ticker */
        else if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TickerStart ())
        {
            rc = ADI_WIL_ERR_FAIL;
        }
        /* If all above steps were successful, set the response code to success */
        else
        {
            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    /* Re-enable ISRs */
    bIsrDisabled = false;

    /* Return the response code */
    return rc;
}

adi_wil_err_t wb_nil_Terminate (void)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Disable ISRs */
    bIsrDisabled = true;

    /* Stop the timer */
    if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TickerStop ())
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    /* Stop the ticker */
    else if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_TmrStop ())
    {
        rc = ADI_WIL_ERR_FAIL;
    }
    else
    {
        /* Zero the list of in-use flags */
        (void) memset (&bInUseList [0], 0, sizeof (bInUseList));

        /* Set the return code to success */
        rc = ADI_WIL_ERR_SUCCESS;
    }

    /* Return the response code */
    return rc;
}

adi_wil_err_t wb_nil_Process (adi_wil_port_t * const pPort)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    if ((void *) 0 == pPort)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Loop through and process all Rx buffers that are ready for processing */
        for (uint8_t i = 0u; i < ADI_WIL_PORT_RX_FRAME_COUNT; i++)
        {
            if (pPort->Internals.RxBuffer [i].bReadyForProcessing)
            {
                /* Read the frame */
                wb_nil_ReadFrame (pPort, &pPort->Internals.RxBuffer [i].iData [0]);

                /* Release the frame */
                wb_nil_ReleaseFrame (pPort, &pPort->Internals.RxBuffer [i].iData [0]);
            }
        }

        /* Check the link is still active */
        wb_nil_CheckLinkStatus (pPort,
                                &pPort->Internals,
                                &pPort->Internals.PortStatistics);

        /* Set the return code to success*/
        rc = ADI_WIL_ERR_SUCCESS;
    }

    return rc;
}

void wb_nil_ProcessAllPorts (void)
{
    /* Loop through all ports in the list */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* If an entry is non-null and in-use, select it */
        if (bInUseList [i] && ((void *) 0 != DeviceList [i]))
        {
            /* If an entry is currently performing a QueryDevice operation, process it */
            if (DeviceList [i]->Internals.QueryDeviceState.bInProgress)
            {
                /* Process any Rx frames */
                (void) wb_nil_Process (DeviceList [i]);

                /* Check for any query device timeouts */
                wb_wil_QueryDeviceCheckTimeout (DeviceList [i]);
            }
        }
    }
}

adi_wil_err_t wb_nil_SubmitFrame (adi_wil_port_t * const pPort,
                                  wb_pack_element_t const * const pFrame)
{
    /* Value for storing calculated CRC */
    uint32_t iCRC;

    /* Return value of this function */
    adi_wil_err_t rc;

    if (((void *) 0 == pPort) || ((void *) 0 == pFrame))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else if ((pFrame->size > WBMS_SPI_TRANSACTION_SIZE) ||
             (pFrame->data > WBMS_FRAME_PAYLOAD_MAX_SIZE))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Write the payload length field */
        pFrame->origin [0u] = (uint8_t) (pFrame->data & 0xFFu);

        /* Write the session ID field */
        pFrame->origin [WBMS_FRAME_SESSION_ID_OFFSET] = pPort->Internals.iSessionId;

        /* Calculate the CRC over the header and payload */
        iCRC = wb_crc_ComputeCRC32 (&pFrame->origin [0u],
                                    (uint32_t) pFrame->data + WBMS_FRAME_HDR_LEN,
                                    WB_CRC_SEED);

        /* Write the 32-bit CRC value */
        pFrame->origin [WBMS_FRAME_CRC_OFFSET + 0u] = (uint8_t) (((iCRC & 0xFF000000u) >> 24u) & 0xFFu);
        pFrame->origin [WBMS_FRAME_CRC_OFFSET + 1u] = (uint8_t) (((iCRC & 0x00FF0000u) >> 16u) & 0xFFu);
        pFrame->origin [WBMS_FRAME_CRC_OFFSET + 2u] = (uint8_t) (((iCRC & 0x0000FF00u) >> 8u) & 0xFFu);
        pFrame->origin [WBMS_FRAME_CRC_OFFSET + 3u] = (uint8_t) (((iCRC & 0x000000FFu) >> 0u) & 0xFFu);

        /* If the frame is a user request frame, mark it as ready for transmission */
        if (pFrame->origin == &pPort->Internals.UserRequestFrame [0])
        {
            pPort->Internals.bUserRequestFramePending = true;
        }
        /* ...else it's a process task frame, mark that instead */
        else
        {
            pPort->Internals.bProcessTaskRequestFramePending = true;
        }

        /* Set return value to success */
        rc = ADI_WIL_ERR_SUCCESS;
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_nil_InitPort (adi_wil_pack_internals_t * const pInternals,
                               adi_wil_port_t * const pPort)
{
    adi_wil_err_t rc;

    /* Validate input parameters and don't check pack pointer for NULL here as
     * QueryDevice calls with a NULL pointer */
    if ((void *) 0 == pPort)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        (void) memset (&pPort->Internals, 0, sizeof (pPort->Internals));

        pPort->Internals.pPackInternals = pInternals;
        pPort->Internals.iSessionId = WBMS_SPI_LOGGED_OUT_SESSION_ID;
        pPort->Internals.IdleFrame [WBMS_FRAME_SESSION_ID_OFFSET] = WBMS_SPI_LOGGED_OUT_SESSION_ID;

        /* Stop the timer */
        (void) adi_wil_hal_TmrStop ();

        /* Add the port to the device list */
        rc = wb_nil_AddPort (pPort);

        /* Restart the timer */
        (void) adi_wil_hal_TmrStart (&wb_nil_TimerCb);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_nil_ClosePort (adi_wil_port_t const * const pPort)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input parameters */
    if (pPort == (void *) 0)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Stop the timer */
        (void) adi_wil_hal_TmrStop ();

        /* Remove the port from the device list. Set the rc to indicate if we
         * found and removed the port */
        rc = wb_nil_RemovePort (pPort) ? ADI_WIL_ERR_SUCCESS : ADI_WIL_ERR_FAIL;

        /* Restart the timer */
        (void) adi_wil_hal_TmrStart (&wb_nil_TimerCb);
    }

    /* Return response code */
    return rc;
}

/******************************************************************************
* Static functions
******************************************************************************/

static void wb_nil_TxDoneCb (uint8_t iSPIDevice, uint8_t iChipSelect)
{
    /* Loop through all slots to find this port...  */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check if this slot is set... */
        if (bInUseList [i] && ((void *) 0 != DeviceList [i]))
        {
            /* If it is, check if it shares a SPI device and a CS... */
            if ((DeviceList [i]->iSPIDevice == iSPIDevice) &&
                (DeviceList [i]->iChipSelect == iChipSelect))
            {
                /* Mark the Tx frame as transmitted */
                wb_nil_MarkTxFrameAsTransmitted (&DeviceList [i]->Internals);

                /* Mark the Rx frame as ready for processing */
                wb_nil_MarkRxFrameForProcessing (&DeviceList [i]->Internals);

                /* Clear port in use flag */
                DeviceList [i]->Internals.bInUse = false;

                /* Tasks completed - break out of the loop */
                break;
            }
        }
    }
}

static void wb_nil_MarkTxFrameAsTransmitted (adi_wil_port_internal_t * const pInternals)
{
    /* Check if we just sent a user frame. If so, mark as no longer pending */
    if (pInternals->pTx == &pInternals->UserRequestFrame [0])
    {
        pInternals->bUserRequestFramePending = false;
    }
    /* ...else, Check if we just sent a process task frame. If so, mark as
     * no longer pending */
    else if (pInternals->pTx == &pInternals->ProcessTaskRequestFrame [0])
    {
        pInternals->bProcessTaskRequestFramePending = false;
    }
    else
    {
        /* Do nothing, idle frame was sent */
    }
}

static void wb_nil_MarkRxFrameForProcessing (adi_wil_port_internal_t * const pInternals)
{
    /* Loop through all Rx frames */
    for (uint8_t i = 0u; i < ADI_WIL_PORT_RX_FRAME_COUNT; i++)
    {
        /* Check if this frame is the buffer we just used... */
        if (pInternals->pRx == &pInternals->RxBuffer [i].iData [0])
        {
            /* If it is, mark as ready for processing */
            pInternals->RxBuffer [i].bReadyForProcessing = true;

            /* Break out of the loop */
            break;
        }
    }
}

static void wb_nil_CheckLinkStatus (adi_wil_port_t * const pPort,
                                    adi_wil_port_internal_t * const pInternals,
                                    adi_wil_port_stats_t * const pStatistics)
{
    /* Variable used for storing awake link calculation result */
    uint32_t iLinkUpFrames;

    /* Add together the statistics used to detect an awake link */
    iLinkUpFrames = (pStatistics->iRunningLoggedOutFrameCount + pStatistics->iRunningValidFrameCount);

    /* Ensure adding the two 32-bit integers didn't overflow */
    if (iLinkUpFrames < pStatistics->iRunningLoggedOutFrameCount)
    {
        iLinkUpFrames = (uint32_t) UINT32_MAX;
    }

    /* Check Rx frame error count, if it exceeds the threshold and the current link is up, signal a link lost event */
    /* Check if we missed a LINK LOST event, so log it now */
    if ((pInternals->bLinkAvailable && (pStatistics->iRunningRxErrorCount > WB_NIL_LINK_LOST_DETECTION_THRESHOLD)) ||
        (pInternals->bConnected && (pStatistics->iRunningLoggedOutFrameCount > WB_NIL_LINK_LOST_DETECTION_THRESHOLD)))
    {
        pStatistics->iRunningRxErrorCount = 0u;
        pStatistics->iRunningLoggedOutFrameCount = 0u;
        pStatistics->iRunningValidFrameCount = 0u;
        pInternals->bLinkAvailable = false;
        wb_wil_HandlePortDownEvent (pInternals->pPackInternals, pPort);
    }
    /* ...else, check if manager link is now available */
    else if (!pInternals->bLinkAvailable && (iLinkUpFrames > WB_NIL_LINK_UP_DETECTION_THRESHOLD))
    {
        pStatistics->iRunningRxErrorCount = 0u;
        pStatistics->iRunningLoggedOutFrameCount = 0u;
        pStatistics->iRunningValidFrameCount = 0u;
        pInternals->bLinkAvailable = true;
        wb_wil_HandlePortAvailableEvent (pInternals->pPackInternals, pPort);
    }
    else
    {
        /* MISRA else */
    }
}

static adi_wil_err_t wb_nil_AddPort (adi_wil_port_t * const pPort)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Boolean to indicate that no previous port is using this SPI device */
    bool bNewSPIDevice;

    /* Initialize boolean for a new SPI device to true */
    bNewSPIDevice = true;

    /* Initialize response code to failure for if a free slot is not found */
    rc = ADI_WIL_ERR_FAIL;

    /* Loop through all slots to find if this SPI device is already initialized */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check if this slot is set... */
        if (bInUseList [i] && ((void *) 0 != DeviceList [i]))
        {
            /* If it is, check if it shares a SPI device... */
            if (DeviceList [i]->iSPIDevice == pPort->iSPIDevice)
            {
                /* If existing, set the flag to false so we don't init it again */
                bNewSPIDevice = false;

                /* Device found, break out of the loop */
                break;
            }
        }
    }

    /* Loop through all slots */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check if this slot is free. Don't check for an existing entry in
         * the device list as we want to overwrite any not in-use */
        if (!bInUseList [i])
        {
            /* Free slot found, set RC to success */
            rc = ADI_WIL_ERR_SUCCESS;

            /* If its a new SPI device, attempt to init it... */
            if (bNewSPIDevice)
            {
                /* If initialization fails, set RC to external failure */
                if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_SpiInit (pPort->iSPIDevice, &wb_nil_TxDoneCb))
                {
                    rc = ADI_WIL_ERR_EXTERNAL;
                }
            }
            /* If we're still successful, store the port in the list and mark
             * the entry as in-use */
            if (ADI_WIL_ERR_SUCCESS == rc)
            {
                DeviceList [i] = pPort;
                bInUseList [i] = true;
            }

            /* Break out of for loop as we've finished looking for a slot */
            break;
        }
    }

    /* Return the response code */
    return rc;
}

static bool wb_nil_RemovePort (adi_wil_port_t const * const pPort)
{
    /* Boolean used to track if another port is still using this SPI device */
    bool bCloseSPI;

    /* Boolean used to track if we found and removed our SPI device*/
    bool bFound;

    /* Initialize to assume there is no other device */
    bCloseSPI = true;

    /* Initialize to not having found our port yet */
    bFound = false;

    /* Loop through all slots to find this port...  */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check if this slot is set... */
        if (bInUseList [i] && ((void *) 0 != DeviceList [i]))
        {
            /* If it is, check if it shares a SPI device and a CS... */
            if ((DeviceList [i]->iSPIDevice == pPort->iSPIDevice) &&
                (DeviceList [i]->iChipSelect != pPort->iChipSelect))
            {
                /* If another port is using the same SPI device, don't close */
                bCloseSPI = false;
            }

            if (pPort == DeviceList [i])
            {
                /* Mark this entry as no - longer in use. Don't clear
                 * the entry to protect from race condition */
                bInUseList [i] = false;
                bFound = true;
            }

            /* If we've found and removed our entry and we've also found a port
             * using the same SPI device, we can break out early. */
            if (bFound && !bCloseSPI)
            {
                break;
            }
        }
    }

    /* If the device was found and was the only port using this device,
     * close the SPI port */
    if (bFound && bCloseSPI)
    {
        (void) adi_wil_hal_SpiClose (pPort->iSPIDevice);
    }

    return bFound;
}

bool wb_nil_CheckForPort (adi_wil_port_t const * const pPort)
{
    /* Return value of this function */
    bool bExists;

    /* Initialize variable to assume port doesn't exist in list */
    bExists = false;

    /* Validate input parameters */
    if ((void *) 0 != pPort)
    {
        /* Loop through all slots to find this port...  */
        for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
        {
            /* Check if we've found a match... */
            if (bInUseList [i] && (pPort == (adi_wil_port_t const *) DeviceList [i]))
            {
                /* We've found a match - flag and break out of loop */
                bExists = true;
                break;
            }
        }
    }
    /* Return boolean indicating if we found the device in the list */
    return bExists;
}

void wb_nil_Login (adi_wil_port_t * const pPort, uint8_t iSessionId)
{
    /* Validate input parameters */
    if ((void *) 0 != pPort)
    {
        /* Reset error counters used for link detection */
        pPort->Internals.PortStatistics.iRunningRxErrorCount = 0u;
        pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount = 0u;
        pPort->Internals.PortStatistics.iRunningValidFrameCount = 0u;

        /* Set the port members to the logged in values */
        pPort->Internals.iSessionId = iSessionId;
        pPort->Internals.bConnected = true;
        pPort->Internals.IdleFrame [WBMS_FRAME_SESSION_ID_OFFSET] = iSessionId;
    }
}

void wb_nil_Logout (adi_wil_port_t * const pPort)
{
    /* Validate input parameters */
    if ((void *) 0 != pPort)
    {
        /* Reset error counters used for link detection */
        pPort->Internals.PortStatistics.iRunningRxErrorCount = 0u;
        pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount = 0u;
        pPort->Internals.PortStatistics.iRunningValidFrameCount = 0u;

        /* Set the port members to the logged out values */
        pPort->Internals.iSessionId = WBMS_SPI_LOGGED_OUT_SESSION_ID;
        pPort->Internals.bConnected = false;
    }
}

static void wb_nil_Transmit (adi_wil_port_t * const pPort)
{
    /* Check if we're able to submit a SPI transaction on this port */
    if (!pPort->Internals.bInUse)
    {
        /* Flag the port as currently in use */
        pPort->Internals.bInUse = true;

        /* Set up the Tx pointer for transmission */
        wb_wil_SetupTxTransmission (&pPort->Internals);

        /* Set up the Rx pointer for transmission */
        wb_wil_SetupRxTransmission (&pPort->Internals);

        /* Check we were able to allocate an Rx buffer */
        if ((void *) 0 == pPort->Internals.pRx)
        {
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRxBuffAllocErrCount);

            /* Clear port in use flag */
            pPort->Internals.bInUse = false;
        }
        else if (ADI_WIL_HAL_ERR_SUCCESS != adi_wil_hal_SpiTransmit (pPort->iSPIDevice,
                                                                        pPort->iChipSelect,
                                                                        pPort->Internals.pTx,
                                                                        pPort->Internals.pRx,
                                                                        WBMS_SPI_TRANSACTION_SIZE))
        {
            /* Transmission failed - release the allocated frame */
            wb_nil_ReleaseFrame (pPort, pPort->Internals.pRx);

            /* Increment the transmission error count */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iTxErrorCount);

            /* Clear port in use flag */
            pPort->Internals.bInUse = false;
        }
        else
        {
            /* Increment the transmission count */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iTxFrameCount);

            /* Flag this port as having successfully transmitted */
            pPort->Internals.bTransmitted = true;

            /* We've sent at least one frame - mark this port as initialized */
            pPort->Internals.bInitialized = true;
        }
    }
}

static void wb_wil_SetupTxTransmission (adi_wil_port_internal_t * const pInternals)
{
    if (!pInternals->bInitialized)
    {
        /* Always send an idle frame as the first frame to re-sync with manager SPI */
        pInternals->pTx = &pInternals->IdleFrame [0];
    }
    else if (pInternals->bUserRequestFramePending)
    {
        /* If both process task and user frames are queued... */
        if (pInternals->bProcessTaskRequestFramePending)
        {
            /* If the previous request was a user request frame, queue a process task frame. Otherwise, queue a user frame this time */
            pInternals->pTx = pInternals->bPreviousTxWasUserFrame ? &pInternals->ProcessTaskRequestFrame [0] : &pInternals->UserRequestFrame [0];

            /* Set the flag to indicate which of the frames we are transmitting */
            pInternals->bPreviousTxWasUserFrame = (pInternals->pTx == &pInternals->UserRequestFrame [0]);
        }
        else
        {
            /* Only a user frame is queued - use it */
            pInternals->pTx = &pInternals->UserRequestFrame [0];

            /* Set the flag to indicate we are transmitting a user frame */
            pInternals->bPreviousTxWasUserFrame = true;
        }
    }
    else if (pInternals->bProcessTaskRequestFramePending)
    {
        /* Only a process task frame is queued - use it*/
        pInternals->pTx = &pInternals->ProcessTaskRequestFrame [0];

        /* Clear the flag as we are transmitting a process task frame */
        pInternals->bPreviousTxWasUserFrame = false;
    }
    else
    {
        /* No frames are queued, fall back to an idle frame */
        pInternals->pTx = &pInternals->IdleFrame [0];
    }
}

static void wb_wil_SetupRxTransmission (adi_wil_port_internal_t * const pInternals)
{
    /* Initialize Rx buffer pointer to NULL */
    pInternals->pRx = (void *) 0;

    /* Loop through and process all Rx buffers... */
    for (uint8_t i = 0u; i < ADI_WIL_PORT_RX_FRAME_COUNT; i++)
    {
        /* Check if this buffer is free */
        if (!pInternals->RxBuffer [i].bInUse)
        {
            /* If it is, mark it as now in use */
            pInternals->RxBuffer [i].bInUse = true;

            /* Assign the Rx pointer to this value */
            pInternals->pRx = &pInternals->RxBuffer [i].iData [0];

            /* Rx buffer found - break out of loop*/
            break;
        }
    }
}

static void wb_nil_ReadFrame (adi_wil_port_t * const pPort,
                              uint8_t * const pData)
{
    /* Storage for extracted payload length field */
    uint8_t iPayloadLength;

    /* Storage for extracted session ID field */
    uint8_t iSessionId;

    /* Extract payload length and session ID */
    iPayloadLength = pData [0];
    iSessionId = pData [WBMS_FRAME_SESSION_ID_OFFSET];

    /* Check if the frame is an idle frame... */
    if (iPayloadLength == 0u)
    {
        if (iSessionId == pPort->Internals.iSessionId)
        {
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningValidFrameCount);

            /* Valid frame received, so reset running error counter */
            pPort->Internals.PortStatistics.iRunningRxErrorCount = 0u;
        }
        else if (iSessionId == 0u)
        {
            /* Increment the frame error count */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRxErrorCount);

            /* Invalid frame metadata - Increment the error counters */
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningRxErrorCount);
        }
        else
        {
            wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount);
        }
    }
    /* else, check the metadata... */
    else if (!wb_nil_ValidateFrameMetadata (pData, iPayloadLength))
    {
        /* Increment the frame error count */
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRxErrorCount);

        /* Invalid frame metadata - Increment the error counters */
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningRxErrorCount);
    }
    /* else, valid metadata - check the session ID for a "logged out" session ID */
    else if (iSessionId == WBMS_SPI_LOGGED_OUT_SESSION_ID)
    {
        /* Logged out session ID - increment the counters  */
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount);
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRxFrameCount);

        /* Process the frame payload as a "logged out" frame */
        wb_nil_ProcessFramePayload (pPort, iPayloadLength, pData, false);
    }
    /* else, check the session ID for a stale session ID */
    else if (iSessionId != pPort->Internals.iSessionId)
    {
        /* Invalid session ID - increment the counters and discard  */
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount);
    }
    /* else, we've got a valid non-idle frame with our session ID */
    else
    {
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRunningValidFrameCount);
        wb_wil_IncrementWithRollover32 (&pPort->Internals.PortStatistics.iRxFrameCount);

        /* Valid frame received, so reset running error counter */
        pPort->Internals.PortStatistics.iRunningRxErrorCount = 0u;
        pPort->Internals.PortStatistics.iRunningLoggedOutFrameCount = 0u;

        /* Process the frame payload as a "logged in" frame */
        wb_nil_ProcessFramePayload (pPort, iPayloadLength, pData, true);
    }
}

static bool wb_nil_ValidateFrameMetadata (uint8_t const * const pData,
                                          uint8_t iPayloadLength)
{
    /* Storage for the computed CRC */
    uint32_t iComputedCRC;

    /* Storage for the extracted CRC */
    uint32_t iExtractedCRC;

    /* Initialize local variables */
    iComputedCRC = 0u;
    iExtractedCRC = 0u;

    if (iPayloadLength <= WBMS_FRAME_PAYLOAD_MAX_SIZE)
    {
        /* Calculate the CRC over the header and payload */
        iComputedCRC = wb_crc_ComputeCRC32 (pData,
                                            (uint32_t) iPayloadLength + WBMS_FRAME_HDR_LEN,
                                            WB_CRC_SEED);

        /* Extract the CRC from the frame */
        iExtractedCRC = ((((pData [WBMS_FRAME_CRC_OFFSET + 0u] & 0xFFFFFFFFu) << 24u) & 0xFF000000u) |
                         (((pData [WBMS_FRAME_CRC_OFFSET + 1u] & 0xFFFFFFFFu) << 16u) & 0x00FF0000u) |
                         (((pData [WBMS_FRAME_CRC_OFFSET + 2u] & 0xFFFFFFFFu) << 8u)  & 0x0000FF00u) |
                         (((pData [WBMS_FRAME_CRC_OFFSET + 3u] & 0xFFFFFFFFu) << 0u)  & 0x0000000FFu)) & 0xFFFFFFFFu;
    }

    /* Return true if the payload was in range and the two values match */
    return ((iPayloadLength <= WBMS_FRAME_PAYLOAD_MAX_SIZE) &&
            (iComputedCRC == iExtractedCRC));
}

static void wb_nil_ProcessFramePayload (adi_wil_port_t * const pPort,
                                        uint8_t iPayloadLength,
                                        uint8_t * const pData,
                                        bool bLoggedInFrame)
{
    uint32_t iProcessedBytes;
    uint8_t iMessageId;
    uint8_t iMessageLength;

    /* Initialize element manually for extra speed */
    wb_pack_element_t Element = { .data = 0u,
                                  .initial_offset = 0u,
                                  .offset = 0u,
                                  .size = iPayloadLength,
                                  .packer = {.direction = WB_PACK_READ,
                                             .index = 0u }};

    /* Initialize element members pointing to the data separately
     * for MISRA compliance */
    Element.origin = &pData [WBMS_FRAME_HDR_LEN];
    Element.packer.buf = &pData [WBMS_FRAME_HDR_LEN];

    /* Init number of processed bytes to 0 */
    iProcessedBytes = 0u;

    /* Validate the payload length */
    if ((iPayloadLength > 0u) &&
        (iPayloadLength <= WBMS_FRAME_PAYLOAD_MAX_SIZE))
    {
        /* Loop through processing */
        while (iProcessedBytes <= iPayloadLength)
        {
            iMessageId = pData [WBMS_FRAME_HDR_LEN + iProcessedBytes];
            iMessageLength = pData [WBMS_FRAME_HDR_LEN + iProcessedBytes + 1u];

            /* Increment by the number of bytes in the packet header */
            iProcessedBytes += WBMS_PACKET_HDR_SIZE;

            /* Validate we are not reading outside the bounds of the payload */
            if ((iProcessedBytes + iMessageLength) > iPayloadLength)
            {
                /* If we are, break out of the processing loop */
                break;
            }
            else
            {
                /* Set up the element size for the length of this packet
                 * payload */
                Element.size = iMessageLength;

                /* Set up the packer index to point to the start of this packet
                 * payload. As our origin is set to the start of the frame
                 * payload this is the number of processed bytes so far */
                Element.packer.index = (uint16_t) iProcessedBytes;

                /* Switch available message types based on whether this was
                 * sent with a logged out or logged in session ID */
                if (bLoggedInFrame)
                {
                    (void) wb_nil_packet_Process (pPort, &Element, iMessageId);
                }
                else
                {
                    (void) wb_nil_packet_ProcessLoggedOutPacket (pPort, &Element, iMessageId);
                }

                /* Increment the processed bytes by the size of this packet */
                iProcessedBytes += iMessageLength;
            }
        }
    }
}

static void wb_nil_ReleaseFrame (adi_wil_port_t * const pPort,
                                 uint8_t const * const pData)
{
    /* Loop through all Rx buffers looking for our frame */
    for (uint8_t i = 0u; i < ADI_WIL_PORT_RX_FRAME_COUNT; i++)
    {
        /* Check if this frame is the one we are looking for */
        if ((const uint8_t *) &pPort->Internals.RxBuffer [i].iData [0] == pData)
        {
            /* Memset everything in this buffer back to 0 */
            (void) memset (&pPort->Internals.RxBuffer [i], 0, sizeof (pPort->Internals.RxBuffer [i]));

            /* We found what we're looking for, break out of the loop */
            break;
        }
    }
}

static void wb_nil_TimerCb (void)
{
    /* Only perform activity if ISRs are enabled*/
    if (!bIsrDisabled)
    {
        /* Transmit on each first unique SPI device available in the list */
        wb_nil_TransmitOnAvailableDevices ();

        /* Iterate through all ports and see if we need to reset the transmission
           status as all shared SPI devices have had a chance to transmit */
        wb_nil_CheckTransmissibleStatus ();
    }
}

static void wb_nil_TransmitOnAvailableDevices (void)
{
    /* Array of SPI Devices we have transmitted on in this period */
    uint8_t iTransmittedList [ADI_WIL_MAX_PORTS];

    /* Number of entries added to the list this period */
    uint32_t iTransmittedCount;

    /* Initialize list count to 0 */
    iTransmittedCount = 0u;

    /* Loop through all ports... */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check this is an in-use and non-null entry */
        if (!bInUseList [i] || ((void *) 0 == DeviceList [i]))
        {
            /* Empty entry - go to next entry */
        }
        else
        {
            /* If its a non null entry, check we haven't already transmitted in
             * this period */
            if (!DeviceList [i]->Internals.bTransmitted &&
                !wb_nil_CheckTransmittedList (&iTransmittedList [0],
                                              iTransmittedCount,
                                              DeviceList [i]->iSPIDevice))
            {
                /* If we haven't, perform transfer on this device */
                wb_nil_Transmit (DeviceList [i]);

                /* Add the SPI device to the list and increment count of
                 * transmitted devices in a CERT-C compatible way */
                if (iTransmittedCount < ADI_WIL_MAX_PORTS)
                {
                    iTransmittedList [iTransmittedCount] = DeviceList [i]->iSPIDevice;
                    iTransmittedCount++;
                }
            }
        }
    }
}

static bool wb_nil_CheckTransmittedList (uint8_t const * const pTransmittedList,
                                         uint32_t iTransmittedCount,
                                         uint8_t iSPIDevice)
{
    /* Return value of this function */
    bool bFound;

    /* Initialize to not found */
    bFound = false;

    /* Loop through transmitted list */
    for (uint32_t i = 0u; i < iTransmittedCount; i++)
    {
        /* If we've already transmitted on this device, flag and break out */
        if (pTransmittedList [i] == iSPIDevice)
        {
            bFound = true;
            break;
        }
    }

    /* Return as transmitted */
    return bFound;
}

static void wb_nil_CheckTransmissibleStatus (void)
{
    /* Loop through looking for ports that have been marked as having transmitted */
    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check this is a non-null entry */
        if (!bInUseList [i] || ((void *) 0 == DeviceList [i]))
        {
            /* Empty entry - go to next entry */
        }
        else
        {
            /* If its a non null entry, check that it's marked as transmitted */
            if (DeviceList [i]->Internals.bTransmitted)
            {
                /* Check if there are other ports using this SPI device still waiting to
                 * transmit. If none are found, mark all ports using this SPI device
                 * as transmissible once again */
                wb_nil_CheckSPIForTransmissibleStatusReset (DeviceList[i]->iSPIDevice);
            }
        }
    }
}

static void wb_nil_CheckSPIForTransmissibleStatusReset (uint8_t iSPIDevice)
{
    /* Array of indexes in the device list that have this SPI Device */
    uint8_t iDeviceIndexList [ADI_WIL_MAX_PORTS];

    /* Number of entries added to the list this period */
    uint32_t iDeviceIndexCount;

    /* Flag indicating if we should reset the transmittable status for this
     * SPI device */
    bool bReset;

    /* Initialize list count to 0 */
    iDeviceIndexCount = 0u;

    /* Assume there are no other devices waiting to transmit */
    bReset = true;

    for (uint8_t i = 0u; i < ADI_WIL_MAX_PORTS; i++)
    {
        /* Check this is a non-null entry */
        if (!bInUseList [i] || ((void *) 0 == DeviceList [i]))
        {
            /* Empty entry - go to next entry */
        }
        /* If its a non null entry, check it shares a SPI device */
        else if (DeviceList [i]->iSPIDevice == iSPIDevice)
        {
            /* Check that it's not market as transmitted */
            if (!DeviceList [i]->Internals.bTransmitted)
            {
                /* Another port is waiting to use this SPI device - don't
                 * reset transmission status */
                bReset = false;
                break;
            }
            else
            {
                /* ...else , add this device to the list of port indices using
                 * this SPI device value and increment devices found in a CERT-C
                 * compatible way */
                if (iDeviceIndexCount < ADI_WIL_MAX_PORTS)
                {
                    iDeviceIndexList [iDeviceIndexCount] = i;
                    iDeviceIndexCount++;
                }
            }
        }
        else
        {
            /* Do nothing - this port does not share a SPI device */
        }
    }

    /* If we didn't find a port with this SPI device still waiting
     * to transmit, reset the transmission status for ports so they
     * can transmit in the next interval */
    if (bReset)
    {
        /* Loop through all found indices and reset the associated port's
         * transmission status */
        for (uint8_t i = 0u; i < iDeviceIndexCount; i++)
        {
            DeviceList [iDeviceIndexList [i]]->Internals.bTransmitted = false;
        }
    }
}

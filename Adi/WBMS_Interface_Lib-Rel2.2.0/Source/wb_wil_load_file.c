/******************************************************************************
 * @file    wb_wil_load_file.c
 *
 * @brief   State machine to send file to device (node/manager)
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_ui.h"
#include "wb_wil_load_file.h"
#include "wbms_cmd_mgr_defs.h"
#include "wb_wil_request.h"
#include "wb_req_otap_hs.h"
#include "wb_req_otap_data.h"
#include "wb_req_generic.h"
#include "wb_rsp_generic.h"
#include "wb_rsp_otap_hs.h"
#include "wb_rsp_otap_status.h"
#include "wb_packer.h"
#include "adi_wil_pack_internals.h"
#include "wb_wil_utils.h"
#include "adi_wil_device_removed.h"
#include "adi_wil_load_file.h"
#include "wb_wil_api.h"
#include "wb_wil_device.h"
#include <string.h>

/**
 * @brief File download Sector size (size to download before checking status)
 * Used for both firmware and general file downloads
 */
#define WB_WIL_DOWNLOAD_SECTOR_SIZE (8192u)

/**
 * @brief Number of blocks in sector for download
 */
#define WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR (WB_WIL_DOWNLOAD_SECTOR_SIZE / ADI_WIL_LOADFILE_DATA_SIZE)

/**
 * @brief Custom retry counter for OTAP related commands
 */
#define WB_WIL_OTAP_RETRIES (15u)

/**
 * @brief Custom timeout value for OTAP related commands
 */
#define WB_WIL_OTAP_TIMEOUT (200u)

/**
 * @brief The same block can be requested for only up to MAX_BLOCK_RETRY times
 * in a row.
 */
#define MAX_BLOCK_RETRY (100u)

/**
 * @brief Blocks for the same sector can be requested for only up to
 * MAX_RETRANSMISSION_ATTEMPTS_PER_SECTOR times in a row.
 */
#define MAX_RETRANSMISSION_ATTEMPTS_PER_SECTOR (64u)

/*****************************************************************************/
/* Static function declarations                                              */
/*****************************************************************************/

static void wb_wil_LoadFileTransmitData(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_LoadFileHandshakeFunc(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_LoadFileStatusFunc(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_LoadFileCommitFunc(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_LoadFileComplete(adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc);
static void wb_wil_CheckMissingBlocks(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_SetupNextSector(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_RequestBlock(adi_wil_pack_internals_t * const pInternals);
static bool wb_wil_RemoveSingleDeviceFromUpdate(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, adi_wil_err_t rc, uint16_t iToken);
static bool wb_wil_RemoveRemainingDevicesFromUpdate(adi_wil_pack_internals_t * const pInternals);
static uint8_t wb_wil_GetNextMissingBlockIndex(adi_wil_pack_internals_t * const pInternals);
static uint64_t wb_wil_GetAndUpdateBMNextNode(adi_wil_pack_internals_t * const pInternals);
static void wb_wil_GoToDataTransmissionPhase(adi_wil_pack_internals_t * const pInternals);
static adi_wil_err_t wb_wil_DoHandshake(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceIdNext);
static void wb_wil_InitHandshakeFunc(adi_wil_pack_internals_t * const pInternals, uint8_t const * const pData);
static bool wb_wil_IsBroadcastNeeded(const adi_wil_pack_internals_t * pInternals, adi_wil_device_t eDeviceId);
static bool wb_wil_IsDeviceInUnicastBitmask(const adi_wil_pack_internals_t * pInternals, uint64_t iDeviceIdToTest);
static bool wb_wil_IsCurrentBlockStuckInTxPhase(adi_wil_pack_internals_t * pInternals, uint16_t iInputBlockNumber);
static adi_wil_err_t wb_wil_PrepareToProceedToDataTxPhase(adi_wil_pack_internals_t * pInternals, uint64_t iDeviceId);
static void wb_wil_IncrementToken(uint16_t *pToken);

/******************************************************************************
 * Function definitions
 *****************************************************************************/

adi_wil_err_t wb_wil_LoadFileAPI (adi_wil_pack_internals_t * const pInternals,
                                  adi_wil_device_t eDeviceId,
                                  adi_wil_file_type_t eFileType,
                                  uint8_t const * const pData)
{
    adi_wil_err_t rc;
    const adi_wil_mode_t ValidModes[] = { ADI_WIL_MODE_STANDBY, ADI_WIL_MODE_OTAP };
    const adi_wil_target_t ValidTargets[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_ALL_NODES };
    void const * const NullableParams[] = { pData };
    bool bReleaseLock = false;
    uint64_t iDeviceIdNext;

    rc = adi_wil_ValidateInstance (pInternals, true);

    /* Acquire lock */
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = wb_wil_ui_AcquireLock (pInternals->pPack, pInternals->pPack);
    }

    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        rc = adi_wil_CheckNullableParams ((sizeof (NullableParams) / sizeof (void *)), NullableParams);

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            rc = adi_wil_CheckSystemMode (pInternals, (sizeof (ValidModes) / sizeof (adi_wil_mode_t)), ValidModes);
        }

        if (ADI_WIL_ERR_SUCCESS == rc)
        {
            if (ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER == pInternals->LoadFileState.eState)
            {
                rc = wb_wil_GetUintFromFileType(eFileType, &pInternals->LoadFileState.iFileType);

                if (ADI_WIL_ERR_SUCCESS == rc)
                {
                    pInternals->LoadFileState.iDeviceActiveBitmask = (adi_wil_device_t)eDeviceId;

                    /* Initialize new bitmask LoadFile variables */
                    pInternals->LoadFileState.bBroadcastHandshaking = false;
                    pInternals->LoadFileState.bBroadcastHandshaking = false;
                    pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy = 0ULL;
                    pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS = 0ULL;
                    pInternals->LoadFileState.iDeviceBitMaskReceivedUnicastHS = 0ULL;

                    if((eDeviceId != ADI_WIL_DEV_ALL_MANAGERS) && (eDeviceId != ADI_WIL_DEV_MANAGER_0) && (eDeviceId != ADI_WIL_DEV_MANAGER_1))
                    {
                        pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy = eDeviceId;

                        /* Check if all the connected nodes are present in the
                         * input 64 bit bitmask */
                        if(wb_wil_IsBroadcastNeeded(pInternals, eDeviceId))
                        {
                            pInternals->LoadFileState.bBroadcastHandshaking = true;
                            rc = wb_wil_SetupRequest(pInternals, ADI_WIL_DEV_ALL_NODES, (sizeof (ValidTargets) / sizeof (adi_wil_target_t)), ValidTargets, wb_wil_LoadFileHandshakeFunc);
                            wb_wil_InitHandshakeFunc(pInternals, pData);
                            wb_wil_LoadFileHandshakeFunc(pInternals);
                            pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_HANDSHAKING;
                        }
                        else
                        {
                            if(pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy != 0x0ULL)
                            {
                                /* Need to unicast handshake to the nodes that
                                 * are enabled in the 64-bit bitmask */
                                wb_wil_InitHandshakeFunc(pInternals, pData);
                                pInternals->LoadFileState.bUnicastHandshaking = true;
                                pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS = pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy;
                                pInternals->LoadFileState.iDeviceBitMaskReceivedUnicastHS = pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy;
                                /* Unpack eDeviceId to find which nodes need
                                 * unicast handshakes */
                                iDeviceIdNext = wb_wil_GetAndUpdateBMNextNode(pInternals);
                                /* Send the first handshake */
                                rc = wb_wil_DoHandshake(pInternals, iDeviceIdNext);
                                pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_HANDSHAKING;
                            }
                            else
                            {
                                /* eDeviceId needs to be a non-zero value */
                                wb_wil_LoadFileComplete(pInternals, ADI_WIL_ERR_INVALID_PARAMETER);
                            }
                        }
                    }
                    else
                    {
                        /* No need for unicast handshaking to nodes (because,
                         * the 64-bit bitmask is valid only when iDeviceId is
                         * ADI_WIL_DEV_ALL_NODES) */
                        rc = wb_wil_SetupRequest(pInternals, eDeviceId, (sizeof(ValidTargets) / sizeof(adi_wil_target_t)), ValidTargets, wb_wil_LoadFileHandshakeFunc);
                        wb_wil_InitHandshakeFunc(pInternals, pData);
                        wb_wil_LoadFileHandshakeFunc(pInternals);
                        pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_HANDSHAKING;
                    }
                }
                else
                {
                    /* No point in proceeding forward if return code is not a
                     * success */
                    wb_wil_LoadFileComplete(pInternals, ADI_WIL_ERR_FAIL);
                }
            }
            else
            {
                /* Internal state has moved on from no transfer state, so
                 * proceed to data transmission */
                pInternals->LoadFileState.pData = pData;
                wb_wil_GoToDataTransmissionPhase(pInternals);
            }
        }

        /* Release lock if any of the above steps returns a failure */
        if (ADI_WIL_ERR_SUCCESS != rc)
        {
            bReleaseLock = true;

            /* Memset LoadFileState to 0 if any of the handshaking steps
             * failed mid-way through */
            (void) memset (&pInternals->LoadFileState, 0, sizeof (pInternals->LoadFileState));
        }
    }

    if (bReleaseLock)
    {
        wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
    }

    return rc;
}

void wb_wil_HandleHandshakeResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_otap_handshake_t const * const pResponse)
{
    adi_wil_err_t rc = ADI_WIL_ERR_SUCCESS;

    /* Ignore any more previous handshake responses once we have already gone
     * to data transmission phase */
    if(ADI_WIL_LOAD_FILE_STATE_HANDSHAKING == pInternals->LoadFileState.eState)
    {
        if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
        {
            /* Do nothing, there is no active request with this token */
        }
        else if (WBMS_CMD_RC_SUCCESS != pResponse->rc)
        {
            /* The end device has responded with a return code of not success.
             * So, end the LoadFile API with the same return code */
            wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iHandshakePhaseFails);
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);
            wb_wil_LoadFileComplete (pInternals, wb_wil_GetErrFromUint (pResponse->rc));
        }
        else if ((pResponse->iFileSize + (ADI_WIL_LOADFILE_DATA_SIZE - 1u)) >= (uint32_t) UINT32_MAX)
        {
            /* The end device has responded with requested file size > max
             * allowed. So, end the LoadFile API with FAIL */
            wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iHandshakePhaseFails);
            wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
        else
        {
            rc = wb_wil_ClearPendingResponse (pInternals, iDeviceId);

            /* Erase the bit*/
            pInternals->LoadFileState.iDeviceBitMaskReceivedUnicastHS &= ~(iDeviceId);

            if((rc == ADI_WIL_ERR_SUCCESS) && ((pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS) == 0ULL))
            {
                /* Proceed to data transmission phase only when unicast
                handshake requests have been sent to all the required nodes
                that have been enabled in the bit-mask */
                pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_TRANSFER;
                rc = wb_wil_PrepareToProceedToDataTxPhase(pInternals, iDeviceId);
                if(ADI_WIL_ERR_SUCCESS == rc)
                {
                    /* Restore original device map */
                    pInternals->LoadFileState.iUpdateMap = pInternals->UserRequestState.iPendingResponses;

                    (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);
                    pInternals->LoadFileState.iTotalBlocks = (uint16_t) (((pResponse->iFileSize + (ADI_WIL_LOADFILE_DATA_SIZE - 1u)) / ADI_WIL_LOADFILE_DATA_SIZE) & (uint16_t) 0xffffu);
                    pInternals->LoadFileState.iFileSize = pResponse->iFileSize;
                    wb_wil_SetupNextSector (pInternals);
                }
                else
                {
                    wb_wil_LoadFileComplete(pInternals, ADI_WIL_ERR_FAIL);
                }
            }
        }
    }
}

void wb_wil_HandleStatusResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_otap_status_t const * const pResponse)
{
    /* Check we are currently performing a transfer and the token is valid */
    if ((ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER == pInternals->LoadFileState.eState) ||
        (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false)))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        pInternals->LoadFileState.bWaitReceived = true;

        /* Once a wait has been received as a response from the end device,
         * resend the status request */
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);
            pInternals->UserRequestState.iRetries = 0u;

            /* Update pending response list to what's currently still in the OTAP process */
            pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

            wb_wil_LoadFileStatusFunc (pInternals);
        }
    }
    else if (WBMS_CMD_RC_SUCCESS == pResponse->rc)
    {
        /* Only use this response if it is talking about the current sector
         * we are transmitting */
        if (pResponse->iIndex == pInternals->LoadFileState.iBaseBlockIdx)
        {
            /* Update the first half of missing blocks map */
            pInternals->LoadFileState.iMissingBlockMask1 |= ((uint64_t) pResponse->MissingBlocks [0u] |
                                                             ((uint64_t) pResponse->MissingBlocks [1u] << 8u) |
                                                             ((uint64_t) pResponse->MissingBlocks [2u] << 16u) |
                                                             ((uint64_t) pResponse->MissingBlocks [3u] << 24u) |
                                                             ((uint64_t) pResponse->MissingBlocks [4u] << 32u) |
                                                             ((uint64_t) pResponse->MissingBlocks [5u] << 40u) |
                                                             ((uint64_t) pResponse->MissingBlocks [6u] << 48u) |
                                                             ((uint64_t) pResponse->MissingBlocks [7u] << 56u));
            
            /* Update the second half of missing blocks map */
            pInternals->LoadFileState.iMissingBlockMask2 |= ((uint64_t) pResponse->MissingBlocks [8u] |
                                                             ((uint64_t) pResponse->MissingBlocks [9u] << 8u) |
                                                             ((uint64_t) pResponse->MissingBlocks [10u] << 16u) |
                                                             ((uint64_t) pResponse->MissingBlocks [11u] << 24u) |
                                                             ((uint64_t) pResponse->MissingBlocks [12u] << 32u) |
                                                             ((uint64_t) pResponse->MissingBlocks [13u] << 40u) |
                                                             ((uint64_t) pResponse->MissingBlocks [14u] << 48u) |
                                                             ((uint64_t) pResponse->MissingBlocks [15u] << 56u));
        }

        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            if (pInternals->LoadFileState.bWaitReceived)
            {
                pInternals->UserRequestState.iRetries = 0u;

                /* Update pending response list to what's currently still
                 * in the OTAP process */
                pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

                /* If response returns with a code of SUCCESS, but still an
                 * internal wait flag is uncleared, then, resend status
                 * request */
                wb_wil_LoadFileStatusFunc (pInternals);
            }
            else
            {
                wb_wil_CheckMissingBlocks (pInternals);
            }
        }
    }
    /* If response returns with a code other than SUCCESS or WAIT, then remove
     * that device from the list of successful devices */
    else if (!wb_wil_RemoveSingleDeviceFromUpdate (pInternals, iDeviceId, wb_wil_GetErrFromUint (pResponse->rc), pResponse->iToken))             /* Remove this device from update, returns true if transfer was aborted */
    {
        wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iStatusPhaseFails);

        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            if (pInternals->LoadFileState.bWaitReceived)
            {
                pInternals->UserRequestState.iRetries = 0u;

                /* Update pending response list to what's currently still in the OTAP process */
                pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

                wb_wil_LoadFileStatusFunc (pInternals);
            }
            else
            {
                wb_wil_CheckMissingBlocks (pInternals);
            }
        }
    }
    else
    {
        /* MISRA else */
    }
}

void wb_wil_HandleCommitResponse (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, wbms_cmd_resp_generic_t const * const pResponse)
{
    /* Check we are currently performing a transfer and the token is valid */
    if ((ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER == pInternals->LoadFileState.eState) ||
        (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false)))
    {
        /* Do nothing, there is no active request with this token */
    }
    else if (WBMS_CMD_RC_WAIT == pResponse->rc)
    {
        pInternals->LoadFileState.bWaitReceived = true;

        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);
            pInternals->UserRequestState.iRetries = 0u;

            /* Update pending response list to what's currently still in the
             * OTAP process */
            pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

            wb_wil_LoadFileCommitFunc (pInternals);
        }
    }
    else if (WBMS_CMD_RC_SUCCESS == pResponse->rc)
    {
        pInternals->UserRequestState.bPartialSuccess = true;
        /* Set flag indicating at least one device has completed the transfer*/
        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            if (pInternals->LoadFileState.bWaitReceived)
            {
                pInternals->UserRequestState.iRetries = 0u;

                /* Update pending response list to what's currently still in the OTAP process */
                pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

                wb_wil_LoadFileCommitFunc (pInternals);
            }
            else
            {
                /* Once the response from end device has return code of SUCCESS
                 * and there are no pending internal wait flags, end LoadFile
                 * API with return code = SUCCESS */
                wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_SUCCESS);
            }
        }
    }
    else if (!wb_wil_RemoveSingleDeviceFromUpdate (pInternals, iDeviceId, wb_wil_GetErrFromUint (pResponse->rc), pResponse->iToken))             /* Remove this device from update, returns true if transfer was aborted */
    {
        /* Log the response code */
        wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iCommitPhaseFails);

        if (WBMS_CMD_RC_CRC_ERROR == pResponse->rc)
        {
            wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iCommitCrcFails);
        }
        else if (WBMS_CMD_RC_FILE_REJECTED == pResponse->rc)
        {
            wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iCommitFileRejected);
        }
        else
        {
            /* MISRA else */
        }

        if (ADI_WIL_ERR_SUCCESS == wb_wil_ClearPendingResponse (pInternals, iDeviceId))
        {
            (void) wb_wil_api_CheckToken (pInternals, pResponse->iToken, true);

            if (pInternals->LoadFileState.bWaitReceived)
            {
                pInternals->UserRequestState.iRetries = 0u;

                /* Update pending response list to what's currently still in the OTAP process */
                pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

                wb_wil_LoadFileCommitFunc (pInternals);
            }
            else
            {
                if (pInternals->UserRequestState.bPartialSuccess)
                {
                    wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_SUCCESS);
                }
                else
                {
                    wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
                }
            }
        }
    }
    else
    {
        /* MISRA Else */
    }
}

void wb_wil_HandleDataResponse (adi_wil_pack_internals_t * const pInternals, wbms_cmd_resp_generic_t const * const pResponse)
{
    uint64_t iDeviceIdNext;

    if(ADI_WIL_LOAD_FILE_STATE_HANDSHAKING == pInternals->LoadFileState.eState)
    {
        if (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, false))
        {
            /* Do nothing, there is no active request with this token */
        }
        else
        {
            iDeviceIdNext = wb_wil_GetAndUpdateBMNextNode(pInternals);

            if((iDeviceIdNext != 0ULL) && ((pInternals->UserRequestState.eTarget == ADI_WIL_TARGET_ALL_NODES) || (pInternals->UserRequestState.eTarget == ADI_WIL_TARGET_SINGLE_NODE)))
            {
                /* Handshake is not yet over for all the required nodes.
                Send a handshake request to the next node */
                pInternals->UserRequestState.iDeviceId = iDeviceIdNext;

                wb_wil_LoadFileHandshakeFunc(pInternals);
            }
        }
    }
    else
    {
        /* Check we are currently performing a transfer & the token is valid */
        if ((ADI_WIL_LOAD_FILE_STATE_NO_TRANSFER == pInternals->LoadFileState.eState) ||
            (ADI_WIL_ERR_SUCCESS != wb_wil_api_CheckToken (pInternals, pResponse->iToken, true)))
        {
            /* Do nothing, there is no active request with this token */
        }
        else
        {
            if ((WBMS_CMD_RC_WAIT != pResponse->rc) && (WBMS_CMD_RC_SUCCESS != pResponse->rc))
            {
                wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iDataPhaseFails);
            }

            if (ADI_WIL_LOAD_FILE_STATE_DOWNLOAD == pInternals->LoadFileState.eState)
            {
                wb_wil_IncrementWithRollover8 (&pInternals->LoadFileState.iBlockIdx);
            }
            else if (ADI_WIL_LOAD_FILE_STATE_RETRANSMIT == pInternals->LoadFileState.eState)
            {
                pInternals->LoadFileState.iBlockIdx = wb_wil_GetNextMissingBlockIndex (pInternals);
            }
            else
            {
                /* MISRA else */
            }

            if (pInternals->LoadFileState.iBlockIdx == pInternals->LoadFileState.iBlocksInSector)
            {
                /* Update pending response list to what's currently still in the OTAP process */
                pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

                pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_RETRANSMIT;
                pInternals->UserRequestState.pfRequestFunc = &wb_wil_LoadFileStatusFunc;
                pInternals->UserRequestState.iRetries = 0u;

                /* Increment the number of times we've requested the status for
                 * this sector */
                if (pInternals->LoadFileState.iRetransmissionCount < MAX_RETRANSMISSION_ATTEMPTS_PER_SECTOR)
                {
                    pInternals->LoadFileState.iRetransmissionCount++;
                }

                wb_wil_IncrementToken(&pInternals->UserRequestState.iToken);
                wb_wil_LoadFileStatusFunc (pInternals);
            }
            else
            {
                wb_wil_RequestBlock (pInternals);
            }
        }
    }
}

static void wb_wil_LoadFileTransmitData (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_otap_data_t Request;
    uint8_t iLength;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    if ((pInternals->LoadFileState.iBlockIdx + pInternals->LoadFileState.iBaseBlockIdx) < (uint16_t) UINT16_MAX)
    {
        Request.iBlockNumber = (pInternals->LoadFileState.iBlockIdx + pInternals->LoadFileState.iBaseBlockIdx);
    }
    else
    {
        Request.iBlockNumber = (uint16_t) UINT16_MAX;
    }

    if (pInternals->LoadFileState.eState == ADI_WIL_LOAD_FILE_STATE_DOWNLOAD)
    {
        wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iDataBlocks);
    }
    else
    {
        wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iRetransmittedBlocks);
    }

    if (pInternals->UserRequestState.iRetries >= WB_WIL_OTAP_RETRIES)
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else
    {
        if (((pInternals->LoadFileState.iBlockIdx + pInternals->LoadFileState.iBaseBlockIdx) != pInternals->LoadFileState.iTotalBlocks))
        {
            iLength = ADI_WIL_LOADFILE_DATA_SIZE;
        }
        else if (0u == (pInternals->LoadFileState.iFileSize % ADI_WIL_LOADFILE_DATA_SIZE))
        {
            iLength = ADI_WIL_LOADFILE_DATA_SIZE;
        }
        else
        {
            iLength = (uint8_t) (pInternals->LoadFileState.iFileSize % ADI_WIL_LOADFILE_DATA_SIZE);
        }

        /* Send a request to start data transmission */
        if (ADI_WIL_ERR_SUCCESS != wb_wil_OTAPDataRequest (pInternals, &Request, pInternals->LoadFileState.pData, iLength, WB_WIL_OTAP_TIMEOUT))
        {
            wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
        else
        {
            /* MISRA Else */
        }
    }

    /* If the same block has been requested repeatedly for more than the
     * allowed limit, complete LoadFile API with failure */
    if(wb_wil_IsCurrentBlockStuckInTxPhase(pInternals, Request.iBlockNumber))
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
}

/******************************************************************************
 * Static Function definitions
 *****************************************************************************/

static void wb_wil_LoadFileHandshakeFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_otap_handshake_t Request;

    /* Initialize request structure */
    (void) memset (&Request, 0, sizeof (Request));

    Request.iFileType = pInternals->LoadFileState.iFileType;

    (void) memcpy (&Request.iHeaderData [0], pInternals->LoadFileState.pData, WB_WIL_OTAP_FILE_HEADER_LEN);

    wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iHandshakeRequests);

    if (pInternals->Stats.OTAPStats.iHandshakeMaxRetries < pInternals->UserRequestState.iRetries)
    {
        pInternals->Stats.OTAPStats.iHandshakeMaxRetries = pInternals->UserRequestState.iRetries;
    }

    if (pInternals->UserRequestState.iRetries >= WB_WIL_OTAP_RETRIES)
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_OTAPHandshakeRequest(pInternals, &Request, WB_WIL_OTAP_TIMEOUT))
    {
        wb_wil_LoadFileComplete(pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_LoadFileStatusFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    pInternals->LoadFileState.bWaitReceived = false;

    wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iStatusRequests);

    if (pInternals->Stats.OTAPStats.iStatusMaxRetries < pInternals->UserRequestState.iRetries)
    {
        pInternals->Stats.OTAPStats.iStatusMaxRetries = pInternals->UserRequestState.iRetries;
    }

    /* Check we have not exceeded the retransmission limit for this sector */
    if (pInternals->LoadFileState.iRetransmissionCount >= MAX_RETRANSMISSION_ATTEMPTS_PER_SECTOR)
    {
        /* ...If we have, abort the transfer */
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else if (pInternals->UserRequestState.iRetries >= WB_WIL_OTAP_RETRIES)
    {
        /* Remove remaining devices, returns true if operation was aborted */
        if (!wb_wil_RemoveRemainingDevicesFromUpdate (pInternals))
        {
            wb_wil_CheckMissingBlocks (pInternals);
        }
    }
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_OTAP_STATUS, WB_WIL_OTAP_TIMEOUT))
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_LoadFileCommitFunc (adi_wil_pack_internals_t * const pInternals)
{
    wbms_cmd_req_generic_t Request;

    pInternals->LoadFileState.bWaitReceived = false;

    /* Increment the number of commit requests in an internal
     * statistics register */
    wb_wil_IncrementWithRollover16 (&pInternals->Stats.OTAPStats.iCommitRequests);

    if (pInternals->Stats.OTAPStats.iCommitMaxRetries < pInternals->UserRequestState.iRetries)
    {
        pInternals->Stats.OTAPStats.iCommitMaxRetries = pInternals->UserRequestState.iRetries;
    }

    if (pInternals->UserRequestState.iRetries >= WB_WIL_OTAP_RETRIES)
    {
        /* Remove remaining devices, returns true if operation was aborted */
        if (!wb_wil_RemoveRemainingDevicesFromUpdate (pInternals))
        {
            if (pInternals->UserRequestState.bPartialSuccess)
            {
                wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_SUCCESS);
            }
            else
            {
                wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
            }
        }
    }
    /* Send a Commit request packet */
    else if (ADI_WIL_ERR_SUCCESS != wb_wil_GenericRequest (pInternals, &Request, WBMS_CMD_OTAP_COMMIT, WB_WIL_OTAP_TIMEOUT))
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
    }
    else
    {
        /* MISRA Else */
    }
}

static void wb_wil_CheckMissingBlocks (adi_wil_pack_internals_t * const pInternals)
{
    /* If there are no missing blocks reported, go to the next sector */
    if ((pInternals->LoadFileState.iMissingBlockMask1 == 0ULL) &&
        (pInternals->LoadFileState.iMissingBlockMask2 == 0ULL))
    {
        if ((pInternals->LoadFileState.iBaseBlockIdx + pInternals->LoadFileState.iBlocksInSector) > (uint16_t) UINT16_MAX)
        {
            /* There are no missing blocks. However, the number of blocks to be
             * transferred is more than the max limit, so abort */
            wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
        else
        {
            /* There are no missing blocks. So, prepare to start transferring
             * the next sector */
            pInternals->LoadFileState.iBaseBlockIdx += pInternals->LoadFileState.iBlocksInSector;
            wb_wil_SetupNextSector (pInternals);
        }
    }
    /* ...else, there are missing blocks so find the block and retransmit */
    else
    {
        /* Get the block index of the missing block and request a
         * retransmission */
        pInternals->LoadFileState.iBlockIdx = wb_wil_GetNextMissingBlockIndex (pInternals);
        wb_wil_RequestBlock (pInternals);
    }
}

static void wb_wil_SetupNextSector (adi_wil_pack_internals_t * const pInternals)
{
    pInternals->UserRequestState.iRetries = 0u;
    pInternals->LoadFileState.iBlockIdx = 0u;
    pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_DOWNLOAD;

    /* New sector - reset retransmission count */
    pInternals->LoadFileState.iRetransmissionCount = 0u;

    /* Reset the missing block masks */
    pInternals->LoadFileState.iMissingBlockMask1 = 0ULL;
    pInternals->LoadFileState.iMissingBlockMask2 = 0ULL;

    if (pInternals->LoadFileState.iBaseBlockIdx == pInternals->LoadFileState.iTotalBlocks)
    {
        /* Update pending response list to what's currently still in the OTAP process */
        pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iUpdateMap;

        pInternals->UserRequestState.pfRequestFunc = &wb_wil_LoadFileCommitFunc;
        pInternals->UserRequestState.iRetries = 0u;
        wb_wil_IncrementToken(&pInternals->UserRequestState.iToken);

        /* Request Commit packet */
        wb_wil_LoadFileCommitFunc (pInternals);
    }
    /* The number of blocks to be transferred is not an exact multiple
     * of WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR */
    else if ((pInternals->LoadFileState.iBaseBlockIdx + WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR) > pInternals->LoadFileState.iTotalBlocks)
    {
        pInternals->LoadFileState.iBlocksInSector = (uint8_t) (((pInternals->LoadFileState.iFileSize + (ADI_WIL_LOADFILE_DATA_SIZE - 1u)) / ADI_WIL_LOADFILE_DATA_SIZE) % WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR);
        wb_wil_RequestBlock (pInternals);
    }
    /* The number of blocks to be transferred is an exact multiple of
     * WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR */
    else
    {
        pInternals->LoadFileState.iBlocksInSector = WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR;
        wb_wil_RequestBlock (pInternals);
    }
}

static void wb_wil_RequestBlock (adi_wil_pack_internals_t * const pInternals)
{
    adi_wil_loadfile_status_t Status;

    Status.iOffset = (uint32_t) WB_WIL_OTAP_FILE_HEADER_LEN +
                     ((uint32_t) ADI_WIL_LOADFILE_DATA_SIZE * pInternals->LoadFileState.iBaseBlockIdx) +
                     ((uint32_t) ADI_WIL_LOADFILE_DATA_SIZE * pInternals->LoadFileState.iBlockIdx);

    Status.iDeviceLoadSuccess = pInternals->LoadFileState.iDeviceActiveBitmask;

    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    /* Generate callback and release lock */
    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_LOAD_FILE, ADI_WIL_ERR_IN_PROGRESS, &Status);
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

static uint8_t wb_wil_GetNextMissingBlockIndex (adi_wil_pack_internals_t * const pInternals)
{
    uint8_t iMissingBlockIndex;

    iMissingBlockIndex = 0u;

    /* Check if there are missing blocks in the first half of the sector */
    if (pInternals->LoadFileState.iMissingBlockMask1 != 0ULL)
    {
        /* Loop through each bit of the mask to find the next missing block */
        while ((pInternals->LoadFileState.iMissingBlockMask1 & (1ULL << iMissingBlockIndex)) == 0ULL)
        {
            iMissingBlockIndex++;
        }

        /* Erase the bit */
        pInternals->LoadFileState.iMissingBlockMask1 &= ~(1ULL << iMissingBlockIndex);
    }
    /* Else check there are missing blocks in the second half of the sector */
    else if (pInternals->LoadFileState.iMissingBlockMask2 != 0ULL)
    {
        /* Loop through each bit of the mask to find the next missing block */
        while ((pInternals->LoadFileState.iMissingBlockMask2 & (1ULL << iMissingBlockIndex)) == 0ULL)
        {
            iMissingBlockIndex++;
        }

        /* Erase the bit */
        pInternals->LoadFileState.iMissingBlockMask2 &= ~(1ULL << iMissingBlockIndex);
        
        /* Add the number of blocks in the first half of the sector */
        iMissingBlockIndex += WB_WIL_DOWNLOAD_BLOCKS_IN_SECTOR >> 1u;
    }
    else
    {
        iMissingBlockIndex = pInternals->LoadFileState.iBlocksInSector;
    }

    return iMissingBlockIndex;
}

static bool wb_wil_RemoveSingleDeviceFromUpdate (adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceId, adi_wil_err_t rc, uint16_t iToken)
{
    adi_wil_device_removed_t DeviceRemovalInfo;
    bool bAbortTransfer;

    /* Initialize DeviceRemovalInfo structure */
    (void) memset (&DeviceRemovalInfo, 0, sizeof (DeviceRemovalInfo));

    bAbortTransfer = true;
    DeviceRemovalInfo.eDeviceId = iDeviceId;
    DeviceRemovalInfo.rc = rc;

    if((pInternals->LoadFileState.bUnicastHandshaking) && (!wb_wil_IsDeviceInUnicastBitmask(pInternals, iDeviceId)))
    {
        /* Do nothing because the response for the iDeviceId does not need to
         * be addressed (since it is not present in the 64-bit mask)*/
    }
    else
    {
        /* Check that the bit already hasn't been cleared and this device is still in the update list */
        if ((pInternals->LoadFileState.iUpdateMap & iDeviceId) != 0ULL)
        {
            /* Erase the bit */
            pInternals->LoadFileState.iUpdateMap &= ~iDeviceId;
            pInternals->LoadFileState.iDeviceActiveBitmask &= ~iDeviceId;

            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_XFER_DEVICE_REMOVED, &DeviceRemovalInfo);
            pInternals->LoadFileState.bDeviceRemoved = true;
        }
    }
    if((pInternals->LoadFileState.bUnicastHandshaking) && (!wb_wil_IsDeviceInUnicastBitmask(pInternals, iDeviceId)))
    {
        bAbortTransfer = true;
        pInternals->UserRequestState.bPartialSuccess = false;
    }
    else
    {
        /* Check if there are still devices in the update mask */
        if (pInternals->LoadFileState.iUpdateMap != 0u)
        {
            bAbortTransfer = false;
        }

        if (bAbortTransfer && !pInternals->UserRequestState.bPartialSuccess)
        {
            /* Remove job from the queue if we're aborting */
            (void) wb_wil_api_CheckToken (pInternals, iToken, true);
            wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_FAIL);
        }
    }
    return bAbortTransfer && !pInternals->UserRequestState.bPartialSuccess;
}

static bool wb_wil_RemoveRemainingDevicesFromUpdate (adi_wil_pack_internals_t * const pInternals)
{
    adi_wil_device_removed_t DeviceRemovalInfo;
    bool bAbortTransfer;

    /* Initialize DeviceRemovalInfo structure */
    (void) memset (&DeviceRemovalInfo, 0, sizeof (DeviceRemovalInfo));

    bAbortTransfer = false;

    DeviceRemovalInfo.rc = ADI_WIL_ERR_TIMEOUT;

    /* Scan through all the devices and identify which devices need to be
     * removed from the list of devices with successful file transfer. */
    for (uint8_t i = 0u; i < (ADI_WIL_DEVICE_MASK_LEN * 8u); i++)
    {
        DeviceRemovalInfo.eDeviceId = (1ULL << i);

        if ((pInternals->UserRequestState.iPendingResponses & DeviceRemovalInfo.eDeviceId) != 0u)
        {
            /* Erase the bit */
            pInternals->UserRequestState.iPendingResponses &= ~DeviceRemovalInfo.eDeviceId;
            pInternals->LoadFileState.iUpdateMap &= ~DeviceRemovalInfo.eDeviceId;
            pInternals->LoadFileState.iDeviceActiveBitmask &= ~DeviceRemovalInfo.eDeviceId;

            wb_wil_ui_GenerateEvent (pInternals->pPack, ADI_WIL_EVENT_XFER_DEVICE_REMOVED, &DeviceRemovalInfo);
            pInternals->LoadFileState.bDeviceRemoved = true;

            if ((ADI_WIL_TARGET_SINGLE_NODE == pInternals->UserRequestState.eTarget) || (ADI_WIL_TARGET_SINGLE_MANAGER == pInternals->UserRequestState.eTarget))
            {
                bAbortTransfer = true;
                break;
            }
        }
    }

    if (!bAbortTransfer && !pInternals->UserRequestState.bPartialSuccess)
    {
        /* Check if there are still devices in the update mask */
        if (pInternals->LoadFileState.iUpdateMap != 0u)
        {
            bAbortTransfer = false;
        }
        else
        {
            bAbortTransfer = true;
        }
    }

    /* When it is time to abort a transfer and there is not even one device
     * that has been successful with a transfer (partial success)
     * LoadFile API is completed with return code = TIMEOUT */
    if (bAbortTransfer && !pInternals->UserRequestState.bPartialSuccess)
    {
        wb_wil_LoadFileComplete (pInternals, ADI_WIL_ERR_TIMEOUT);
    }

    return bAbortTransfer && !pInternals->UserRequestState.bPartialSuccess;
}

static void wb_wil_LoadFileComplete (adi_wil_pack_internals_t * const pInternals, adi_wil_err_t rc)
{
    adi_wil_err_t UserRC;
    struct adi_wil_loadfile_status_t ReturnStruct;

    pInternals->UserRequestState.bValid = false;
    pInternals->UserRequestState.pfRequestFunc = (void *) 0;

    /* Memset return structure prior to populating it (if required) */
    (void) memset (&ReturnStruct, 0, sizeof (ReturnStruct));

    /* If we get RC success here, check if any devices were removed.
     * If so, return partial success instead */
    if ((ADI_WIL_ERR_SUCCESS == rc) && (pInternals->LoadFileState.bDeviceRemoved))
    {
        UserRC = ADI_WIL_ERR_PARTIAL_SUCCESS;
    }
    else
    {
        UserRC = rc;
    }

    /* Populate the return structure that has the bit mask of the devices
     * that succeeded in file transfer */
    if((UserRC == ADI_WIL_ERR_SUCCESS) || (UserRC == ADI_WIL_ERR_PARTIAL_SUCCESS))
    {
        ReturnStruct.iDeviceLoadSuccess = pInternals->LoadFileState.iDeviceActiveBitmask;
    }

    /* Memset LoadFileState to 0 prior to generating callback but after
     * populating return structure */
    (void) memset (&pInternals->LoadFileState, 0, sizeof (pInternals->LoadFileState));

    wb_wil_ui_GenerateCb (pInternals->pPack, ADI_WIL_API_LOAD_FILE, UserRC, &ReturnStruct);

    /* Release lock */
    wb_wil_ui_ReleaseLock (pInternals->pPack, pInternals->pPack);
}

static uint64_t wb_wil_GetAndUpdateBMNextNode(adi_wil_pack_internals_t * const pInternals)
{
    /* Loop variable for finding next set bit */
    uint8_t iDeviceIndex;

    /* Return value of this function */
    uint64_t iDeviceID;

    /* Initialize index to 0 */
    iDeviceIndex = 0u;

    /* Initialize return value to 0 */
    iDeviceID = 0u;

    /* Check there are nodes in the list still to update... */
    if (pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS != 0ULL)
    {
        /* Loop through and find the next set bit */
        while (((pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS >> iDeviceIndex) & 1ULL) != 1ULL)
        {
            iDeviceIndex++;
        }

        /* Generate the ID from the index */
        iDeviceID = (1ULL << iDeviceIndex);

        /* Erase the bit */
        pInternals->LoadFileState.iDeviceBitMaskSentUnicastHS &= ~(iDeviceID);
    }

    /* Return an adi_wil_device_t set to the next node's ID */
    return iDeviceID;
}

static void wb_wil_GoToDataTransmissionPhase(adi_wil_pack_internals_t * const pInternals)
{
    /* Initialize all the internal variables to prepare to do data
     * transmission */
    pInternals->UserRequestState.pfRequestFunc = &wb_wil_LoadFileTransmitData;
    pInternals->UserRequestState.iRetries = 0u;
    if(pInternals->LoadFileState.eState == ADI_WIL_LOAD_FILE_STATE_HANDSHAKING)
    {
        pInternals->LoadFileState.eState = ADI_WIL_LOAD_FILE_STATE_TRANSFER;
    }
    /* Send data transmission request after generating the corresponding
     * token for this new request */
    wb_wil_IncrementToken(&pInternals->UserRequestState.iToken);
    wb_wil_LoadFileTransmitData(pInternals);
}

static adi_wil_err_t wb_wil_DoHandshake(adi_wil_pack_internals_t * const pInternals, uint64_t iDeviceIdNext)
{
    adi_wil_err_t rc;
    /* This handshake request is for the first node */
    const adi_wil_target_t ValidTargetsSingleNode[] = { ADI_WIL_TARGET_SINGLE_NODE };
    rc = wb_wil_SetupRequest(pInternals, iDeviceIdNext, (sizeof(adi_wil_target_t) / sizeof(adi_wil_target_t)), ValidTargetsSingleNode, wb_wil_LoadFileHandshakeFunc);
    if (ADI_WIL_ERR_SUCCESS == rc)
    {
        /* Store current list of pending devices */
        pInternals->LoadFileState.iUpdateMap = pInternals->UserRequestState.iPendingResponses;

        /* Send handshake request */
        wb_wil_LoadFileHandshakeFunc(pInternals);
    }
    return rc;
}

static void wb_wil_InitHandshakeFunc(adi_wil_pack_internals_t * const pInternals, uint8_t const * const pData)
{
    /* Store current list of pending devices */
    pInternals->LoadFileState.iUpdateMap = pInternals->UserRequestState.iPendingResponses;

    pInternals->LoadFileState.pData = pData;
    pInternals->LoadFileState.iBaseBlockIdx = 0u;
    pInternals->LoadFileState.iBlockIdx = 0u;
    pInternals->LoadFileState.iTotalBlocks = 0u;
    pInternals->LoadFileState.bDeviceRemoved = false;

    /* Clear the internal struct that holds the statistics for
     * LoadFile operation */
    (void)memset(&pInternals->Stats.OTAPStats, 0, sizeof(pInternals->Stats.OTAPStats));
}

static bool wb_wil_IsBroadcastNeeded(const adi_wil_pack_internals_t * pInternals, adi_wil_device_t eDeviceId)
{
    /* Return true if either of the following conditions are met:
     * - The user is explicitly requesting the ALL_NODES device ID
     * - The bitmap passed in contains all existing nodes AND all nodes are connected */
    return (eDeviceId == ADI_WIL_DEV_ALL_NODES) ||
            ((pInternals->NodeState.iCount <= ADI_WIL_MAX_NODES) &&
             (pInternals->NodeState.iConnectState == (0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount))) &&
             (eDeviceId == pInternals->NodeState.iConnectState));
}

static bool wb_wil_IsDeviceInUnicastBitmask(const adi_wil_pack_internals_t * pInternals, uint64_t iDeviceIdToTest)
{
    /* Check if the input device ID (iDeviceIdToTest) is present in the 64 bit
     * bitmask of nodes to be handshaked individually */
    return ((iDeviceIdToTest & pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy) != 0ULL);
}

static bool wb_wil_IsCurrentBlockStuckInTxPhase(adi_wil_pack_internals_t * pInternals, uint16_t iInputBlockNumber)
{
    bool bReturnValue = false;
    /* Check if the same block has been requested for multiple times
     * in a row */
    if(pInternals->LoadFileState.iPreviouslyRequestedBlockNumber == iInputBlockNumber)
    {
        pInternals->LoadFileState.iNumberOfBlkNumberRetry++;
    }
    else
    {
        pInternals->LoadFileState.iPreviouslyRequestedBlockNumber = iInputBlockNumber;
        pInternals->LoadFileState.iNumberOfBlkNumberRetry = 0;
    }

    /* If the same block has been requested for max times in a row,
     * reset count and return true */
    if(pInternals->LoadFileState.iNumberOfBlkNumberRetry > MAX_BLOCK_RETRY)
    {
        pInternals->LoadFileState.iNumberOfBlkNumberRetry = 0;
        bReturnValue = true;
    }
    return bReturnValue;
}

static adi_wil_err_t wb_wil_PrepareToProceedToDataTxPhase(adi_wil_pack_internals_t * pInternals, uint64_t iDeviceId)
{
    adi_wil_err_t rc;
    const adi_wil_target_t ValidTargetsBeforeDataTx[] = { ADI_WIL_TARGET_ALL_NODES };
    const adi_wil_target_t ValidTargetsAll[] = { ADI_WIL_TARGET_SINGLE_MANAGER, ADI_WIL_TARGET_SINGLE_NODE, ADI_WIL_TARGET_ALL_MANAGERS, ADI_WIL_TARGET_ALL_NODES };
    if(pInternals->LoadFileState.bUnicastHandshaking)
    {
        /* Unicast handshaking is done only for nodes. Therefore,
         * set up a request for all nodes */
        rc = wb_wil_SetupRequest(pInternals, ADI_WIL_DEV_ALL_NODES, ((sizeof(ValidTargetsBeforeDataTx) / sizeof(adi_wil_target_t))), ValidTargetsBeforeDataTx, wb_wil_LoadFileHandshakeFunc);

        /* Update the expected response mask based on what's in the LoadFileState internal copy */
        pInternals->UserRequestState.iPendingResponses = pInternals->LoadFileState.iDeviceBitMaskInternalNodeCopy;
    }
    else
    {
        /* This is a broadcast handshaking case for all nodes.
         * Therefore, set up a request for all nodes */
        if (pInternals->LoadFileState.bBroadcastHandshaking)
        {
            rc = wb_wil_SetupRequest(pInternals, ADI_WIL_DEV_ALL_NODES, (sizeof(ValidTargetsAll) / sizeof(adi_wil_target_t)), ValidTargetsAll, wb_wil_LoadFileHandshakeFunc);
        }
        else
        {
            /* Set-up the correct bit-mask to prepare for a new packet
             * request generation */
            if(pInternals->UserRequestState.eTarget == ADI_WIL_TARGET_ALL_MANAGERS)
            {
                /* All managers case: Send request to all the managers */
                rc = wb_wil_SetupRequest(pInternals, ADI_WIL_DEV_ALL_MANAGERS, (sizeof(ValidTargetsAll) / sizeof(adi_wil_target_t)), ValidTargetsAll, wb_wil_LoadFileHandshakeFunc);
            }
            else
            {
                /* Individual manager case: Send request to the
                 * required manager */
                rc = wb_wil_SetupRequest(pInternals, iDeviceId, (sizeof(ValidTargetsAll) / sizeof(adi_wil_target_t)), ValidTargetsAll, wb_wil_LoadFileHandshakeFunc);
            }
        }
    }
    return rc;
}

static void wb_wil_IncrementToken (uint16_t * pToken)
{
    /* Increment the token if we're not at the max */
    if ((uint16_t) UINT16_MAX != *pToken)
    {
        (*pToken)++;
    }
    /* If we were at the max, reset token to 1 */
    else
    {
        (*pToken) = 1u;
    }
}

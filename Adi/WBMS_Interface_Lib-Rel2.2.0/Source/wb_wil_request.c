/******************************************************************************
 * @file    wb_wil_request.c
 *
 * @brief   Pack manager and node requests and submit to manager API port
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_wil_request.h"
#include "wb_nil.h"
#include "wb_req_set_mode.h"
#include "wb_req_generic.h"
#include "wb_req_set_gpio.h"
#include "wb_req_get_gpio.h"
#include "wb_req_select_script.h"
#include "wb_req_get_contextual.h"
#include "wb_req_set_contextual.h"
#include "wb_req_otap_hs.h"
#include "wb_req_send_data.h"
#include "wb_req_set_contextual.h"
#include "wb_req_set_customer_identifier.h"
#include "wb_req_set_state_of_health.h"
#include "wb_req_set_acl.h"
#include "wb_req_modify_script.h"
#include "wb_req_get_file.h"
#include "wb_req_otap_data.h"
#include "adi_wil_pack_internals.h"
#include "wbms_port_config.h"
#include "adi_wil_file.h"
#include "wb_req_set_mon_params_data.h"
#include "wb_req_inventory_transition.h"
#include "wb_req_get_file_crc.h"
#include "wb_req_erase_file.h"
#include "wb_req_rotate_key.h"
#include "wb_req_get_acl.h"
#include "wb_req_fault_service.h"
#include "wb_req_dmh_apply.h"
#include "wb_wil_utils.h"
#include "wb_wil_api.h"
#include "wb_wil_device.h"
#include "wb_packer.h"
#include "wb_pack_cmd.h"
#include "wb_pack_cmd_mgr.h"
#include "wb_pack_cmd_node.h"
#include "adi_wil_port.h"
#include <string.h>
/******************************************************************************
 * #define
 *****************************************************************************/

/** @brief Bit position of Manager 0 in 64-bit map */
#define ADI_WIL_REQUEST_MANAGER_0_BITP (62u)

/** @brief Bit position of Manager 1 in 64-bit map */
#define ADI_WIL_REQUEST_MANAGER_1_BITP (63u)

/** @brief Pre-calculated LUT for 64-bit De Bruijn Log2 */
#define ADI_WIL_REQUEST_DEBRUJIN_MAP { 63u,  0u, 58u,  1u, 59u, 47u, 53u,  2u, \
                                       60u, 39u, 48u, 27u, 54u, 33u, 42u,  3u, \
                                       61u, 51u, 37u, 40u, 49u, 18u, 28u, 20u, \
                                       55u, 30u, 34u, 11u, 43u, 14u, 22u,  4u, \
                                       62u, 57u, 46u, 52u, 38u, 26u, 32u, 41u, \
                                       50u, 36u, 17u, 19u, 29u, 10u, 13u, 21u, \
                                       56u, 45u, 25u, 31u, 35u, 16u,  9u, 12u, \
                                       44u, 24u, 15u,  8u, 23u,  7u,  6u,  5u  }

/** @brief Pre-calculated constant for 64-bit De Bruijn Log2 */
#define ADI_WIL_REQUEST_DEBRUJIN_SEQ (0x7EDD5E59A4E28C2ULL)

/******************************************************************************
 * Typedefs
 *****************************************************************************/

/** @brief Used for storing a message length and message Id */
typedef struct {
    uint8_t iLength;
    uint8_t iMessageId;
    bool bUserRequestContext;
    bool bUseTokenForSendData;
} wb_request_message_details_t;

/******************************************************************************
 * Static function declarations
 *****************************************************************************/

static bool wb_wil_SetTargetForRequest (adi_wil_pack_internals_t * const pInternals,
                                        adi_wil_target_t const ValidTargets [],
                                        uint8_t iTargetCount);

static bool wb_wil_ValidateTargetExists (adi_wil_pack_internals_t const * const pInternals,
                                         adi_wil_device_t iDeviceId,
                                         adi_wil_target_t eTarget);

static bool wb_wil_ValidateTargetAllowed (adi_wil_target_t eTarget,
                                          adi_wil_target_t const ValidTargets [],
                                          uint8_t iTargetCount);

static void wb_wil_WriteSendDataRequest (adi_wil_pack_internals_t const * const pInternals,
                                         wb_pack_element_t * pElement,
                                         wb_request_message_details_t const * const pMessageDetails,
                                         wb_request_message_details_t * const pSendDataDetails);

static bool wb_nil_GetBufferFromPort (adi_wil_port_t * const pPort,
                                      wb_pack_element_t * const pElement,
                                      bool bUserContext);

static bool wb_nil_GetBufferFromPack (adi_wil_pack_internals_t * const pInternals,
                                      wb_pack_element_t * const pElement,
                                      adi_wil_port_t ** ppPort,
                                      bool bUserContext);

static void wb_wil_AppendPayload (wb_request_message_details_t * const pMessageDetails,
                                  wb_pack_element_t * const pElement,
                                  uint8_t * const pDestination,
                                  uint8_t const * const pSource,
                                  uint8_t iLength);

static adi_wil_err_t wb_wil_SubmitBuffer (adi_wil_pack_internals_t const * const pInternals,
                                          adi_wil_port_t * const pPort,
                                          wb_pack_element_t * const pElement,
                                          wb_request_message_details_t const * const pMessageDetails);

static void adi_wil_PrepareElementForSubmission (wb_pack_element_t * const pElement,
                                                 wb_request_message_details_t const * const pMessageDetails);

static adi_wil_port_t * adi_wil_SelectPort (adi_wil_pack_internals_t * const pInternals);

static adi_wil_port_t * adi_wil_SelectPortForNodeTarget (adi_wil_pack_internals_t * const pInternals);

static uint8_t wb_wil_GetNetworkDeviceId (uint64_t iDeviceId);

static bool wb_wil_CheckDeviceSingleTarget (uint64_t iDeviceId);

static bool wb_wil_CheckNodeTarget (uint64_t iDeviceId);

static bool wb_wil_CheckExclusiveTarget (uint64_t iDeviceId);

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SetupRequest (adi_wil_pack_internals_t * const pInternals,
                                   uint64_t iDeviceId,
                                   uint8_t iTargetCount,
                                   adi_wil_target_t const ValidTargets [],
                                   adi_wil_api_process_func_t pfRequestFunc)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input parameters before dereferencing */
    if (((void *) 0 == pInternals) ||
        ((void *) 0 == pfRequestFunc))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Store user request Device Id */
        pInternals->UserRequestState.iDeviceId = iDeviceId;

        /* Check the targeted device exists, and is allowed. Sets eTarget and response mask */
        if (!wb_wil_SetTargetForRequest (pInternals,
                                         ValidTargets,
                                         iTargetCount))
        {
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
        }
        /* If the above was successful, store the parameters used by other modules */
        else
        {
            /* Increment the token */
            wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);

            /* If we rolled over, increment it again so the token is now non-zero */
            if (pInternals->UserRequestState.iToken == 0u)
            {
                wb_wil_IncrementWithRollover16 (&pInternals->UserRequestState.iToken);
            }

            /* Reset parameters used for retrying a request */
            pInternals->UserRequestState.iRetries = 0u;
            pInternals->UserRequestState.bPartialSuccess = false;
            pInternals->UserRequestState.pfRequestFunc = pfRequestFunc;

            /* Mark operation as success */
            rc = ADI_WIL_ERR_SUCCESS;
        }
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_ClearPendingResponse (adi_wil_pack_internals_t * const pInternals,
                                           uint64_t iDeviceId)
{
    /* Return value of this function */
    adi_wil_err_t rc;

    /* Ensure internals are not NULL before dereferencing and the incoming
     * device ID is a single bit */
    if (((void *) 0 == pInternals) || !wb_wil_CheckDeviceSingleTarget (iDeviceId))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    /* Else, it's valid so clear the bit */
    else
    {
        /* Invert and logically AND to clear */
        pInternals->UserRequestState.iPendingResponses &= ~(iDeviceId);

        /* If we've cleared all responses set the response code to success */
        if (pInternals->UserRequestState.iPendingResponses == 0u)
        {
            rc = ADI_WIL_ERR_SUCCESS;
        }
        /* ... else, we're still in progress. Set RC to in-progress */
        else
        {
            rc = ADI_WIL_ERR_IN_PROGRESS;
        }
    }

    /* Return response code */
    return rc;
}

/******************************************************************************
 * Public functions - Port Based Requests
 *****************************************************************************/

adi_wil_err_t wb_wil_ConnectRequest (adi_wil_port_t * const pPort,
                                     wbms_cmd_req_connect_t * const pRequest)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_CONNECT_LEN,
                                                          .iMessageId = WBMS_CMD_CONNECT,
                                                          .bUserRequestContext = false,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPort (pPort,
                                                               &Element,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else 
    {
        /* Write the request to the buffer */
        wb_pack_ConnectReq (&Element,
                            pRequest);

        /* Write the Packet metadata to the element and prepare for submission */
        adi_wil_PrepareElementForSubmission (&Element,
                                             &MessageDetails);

        /* Submit the buffer for transmission */
        rc = wb_nil_SubmitFrame (pPort, &Element);
    }
    
    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_QueryDeviceRequest (adi_wil_port_t * pPort,
                                         wbms_cmd_req_generic_t * const pRequest)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GENERIC_LEN,
                                                          .iMessageId = WBMS_CMD_QUERY_DEVICE,
                                                          .bUserRequestContext = false,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPort (pPort,
                                                               &Element,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Write the request to the buffer */
        wb_pack_GenericReq (&Element,
                            pRequest);

        /* Write the Packet metadata to the element and prepare for submission */
        adi_wil_PrepareElementForSubmission (&Element,
                                             &MessageDetails);

        /* Submit the buffer for transmission */
        rc = wb_nil_SubmitFrame (pPort, &Element);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GenericAcknowledgement (adi_wil_port_t * const pPort,
                                             wbms_notif_ack_t * const pRequest,
                                             uint8_t iMessageId)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_NOTIF_ACK_LEN,
                                                          .iMessageId = iMessageId,
                                                          .bUserRequestContext = false,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPort (pPort,
                                                               &Element,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Write the request to the buffer */
        wb_pack_NotifAck (&Element,
                          pRequest);

        /* Write the Packet metadata to the element and prepare for submission */
        adi_wil_PrepareElementForSubmission (&Element,
                                             &MessageDetails);

        /* Submit the buffer for transmission */
        rc = wb_nil_SubmitFrame (pPort, &Element);
    }

    /* Return response code */
    return rc;
}

/******************************************************************************
 * Public functions - Pack based requests
 *****************************************************************************/

adi_wil_err_t wb_wil_GenericRequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_generic_t * const pRequest,
                                     uint8_t iMessageId,
                                     uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GENERIC_LEN,
                                                          .iMessageId = iMessageId,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GenericReq (&Element,
                            pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetModeRequest (adi_wil_pack_internals_t * const pInternals, 
                                     wbms_cmd_req_set_mode_t * const pRequest, 
                                     uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_MODE_LEN,
                                                          .iMessageId = WBMS_CMD_SET_MODE,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetModeReq (&Element,
                            pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_OTAPHandshakeRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_otap_handshake_t * const pRequest,
                                           uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_OTAP_HANDSHAKE_LEN,
                                                          .iMessageId = WBMS_CMD_OTAP_HANDSHAKE,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = true };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_OTAPHandshakeReq (&Element,
                                  pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_OTAPDataRequest (adi_wil_pack_internals_t * const pInternals,
                                      wbms_cmd_req_otap_data_t * const pRequest,
                                      uint8_t const * const pData,
                                      uint8_t iLength,
                                      uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_OTAP_DATA_LEN,
                                                    .iMessageId = WBMS_CMD_OTAP_DATA,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = true };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;
    
    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_OTAPDataReq (&Element,
                             pRequest,
                             &pOutgoingData);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetGPIORequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_set_gpio_t * const pRequest,
                                     uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_GPIO_LEN,
                                                          .iMessageId = WBMS_CMD_SET_GPIO,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetGpioReq (&Element,
                            pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GetGPIORequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_get_gpio_t * const pRequest,
                                     uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GET_GPIO_LEN,
                                                          .iMessageId = WBMS_CMD_GET_GPIO,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GetGpioReq (&Element,
                            pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SelectScriptRequest (adi_wil_pack_internals_t * const pInternals,
                                          wbms_cmd_req_select_script_t * const pRequest,
                                          uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SELECT_SCRIPT_LEN,
                                                          .iMessageId = WBMS_CMD_SELECT_SCRIPT,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SelectScriptReq (&Element,
                                 pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_ModifyScriptRequest (adi_wil_pack_internals_t * const pInternals,
                                          wbms_cmd_req_modify_script_t * const pRequest,
                                          uint8_t const * const pData,
                                          uint8_t iLength,
                                          uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_MODIFY_SCRIPT_LEN,
                                                    .iMessageId = WBMS_CMD_MODIFY_SCRIPT,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Variable for storing CRC value of header params */
    uint16_t iCRC;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_ModifyScriptReq (&Element,
                                 pRequest,
                                 &iCRC,
                                 &pOutgoingData);

        /* Append 16-bit CRC to modify script request */
        wb_packer_uint16 (&Element.packer, &iCRC);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetContextualRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_set_contextual_t * const pRequest,
                                           uint8_t const * const pData,
                                           uint8_t iLength,
                                           uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_CONTEXTUAL_LEN,
                                                    .iMessageId = WBMS_CMD_SET_CONTEXTUAL_DATA,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetContextualReq (&Element,
                                  pRequest,
                                  &pOutgoingData);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GetContextualRequest (adi_wil_pack_internals_t * const pInternals,
                                           wbms_cmd_req_get_contextual_t * const pRequest,
                                           uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GET_CONTEXTUAL_LEN,
                                                          .iMessageId = WBMS_CMD_GET_CONTEXTUAL_DATA,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GetContextualReq (&Element,
                                  pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}


adi_wil_err_t wb_wil_SetCustomerIdentifierRequest (adi_wil_pack_internals_t * const pInternals,
                                                   wbms_cmd_req_set_customer_identifier_t * const pRequest,
                                                   uint8_t const * const pData,
                                                   uint8_t iLength,
                                                   uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_CUSTOMER_IDENTIFIER_LEN,
                                                    .iMessageId = WBMS_CMD_SET_CUSTOMER_IDENTIFIER,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetCustomerIdentifierReq (&Element,
                                          pRequest,
                                          &pOutgoingData);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetStateOfHealthRequest (adi_wil_pack_internals_t * const pInternals,
                                              wbms_cmd_req_set_state_of_health_t * const pRequest,
                                              uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_SOH_LEN,
                                                          .iMessageId = WBMS_CMD_SET_STATE_OF_HEALTH,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetStateOfHealthReq (&Element,
                                      pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GetFileRequest (adi_wil_pack_internals_t * const pInternals,
                                     wbms_cmd_req_get_file_t * const pRequest,
                                     uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GET_FILE_LEN,
                                                          .iMessageId = WBMS_CMD_GET_FILE,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GetFileReq (&Element,
                            pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_EraseFileRequest (adi_wil_pack_internals_t * const pInternals,
                                       wbms_cmd_req_erase_file_t * const pRequest,
                                       uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_ERASE_FILE_LEN,
                                                          .iMessageId = WBMS_CMD_ERASE_FILE,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_EraseFileReq (&Element,
                              pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_InventoryTransitionRequest (adi_wil_pack_internals_t * const pInternals,
                                                 wbms_cmd_req_inventory_transition_t * const pRequest,
                                                 uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_INVENTORY_TRANS_LEN,
                                                          .iMessageId = WBMS_CMD_INVENTORY_STATE_TRANSITION,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_InventoryTransitionReq (&Element,
                                        pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GetFileCRCRequest (adi_wil_pack_internals_t * const pInternals,
                                        wbms_cmd_req_get_file_crc_t * const pRequest,
                                        uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GET_FILE_CRC_LEN,
                                                          .iMessageId = WBMS_CMD_GET_FILE_CRC,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GetFileCRCReq (&Element,
                               pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_RotateKeyRequest (adi_wil_pack_internals_t * const pInternals,
                                       wbms_cmd_req_rotate_key_t * const pRequest,
                                       uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_ROTATE_KEY_LEN,
                                                          .iMessageId = WBMS_CMD_ROTATE_KEY,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_RotateKeyReq (&Element,
                              pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetACLRequest (adi_wil_pack_internals_t * const pInternals,
                                    wbms_cmd_req_set_acl_t * const pRequest,
                                    uint8_t const * const pData,
                                    uint8_t iLength,
                                    uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_ACL_LEN,
                                                    .iMessageId = WBMS_CMD_SET_ACL,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetAclReq (&Element,
                           pRequest,
                           &pOutgoingData);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_GetACLRequest (adi_wil_pack_internals_t * const pInternals,
                                    wbms_cmd_req_get_acl_t * const pRequest,
                                    uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_GET_ACL_LEN,
                                                          .iMessageId = WBMS_CMD_GET_ACL,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_GetAclReq (&Element,
                           pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_SetFaultModeRequest (adi_wil_pack_internals_t * const pInternals,
                                          wbms_cmd_req_fault_service_t * const pRequest,
                                          uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_FAULT_MODE_LEN,
                                                          .iMessageId = WBMS_CMD_SET_FAULT_SERVICE_MODE,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetFaultModeReq (&Element,
                                 pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_UpdateMonitorParametersDataRequest (adi_wil_pack_internals_t * const pInternals,
                                                         wbms_cmd_req_set_mon_params_data_t * const pRequest,
                                                         uint8_t const * const pData,
                                                         uint8_t iLength,
                                                         uint32_t iTimeout)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SET_MON_PARAMS_DATA_LEN,
                                                    .iMessageId = WBMS_CMD_SET_MON_PARAMS_DATA,
                                                    .bUserRequestContext = true,
                                                    .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Variable for receiving position of "data" in buffer to write to */
    uint8_t * pOutgoingData;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_SetMonParamsDataReq (&Element,
                                     pRequest,
                                     &pOutgoingData);

        /* Append the payload data to this message */
        wb_wil_AppendPayload (&MessageDetails,
                              &Element,
                              pOutgoingData,
                              pData,
                              iLength);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}

adi_wil_err_t wb_wil_ApplyNetworkTopologyRequest (adi_wil_pack_internals_t * const pInternals,
                                                  wbms_cmd_req_dmh_apply_t * const pRequest,
                                                  uint32_t iTimeout)
{
    /* Storage for the message details */
    const wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_DMH_APPLY_LEN,
                                                          .iMessageId = WBMS_CMD_DMH_APPLY,
                                                          .bUserRequestContext = true,
                                                          .bUseTokenForSendData = false };

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pRequest) || !wb_nil_GetBufferFromPack (pInternals,
                                                               &Element,
                                                               &pPort,
                                                               MessageDetails.bUserRequestContext))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Start the timer and obtain the current token */
        (void) wb_wil_api_StartTimer (pInternals,
                                      &pRequest->iToken,
                                      iTimeout);

        /* Write the request to the buffer */
        wb_pack_DMHApplyReq (&Element,
                             pRequest);

        /* Submit the buffer for transmission */
        rc = wb_wil_SubmitBuffer (pInternals,
                                  pPort,
                                  &Element,
                                  &MessageDetails);
    }

    /* Return response code */
    return rc;
}


adi_wil_err_t wb_wil_ASSLSendDataRequest (adi_wil_pack_internals_t * const pInternals,
                                          uint8_t const * const pMessage,
                                          uint8_t iDeviceId)
{
    /* Storage for the message details - to be modified by wb_wil_AppendData */
    wb_request_message_details_t MessageDetails = { .iLength = WBMS_CMD_REQ_SEND_DATA_LEN,
                                                    .iMessageId = WBMS_CMD_SEND_DATA,
                                                    .bUserRequestContext = false,
                                                    .bUseTokenForSendData = false };

    /* Initialize SendData Header */
    wbms_cmd_req_send_data_t Request = { .iToken = 0u,
                                         .iDeviceId = iDeviceId,
                                         .iLength = ADI_WIL_MAX_APP_PAYLOAD_SIZE,
                                         .iHighPriority = 1u,
                                         .iPortId = WB_SCL_PORT_ID };

    /* Storage for Payload pointer */
    uint8_t * pData;

    /* Variable for storing message packing details */
    wb_pack_element_t Element;

    /* Variable for storing port to send message on */
    adi_wil_port_t * pPort;

    /* Return value of this function */
    adi_wil_err_t rc;

    /* Validate input params and set up the element for packing a message */
    if (((void *) 0 == pMessage) || ((void *) 0 == pInternals))
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        /* Obtain the port to transmit wirelessly on */
        pPort = adi_wil_SelectPortForNodeTarget (pInternals);

        /* Attempt to get a buffer from this port */
        if ((void *) 0 == pPort)
        {
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
        }
        else
        {
            /* Clear Element's internals prior to use */
            (void) memset (&Element, 0, sizeof (Element));

            /* Initialize Element internals */
            Element.origin = &pPort->Internals.ProcessTaskRequestFrame [0];
            Element.packer.index = WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE;

            /* Initialize Packer internals */
            Element.packer.buf = Element.origin;
            Element.packer.index = WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE;
            Element.packer.direction = WB_PACK_WRITE;

            /* Clear contents of buffer before use */
            (void) memset (Element.origin, 0, ADI_WIL_SPI_TRANSACTION_SIZE);

            /* Write the SendData header at the start of the frame payload */
            wb_pack_SendDataReq (&Element, &Request, &pData);

            /* Append the payload data to this message */
            wb_wil_AppendPayload (&MessageDetails,
                                  &Element,
                                  pData,
                                  pMessage,
                                  ADI_WIL_MAX_APP_PAYLOAD_SIZE);

            /* Write the Packet metadata to the element and prepare for submission */
            adi_wil_PrepareElementForSubmission (&Element,
                                                 &MessageDetails);

            /* Submit the buffer for transmission */
            rc = wb_nil_SubmitFrame (pPort, &Element);
        }
    }

    /* Return response code */
    return rc;
}

/******************************************************************************
 * Static function definitions
 *****************************************************************************/

static bool wb_wil_SetTargetForRequest (adi_wil_pack_internals_t * const pInternals,
                                        adi_wil_target_t const ValidTargets [],
                                        uint8_t iTargetCount)
{
    /* Return value of this function */
    bool bValid;

    /* Initialize return value as invalid */
    bValid = false;

    /* Clear any existing pending responses */
    pInternals->UserRequestState.iPendingResponses = 0u;

    /* Check if device ID only contains either nodes or managers */
    if (!wb_wil_CheckExclusiveTarget (pInternals->UserRequestState.iDeviceId))
    {
        /* Do nothing - return failure */
    }
    else if (wb_wil_CheckNodeTarget (pInternals->UserRequestState.iDeviceId))
    {
        /* Check if it's for a single node... */
        if (wb_wil_CheckDeviceSingleTarget (pInternals->UserRequestState.iDeviceId))
        {
            pInternals->UserRequestState.eTarget = ADI_WIL_TARGET_SINGLE_NODE;

            /* Set the singular bit in the mask */
            pInternals->UserRequestState.iPendingResponses = pInternals->UserRequestState.iDeviceId;
        }
        /* Else, it's for all nodes */
        else
        {
            pInternals->UserRequestState.eTarget = ADI_WIL_TARGET_ALL_NODES;

            /* Set the pending responses to map to all nodes currently in the ACL */
            if (pInternals->NodeState.iCount != 0u)
            {
                pInternals->UserRequestState.iPendingResponses = 0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount);
            }
            /* If we have no nodes in the ACL, set pending responses to nothing */
            else
            {
                pInternals->UserRequestState.iPendingResponses = 0ULL;
            }
        }
    }
    /* Else, its for the manager(s)... */
    else
    {
        /* Check if it's for a single manager... */
        if (wb_wil_CheckDeviceSingleTarget (pInternals->UserRequestState.iDeviceId))
        {
            pInternals->UserRequestState.eTarget = ADI_WIL_TARGET_SINGLE_MANAGER;

            /* Set the singular bit in the mask */
            pInternals->UserRequestState.iPendingResponses = pInternals->UserRequestState.iDeviceId;
        }
        /* Else, it's for all managers */
        else
        {
            pInternals->UserRequestState.eTarget = ADI_WIL_TARGET_ALL_MANAGERS;

            /* Always set the singular bit in the mask for manager 0 */
            pInternals->UserRequestState.iPendingResponses |= (1ULL << ADI_WIL_REQUEST_MANAGER_0_BITP);

            /* If Manager 1 exists, set the bit for it too */
            if ((void *) 0 != pInternals->pManager1Port)
            {
                pInternals->UserRequestState.iPendingResponses |= (1ULL << ADI_WIL_REQUEST_MANAGER_1_BITP);
            }
        }
    }

    /* Check the target device exists... */
    if (wb_wil_ValidateTargetExists (pInternals,
                                     pInternals->UserRequestState.iDeviceId,
                                     pInternals->UserRequestState.eTarget))
    {
        /* If the above steps were successful, check that this request
         * is allowed to be called with this device id */
        bValid = wb_wil_ValidateTargetAllowed (pInternals->UserRequestState.eTarget,
                                               ValidTargets,
                                               iTargetCount);
    }

    /* Return response code */
    return bValid;
}

static bool wb_wil_ValidateTargetExists (adi_wil_pack_internals_t const * const pInternals,
                                         adi_wil_device_t iDeviceId,
                                         adi_wil_target_t eTarget)
{
    /* Storage for finding the bit position of the node when it's a single node */
    uint64_t iAllowedNodes;

    /* Return value of this function */
    bool bValid;

    /* If single node target, check device is in ACL */
    if (eTarget == ADI_WIL_TARGET_SINGLE_NODE)
    {
        /* Ensure the node count is in range... */
        if ((pInternals->NodeState.iCount != 0u) &&
            (pInternals->NodeState.iCount <= ADI_WIL_MAX_NODES))
        {
            /* Build a bitmap of all nodes in the ACL */
            iAllowedNodes = 0xFFFFFFFFFFFFFFFFULL >> (64u - pInternals->NodeState.iCount);

            /* Check the device ID exists in the ACL */
            bValid = (iAllowedNodes & iDeviceId) != 0u;
        }
        /* ...else, this is an invalid node target */
        else
        {
            bValid = false;
        }
    }
    /* If all nodes target, make sure at least one node exists...  */
    else if (eTarget == ADI_WIL_TARGET_ALL_NODES)
    {
        bValid = (pInternals->NodeState.iCount != 0u);
    }
    /* If single manager: Ensure device ID is either manager 0 or manager 1 exists... */
    else if (eTarget == ADI_WIL_TARGET_SINGLE_MANAGER)
    {
        bValid = ((iDeviceId == ADI_WIL_DEV_MANAGER_0) ||
                  ((iDeviceId == ADI_WIL_DEV_MANAGER_1) &&
                   (pInternals->pManager1Port != (void *) 0)));
    }
    /* Else, it's all managers which is always valid */
    else
    {
        bValid = true;
    }

    /* Return response code */
    return bValid;
}

static bool wb_wil_ValidateTargetAllowed (adi_wil_target_t eTarget,
                                          adi_wil_target_t const ValidTargets [],
                                          uint8_t iTargetCount)
{
    /* Return value of this function */
    bool bValid;

    /* Initialize to assume item does not exist in list */
    bValid = false;

    /* Loop through the list looking for the target */
    for (uint8_t i = 0u; i < iTargetCount; i++)
    {
        /* If found, set valid to true and break out of loop */
        if (ValidTargets [i] == eTarget)
        {
            bValid = true;
            break;
        }
    }

    return bValid;
}

static bool wb_nil_GetBufferFromPort (adi_wil_port_t * const pPort,
                                      wb_pack_element_t * const pElement,
                                      bool bUserContext)
{
    /* Return value of this function */
    bool bSuccess;

    /* Initialize local variable to false */
    bSuccess = false;

    /* Validate input parameters (pElement can never be NULL) */
    if ((void *) 0 != pPort)
    {
        /* Clear pElement's internals prior to use */
        (void) memset (pElement, 0, sizeof (wb_pack_element_t));

        /* Initialize Element internals */
        pElement->origin = bUserContext ? &pPort->Internals.UserRequestFrame [0] : &pPort->Internals.ProcessTaskRequestFrame [0];

        /* Initialize Packer internals */
        pElement->packer.buf = pElement->origin;
        pElement->packer.index = WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE;
        pElement->packer.direction = WB_PACK_WRITE;

        /* Clear contents of buffer before use */
        (void) memset (pElement->origin, 0, ADI_WIL_SPI_TRANSACTION_SIZE);

        /* Set rc to success */
        bSuccess = true;
    }

    return bSuccess;
}

static bool wb_nil_GetBufferFromPack (adi_wil_pack_internals_t * const pInternals,
                                      wb_pack_element_t * const pElement,
                                      adi_wil_port_t ** ppPort,
                                      bool bUserContext)
{
    /* Return value of this function */
    bool bSuccess;

    /* Initialize local variable to false */
    bSuccess = false;

    /* Validate input parameters (pElement can never be NULL) */
    if ((void *) 0 != pInternals)
    {
        /* Select a Port to transmit the message on */
        *ppPort = adi_wil_SelectPort (pInternals);

        /* Attempt to acquire a buffer from the selected port */
        bSuccess = wb_nil_GetBufferFromPort (*ppPort,
                                             pElement,
                                             bUserContext);

        /* If we were successful, and the intended target is for a node, set
         * the current offset in the element to hold space for a SendData
         * header + command ID */
        if (bSuccess && wb_wil_CheckNodeTarget (pInternals->UserRequestState.iDeviceId))
        {
            pElement->packer.index = WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE + WBMS_CMD_REQ_SEND_DATA_LEN + WBMS_PACKET_CMD_ID_LEN;
        }
    }

    return bSuccess;
}

static adi_wil_port_t * adi_wil_SelectPort (adi_wil_pack_internals_t * const pInternals)
{
    /* Return value of this function */
    adi_wil_port_t * pPort;

    /* If the request is to be sent to the nodes select either connected port */
    if (wb_wil_CheckNodeTarget (pInternals->UserRequestState.iDeviceId))
    {
        pPort = adi_wil_SelectPortForNodeTarget (pInternals);
    }
    /* ... else, we've got a manager target which must always go to the
     * designated port selected in SetupRequest (). Check for manager 1 */
    else if (pInternals->UserRequestState.iDeviceId == ADI_WIL_DEV_MANAGER_1)
    {
        pPort = pInternals->pManager1Port;
    }
    /* Else, must be for all managers or manager 0. Select Manager 0 and the
     * message will be duplicated to manager 1 if intended for both on
     * submission.*/
    else
    {
        pPort = pInternals->pManager0Port;
    }

    return pPort;
}

static adi_wil_port_t * adi_wil_SelectPortForNodeTarget (adi_wil_pack_internals_t * const pInternals)
{
    /* Return value of this function */
    adi_wil_port_t * pPort;

    /* Initialize return value to NULL */
    pPort = (void *) 0;

    /* Validate the port is non-null before dereferencing */
    if (pInternals->pCurrentPort != (void *) 0)
    {
        /* If the expected current port is connected, select it... */
        if (pInternals->pCurrentPort->Internals.bConnected)
        {
            pPort = pInternals->pCurrentPort;

            /* If Manager 1 exists, select it for transmission for the next
             * message to the nodes */
            if ((pInternals->pCurrentPort == pInternals->pManager0Port) &&
                (pInternals->pManager1Port != (void *) 0))
            {
                pInternals->pCurrentPort = pInternals->pManager1Port;
            }
            /* Else, manager 1 does not exist or manager 1 was just selected,
             * so select manager 0 for the next transmission */
            else
            {
                pInternals->pCurrentPort = pInternals->pManager0Port;
            }
        }
        /* Else, at least one port is not connected. Check if manager 0
         * is connected */
        else
        {
            /* Validate that the port is not null before dereferencing */
            if (pInternals->pManager0Port != (void *) 0)
            {
                if (pInternals->pManager0Port->Internals.bConnected)
                {
                    /* Select Manager 0 for transmission */
                    pPort = pInternals->pManager0Port;
                }
            }
            /* If pPort is still null, it's Manager 0 that's not connected so check if
             * Manager 1 exists... */
            if ((pPort == (void *) 0) &&
                (pInternals->pManager1Port != (void *) 0))
            {
                /* Check if Manager 1 is connected... */
                if (pInternals->pManager1Port->Internals.bConnected)
                {
                    /* Select Manager 0 for transmission */
                    pPort = pInternals->pManager1Port;
                }
            }
        }
    }

    return pPort;
}

static void wb_wil_AppendPayload (wb_request_message_details_t * const pMessageDetails,
                                  wb_pack_element_t * const pElement,
                                  uint8_t * const pDestination,
                                  uint8_t const * const pSource,
                                  uint8_t iLength)
{
    /* Ensure payload can be appended within message size limits */
    if (((pMessageDetails->iLength + iLength) <= WBMS_MESSAGE_PAYLOAD_MAX_SIZE) &&
        ((pElement->data + iLength) <= WBMS_MESSAGE_PAYLOAD_MAX_SIZE))
    {
        /* Add the length of the payload to the total length of the message */
        pMessageDetails->iLength += iLength;
        pElement->data += iLength;

        /* Copy the outgoing data to the buffer */
        (void) memcpy (pDestination, pSource, iLength);
    }
}

static adi_wil_err_t wb_wil_SubmitBuffer (adi_wil_pack_internals_t const * const pInternals,
                                          adi_wil_port_t * const pPort,
                                          wb_pack_element_t * const pElement,
                                          wb_request_message_details_t const * const pMessageDetails)
{
    /* Storage for the message details - will be modified by WriteSendDataRequest */
    wb_request_message_details_t SendDataDetails = { .iLength = WBMS_CMD_REQ_SEND_DATA_LEN,
                                                     .iMessageId = WBMS_CMD_SEND_DATA,
                                                     .bUserRequestContext = pMessageDetails->bUserRequestContext,
                                                     .bUseTokenForSendData = pMessageDetails->bUseTokenForSendData };

    /* Return value of this function */
    adi_wil_err_t rc;

    /* If it's a node target prepare the element, */
    if (wb_wil_CheckNodeTarget (pInternals->UserRequestState.iDeviceId))
    {
        wb_wil_WriteSendDataRequest (pInternals,
                                     pElement,
                                     pMessageDetails,
                                     &SendDataDetails);

        adi_wil_PrepareElementForSubmission (pElement,
                                             &SendDataDetails);

        /* Submit the buffer for transmission */
        rc = wb_nil_SubmitFrame (pPort, pElement);
    }
    else
    {
        /* Write the Packet metadata to the element and prepare for submission */
        adi_wil_PrepareElementForSubmission (pElement,
                                             pMessageDetails);

        /* Submit the buffer for transmission */
        rc = wb_nil_SubmitFrame (pPort, pElement);

        /* If we've... 
         * - Got a message from the user context
         * - Successfully submitted on manager 0
         * - Got a request that's targeting all managers
         * - Got a pack instance with two ports
         * Then, copy the frame from manager 0 to manager 1 and submit there.
         */
        if (pMessageDetails->bUserRequestContext &&
            (ADI_WIL_ERR_SUCCESS == rc) &&
            (ADI_WIL_TARGET_ALL_MANAGERS == pInternals->UserRequestState.eTarget) &&
            ((void *) 0 != pInternals->pManager0Port) &&
            ((void *) 0 != pInternals->pManager1Port))
        {
            (void) memcpy (pInternals->pManager1Port->Internals.UserRequestFrame,
                           pInternals->pManager0Port->Internals.UserRequestFrame,
                           ADI_WIL_SPI_TRANSACTION_SIZE);

            /* Setup the element to now point to manager 1's buffer */
            pElement->origin = pInternals->pManager1Port->Internals.UserRequestFrame;

            /* Submit the buffer for transmission */
            rc = wb_nil_SubmitFrame (pInternals->pManager1Port, pElement);
        }
    }

    return rc;
}

static void adi_wil_PrepareElementForSubmission (wb_pack_element_t * const pElement,
                                                 wb_request_message_details_t const * const pMessageDetails)
{
    /* Ensure element will not overflow frame payload size */
    if ((pElement->data <= WBMS_FRAME_PAYLOAD_MAX_SIZE) &&
        (pMessageDetails->iLength <= WBMS_MESSAGE_PAYLOAD_MAX_SIZE))
    {
        /* Set the amount of "data" contained in the element to the payload length */
        pElement->data = pMessageDetails->iLength;

        /* Set the total size of the element to the payload length + metadata */
        pElement->size = (((uint16_t) pMessageDetails->iLength + (WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE)) & 0xFFFFu);

        /* Write the message ID */
        pElement->origin [WBMS_FRAME_HDR_LEN + 0u] = pMessageDetails->iMessageId;

        /* Write the message length */
        pElement->origin [WBMS_FRAME_HDR_LEN + 1u] = pMessageDetails->iLength;

        /* Increase the size of the total payload length by the length of the
         * packet header */
        pElement->data += WBMS_PACKET_HDR_SIZE;
    }
}

static void wb_wil_WriteSendDataRequest (adi_wil_pack_internals_t const * const pInternals,
                                         wb_pack_element_t * pElement,
                                         wb_request_message_details_t const * const pMessageDetails,
                                         wb_request_message_details_t * const pSendDataDetails)
{
    /* Local storage for Message Id to pass to non-const function */
    uint8_t iMessageId = pMessageDetails->iMessageId;

    /* Initialize SendData Header */
    wbms_cmd_req_send_data_t Request = { .iToken = 0u,
                                         .iDeviceId = wb_wil_GetNetworkDeviceId (pInternals->UserRequestState.iDeviceId),
                                         .iHighPriority = 1u,
                                         .iPortId = WB_OPCMD_PORT_ID };

    /* Storage for Payload pointer - unused by implementation */
    uint8_t * pData;

    /* Ensure element will not overflow 80 byte network max payload size and
     * Ensure incrementing the message length will not overflow a uint8 */
    if (((WBMS_PACKET_CMD_ID_LEN + pMessageDetails->iLength) <= ADI_WIL_MAX_APP_PAYLOAD_SIZE) &&
        (((WBMS_PACKET_CMD_ID_LEN + pMessageDetails->iLength) + pSendDataDetails->iLength) <= 0xFFu))
    {
        /* If the SendData request is using the token, set it in the structure */
        if (pMessageDetails->bUseTokenForSendData)
        {
            Request.iToken = pInternals->UserRequestState.iToken;
        }

        /* Set SendData length field to include the length of the message payload */
        Request.iLength = (WBMS_PACKET_CMD_ID_LEN + pMessageDetails->iLength);

        /* Set SendData message details length field to include the length of the message */
        pSendDataDetails->iLength += (WBMS_PACKET_CMD_ID_LEN + pMessageDetails->iLength);

        /* Set the packer to the start of the payload area */
        pElement->packer.index = WBMS_FRAME_HDR_LEN + WBMS_PACKET_HDR_SIZE;

        /* Write the SendData header at the start of the frame payload */
        wb_pack_SendDataReq (pElement, &Request, &pData);

        /* Write the message ID at the start of the SendData payload */
        wb_packer_uint8 (&pElement->packer, &iMessageId);
    }
}

static bool wb_wil_CheckDeviceSingleTarget (uint64_t iDeviceId)
{
    /* Check if a single bit (target) is set */
    return (iDeviceId != 0u) && ((iDeviceId & (iDeviceId - 1u)) == 0u);
}

static bool wb_wil_CheckNodeTarget (uint64_t iDeviceId)
{
    /* Check if a single bit (target) is set */
    return ((iDeviceId & ADI_WIL_DEV_ALL_NODES) != 0u);
}

static bool wb_wil_CheckExclusiveTarget (uint64_t iDeviceId)
{
    /* Check if only managers or only nodes are set */
    return ((iDeviceId & ADI_WIL_DEV_ALL_NODES) != 0u) != 
           ((iDeviceId & ADI_WIL_DEV_ALL_MANAGERS) != 0u);
}

static uint8_t wb_wil_GetNetworkDeviceId (uint64_t iDeviceId)
{
    /* Look up table for de Bruijn sequence */
    static const uint8_t iDeBruijnTable [] = ADI_WIL_REQUEST_DEBRUJIN_MAP;

    /* Return value of this function */
    uint8_t iNetworkDeviceId;

    /* Handle special case of translating multi set bit Device ID = 255 */
    if (ADI_WIL_DEV_ALL_NODES == iDeviceId)
    {
        iNetworkDeviceId = WB_WIL_DEV_ALL_NODES;
    }
    /* ... else, it's a single node so find the bit position of the set bit */
    else
    {
        /* Multiply by de Bruijn sequence constant and shift to get the
         * integer position of the single set bit */
        iNetworkDeviceId = iDeBruijnTable [(iDeviceId * ADI_WIL_REQUEST_DEBRUJIN_SEQ) >> 58u];
    }

    /* Return translated Device ID to caller */
    return iNetworkDeviceId;
}

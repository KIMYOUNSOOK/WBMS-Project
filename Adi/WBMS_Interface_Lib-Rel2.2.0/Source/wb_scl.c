/******************************************************************************
 * @file    wb_scl.c
 *
 * @brief   WIL FE Safety Communications Layer
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_scl.h"
#include "wb_scl_fusa.h"
#include "wb_scl_msg_defs.h"
#include "adi_wil_pack.h"
#include "wb_assl_fusa.h"
#include "wb_wil_msg_header.h"
#include <string.h>

/******************************************************************************
 *   Defines
 *****************************************************************************/

/* Initial seed value 0x20u after performing 32-bit bit reversal */
#define WB_SCL_CRC_INITIAL_SEED_VALUE            (0x04000000u)

#define WB_SCL_HDR_SOURCE_ID_OFFSET              (0u)
#define WB_SCL_HDR_DEST_ID_OFFSET                (1u)
#define WB_SCL_HDR_SEQ_NUM_OFFSET                (2u)
#define WB_SCL_HDR_MSG_TYPE_OFFSET               (3u)
#define WB_SCL_HDR_PAYLOAD_LEN_OFFSET            (4u)
#define WB_SCL_PAYLOAD_OFFSET                    (WB_SCL_MSG_HDR_LEN)
#define WB_WIL_DEV_HOST_CONTROLLER               (230u)
#define WB_WIL_DEV_ALL_NODES                     (255u)

/******************************************************************************
 *  Static function declarations
 *****************************************************************************/

static bool wb_scl_ValidateCrc (uint8_t const * const pBuffer,
                                uint8_t iLength);

static void wb_scl_ComputeCrc (uint8_t const * const pBuffer,
                               uint8_t iLength,
                               uint32_t * pCrc);

static void wb_scl_PackCrc (uint8_t * const pBuffer,
                            uint32_t const * const pCrc);

static void wb_scl_UnpackCrc (uint8_t const * const pBuffer,
                              uint32_t * const pCrc);

static adi_wil_safety_internals_t * wb_scl_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack);

static void wb_scl_ReadSCLHeader (uint8_t const * const pSCLFrame,
                                  wb_msg_header_t * pMsgHeader,
                                  uint8_t * pDestDeviceId);

static void wb_scl_WriteSCLHeader (uint8_t * const pSCLFrame,
                                   wb_msg_header_t const * const pMsgHeader);

static bool wb_scl_ValidateIncomingMessageHeader (wb_msg_header_t const * const pMsgHeader,
                                                  uint8_t iDestDeviceId,
                                                  uint8_t iSourceDeviceId);

static bool wb_scl_ValidateOutgoingMessageHeader (wb_msg_header_t const * const pMsgHeader);

static bool wb_scl_ValidateHandleSCLFrameParameters (adi_wil_safety_internals_t const * const pInternals,
                                                     uint8_t iDeviceId,
                                                     uint8_t iLength,
                                                     uint8_t const * const pData);

static uint32_t wb_scl_BitReverseCrc (uint32_t iCrc);

/******************************************************************************
 *   Public function definitions
 *****************************************************************************/

void wb_scl_HandleSCLFrame (adi_wil_pack_t const * const pPack,
                            uint8_t iDeviceId,
                            uint8_t iLength,
                            uint8_t const * const pData)
{
    /* Static variable for retrieving safety internals pointer */
    adi_wil_safety_internals_t * pInternals;

    /* Destination device ID read from the SCL frame */
    uint8_t iDestDeviceId;

    /* SCL message header */
    wb_msg_header_t MsgHeader;

    /* Retrieve safety internals pointer */
    pInternals = wb_scl_GetSafetyInternalsPointer (pPack);

    /* Validate input parameters */
    if (!wb_scl_ValidateHandleSCLFrameParameters (pInternals, iDeviceId, iLength, pData))
    {
        /* SCL frame error - report to ASSL */
        wb_assl_ReportValidationError (pPack);
    }
    else
    {
        /* Copy out the SCL frame to ASSL memory before parsing */
        (void) memcpy (&pInternals->ASSL.iSCLFrame [0], pData, iLength);

        /* Extract SCL frame header fields from the SCL frame */
        wb_scl_ReadSCLHeader (&pInternals->ASSL.iSCLFrame [0], &MsgHeader, &iDestDeviceId);

        /* Validate SCL frame header fields and CRC */
        if (!wb_scl_ValidateIncomingMessageHeader (&MsgHeader, iDestDeviceId, iDeviceId) ||
            !wb_scl_ValidateCrc (&pInternals->ASSL.iSCLFrame [0], (uint8_t) (WB_SCL_MSG_HDR_LEN + MsgHeader.iPayloadLength)))
        {
            /* SCL frame error - report to ASSL */
            wb_assl_ReportValidationError (pPack);
        }
        else if (MsgHeader.iPayloadLength > 0u)
        {
            /* Validated frame with non-zero payload length. Pass on to ASSL
             * for processing */
            wb_assl_HandleValidatedSCLFrame (pInternals, &MsgHeader, &pInternals->ASSL.iSCLFrame [WB_SCL_PAYLOAD_OFFSET]);
        }
        else
        {
            /* Validated frame with zero payload length. Pass on to ASSL for
             * processing */
            wb_assl_HandleValidatedSCLFrame (pInternals, &MsgHeader, (void*) 0);
        }
    }
}

bool wb_scl_WrapSCLFrame (wb_msg_header_t const * const pMsgHeader,
                          uint8_t * const pBuffer)
{
    /* Boolean result indicating if the SCL frame was written to
     * successfully */
    bool bComplete;

    /* CRC of message and header to write into the SCL frame */
    uint32_t crc;

    /* Validate input parameters */
    /* Validate input message header parameters */
    if ((pBuffer == (void *) 0) ||
        (pMsgHeader == (void *) 0) ||
        !wb_scl_ValidateOutgoingMessageHeader (pMsgHeader))
    {
        bComplete = false;
    }
    else
    {
        /* Write SCL frame header */
        wb_scl_WriteSCLHeader (pBuffer, pMsgHeader);

        /* Compute CRC over message data and header of the SCL frame */
        wb_scl_ComputeCrc (pBuffer, (uint8_t) (WB_SCL_MSG_HDR_LEN + pMsgHeader->iPayloadLength), &crc);

        /* Skip over the data block and write the CRC */
        wb_scl_PackCrc (&pBuffer [WB_SCL_MSG_HDR_LEN + pMsgHeader->iPayloadLength], &crc);

        bComplete = true;
    }

    return bComplete;
}

/******************************************************************************
 *  Static function definitions
 *****************************************************************************/

static bool wb_scl_ValidateCrc (uint8_t const * const pBuffer,
                                uint8_t iLength)
{
    /* Computed CRC value */
    uint32_t crc1;

    /* CRC value unpacked from the SCL Frame */
    uint32_t crc2;

    /* Initialize local variables */
    crc1 = 0u;
    crc2 = 0u;

    /* Compute the CRC for the given input data */
    wb_scl_ComputeCrc (pBuffer, iLength, &crc1);

    /* Unpack the CRC from the SCL frame */
    wb_scl_UnpackCrc (&pBuffer [iLength], &crc2);

    /* Check if CRC value in the SCL frame matches the computed value */
    return (crc1 == crc2);
}

static void wb_scl_ComputeCrc (uint8_t const * const pBuffer,
                               uint8_t iLength,
                               uint32_t * pCrc)
{
    /* CRC look-up table for 21-bit CRC */
    static const uint32_t iSclCrcLUT[] =
    {
        0X00000000u, 0X7DFF4F11u, 0XFBFE9E22u, 0X8601D133u, 0XA7D5AF5Bu, 0XDA2AE04Au, 0X5C2B3179u, 0X21D47E68u,
        0X1F83CDA9u, 0X627C82B8u, 0XE47D538Bu, 0X99821C9Au, 0XB85662F2u, 0XC5A92DE3u, 0X43A8FCD0u, 0X3E57B3C1u,
        0X3F079B52u, 0X42F8D443u, 0XC4F90570u, 0XB9064A61u, 0X98D23409u, 0XE52D7B18u, 0X632CAA2Bu, 0X1ED3E53Au,
        0X208456FBu, 0X5D7B19EAu, 0XDB7AC8D9u, 0XA68587C8u, 0X8751F9A0u, 0XFAAEB6B1u, 0X7CAF6782u, 0X01502893u,
        0X7E0F36A4u, 0X03F079B5u, 0X85F1A886u, 0XF80EE797u, 0XD9DA99FFu, 0XA425D6EEu, 0X222407DDu, 0X5FDB48CCu,
        0X618CFB0Du, 0X1C73B41Cu, 0X9A72652Fu, 0XE78D2A3Eu, 0XC6595456u, 0XBBA61B47u, 0X3DA7CA74u, 0X40588565u,
        0X4108ADF6u, 0X3CF7E2E7u, 0XBAF633D4u, 0XC7097CC5u, 0XE6DD02ADu, 0X9B224DBCu, 0X1D239C8Fu, 0X60DCD39Eu,
        0X5E8B605Fu, 0X23742F4Eu, 0XA575FE7Du, 0XD88AB16Cu, 0XF95ECF04u, 0X84A18015u, 0X02A05126u, 0X7F5F1E37u,
        0XFC1E6D48u, 0X81E12259u, 0X07E0F36Au, 0X7A1FBC7Bu, 0X5BCBC213u, 0X26348D02u, 0XA0355C31u, 0XDDCA1320u,
        0XE39DA0E1u, 0X9E62EFF0u, 0X18633EC3u, 0X659C71D2u, 0X44480FBAu, 0X39B740ABu, 0XBFB69198u, 0XC249DE89u,
        0XC319F61Au, 0XBEE6B90Bu, 0X38E76838u, 0X45182729u, 0X64CC5941u, 0X19331650u, 0X9F32C763u, 0XE2CD8872u,
        0XDC9A3BB3u, 0XA16574A2u, 0X2764A591u, 0X5A9BEA80u, 0X7B4F94E8u, 0X06B0DBF9u, 0X80B10ACAu, 0XFD4E45DBu,
        0X82115BECu, 0XFFEE14FDu, 0X79EFC5CEu, 0X04108ADFu, 0X25C4F4B7u, 0X583BBBA6u, 0XDE3A6A95u, 0XA3C52584u,
        0X9D929645u, 0XE06DD954u, 0X666C0867u, 0X1B934776u, 0X3A47391Eu, 0X47B8760Fu, 0XC1B9A73Cu, 0XBC46E82Du,
        0XBD16C0BEu, 0XC0E98FAFu, 0X46E85E9Cu, 0X3B17118Du, 0X1AC36FE5u, 0X673C20F4u, 0XE13DF1C7u, 0X9CC2BED6u,
        0XA2950D17u, 0XDF6A4206u, 0X596B9335u, 0X2494DC24u, 0X0540A24Cu, 0X78BFED5Du, 0XFEBE3C6Eu, 0X8341737Fu,
        0XA814498Fu, 0XD5EB069Eu, 0X53EAD7ADu, 0X2E1598BCu, 0X0FC1E6D4u, 0X723EA9C5u, 0XF43F78F6u, 0X89C037E7u,
        0XB7978426u, 0XCA68CB37u, 0X4C691A04u, 0X31965515u, 0X10422B7Du, 0X6DBD646Cu, 0XEBBCB55Fu, 0X9643FA4Eu,
        0X9713D2DDu, 0XEAEC9DCCu, 0X6CED4CFFu, 0X111203EEu, 0X30C67D86u, 0X4D393297u, 0XCB38E3A4u, 0XB6C7ACB5u,
        0X88901F74u, 0XF56F5065u, 0X736E8156u, 0X0E91CE47u, 0X2F45B02Fu, 0X52BAFF3Eu, 0XD4BB2E0Du, 0XA944611Cu,
        0XD61B7F2Bu, 0XABE4303Au, 0X2DE5E109u, 0X501AAE18u, 0X71CED070u, 0X0C319F61u, 0X8A304E52u, 0XF7CF0143u,
        0XC998B282u, 0XB467FD93u, 0X32662CA0u, 0X4F9963B1u, 0X6E4D1DD9u, 0X13B252C8u, 0X95B383FBu, 0XE84CCCEAu,
        0XE91CE479u, 0X94E3AB68u, 0X12E27A5Bu, 0X6F1D354Au, 0X4EC94B22u, 0X33360433u, 0XB537D500u, 0XC8C89A11u,
        0XF69F29D0u, 0X8B6066C1u, 0X0D61B7F2u, 0X709EF8E3u, 0X514A868Bu, 0X2CB5C99Au, 0XAAB418A9u, 0XD74B57B8u,
        0X540A24C7u, 0X29F56BD6u, 0XAFF4BAE5u, 0XD20BF5F4u, 0XF3DF8B9Cu, 0X8E20C48Du, 0X082115BEu, 0X75DE5AAFu,
        0X4B89E96Eu, 0X3676A67Fu, 0XB077774Cu, 0XCD88385Du, 0XEC5C4635u, 0X91A30924u, 0X17A2D817u, 0X6A5D9706u,
        0X6B0DBF95u, 0X16F2F084u, 0X90F321B7u, 0XED0C6EA6u, 0XCCD810CEu, 0XB1275FDFu, 0X37268EECu, 0X4AD9C1FDu,
        0X748E723Cu, 0X09713D2Du, 0X8F70EC1Eu, 0XF28FA30Fu, 0XD35BDD67u, 0XAEA49276u, 0X28A54345u, 0X555A0C54u,
        0X2A051263u, 0X57FA5D72u, 0XD1FB8C41u, 0XAC04C350u, 0X8DD0BD38u, 0XF02FF229u, 0X762E231Au, 0X0BD16C0Bu,
        0X3586DFCAu, 0X487990DBu, 0XCE7841E8u, 0XB3870EF9u, 0X92537091u, 0XEFAC3F80u, 0X69ADEEB3u, 0X1452A1A2u,
        0X15028931u, 0X68FDC620u, 0XEEFC1713u, 0X93035802u, 0XB2D7266Au, 0XCF28697Bu, 0X4929B848u, 0X34D6F759u,
        0X0A814498u, 0X777E0B89u, 0XF17FDABAu, 0X8C8095ABu, 0XAD54EBC3u, 0XD0ABA4D2u, 0X56AA75E1u, 0X2B553AF0u
    };

    /* Lookup table index computed using input data byte */
    uint8_t iLUTIndex;

    /* Computed 21-bit CRC value */
    uint32_t iCrcValue;

    /* Initialize the CRC value with the initial seed */
    iCrcValue = WB_SCL_CRC_INITIAL_SEED_VALUE;

    for (uint8_t i = 0u; i < iLength; i++)
    {
        /* XOR-in next input byte into LSB of CRC and get the LSB */
        /* That's our new intermediate dividend */
        iLUTIndex = (uint8_t)((iCrcValue ^ pBuffer[i]) & (uint8_t)0xffu);

        /* Shift out the LSB used for division per look-up table and XOR with
         * the remainder */
        iCrcValue = (uint32_t)(((iCrcValue >> 8u) ^ iSclCrcLUT[iLUTIndex]) & 0xffffffffu);
    }

    /* Reverse the bits in the CRC to produce corrected CRC value*/
    *pCrc = wb_scl_BitReverseCrc (iCrcValue);
}

static void wb_scl_PackCrc (uint8_t * const pBuffer, uint32_t const * const pCrc)
{
    /* Write CRC value in big-endian format */
    pBuffer [0] = (uint8_t) ((*pCrc >> 24u) & 0xffu);
    pBuffer [1] = (uint8_t) ((*pCrc >> 16u) & 0xffu);
    pBuffer [2] = (uint8_t) ((*pCrc >> 8u) & 0xffu);
    pBuffer [3] = (uint8_t) ((*pCrc >> 0u) & 0xffu);
}

static void wb_scl_UnpackCrc (uint8_t const * const pBuffer, uint32_t * const pCrc)
{
    /* Unpack CRC value in big-endian format */
    *pCrc = (((((uint32_t) pBuffer [0]) << 24u) & 0xff000000u) |
             ((((uint32_t) pBuffer [1]) << 16u) & 0x00ff0000u) |
             ((((uint32_t) pBuffer [2]) << 8u) & 0x0000ff00u) |
             ((((uint32_t) pBuffer [3]) << 0u) & 0x000000ffu));
}

static adi_wil_safety_internals_t * wb_scl_GetSafetyInternalsPointer (adi_wil_pack_t const * const pPack)
{
    /* Return value of this method */
    adi_wil_safety_internals_t * pInternals;

    /* Initialize local variable to NULL */
    pInternals = (void *) 0;

    /* Check pack instance before dereferencing */
    if ((void *) 0 != pPack)
    {
        /* Assign safety internals pointer to local variable */
        pInternals = pPack->pSafetyInternals;
    }

    /* Return local variable pointer to safety internals */
    return pInternals;
}

static void wb_scl_ReadSCLHeader (uint8_t const * const pSCLFrame,
                                  wb_msg_header_t * pMsgHeader,
                                  uint8_t * pDestDeviceId)
{
    /* Unpack source device ID */
    pMsgHeader->iSourceDeviceId = pSCLFrame [WB_SCL_HDR_SOURCE_ID_OFFSET];

    /* Unpack destination device ID - this should match the host controller
     * ID */
    *pDestDeviceId = pSCLFrame [WB_SCL_HDR_DEST_ID_OFFSET];

    /* Unpack sequence number of the message */
    pMsgHeader->iSequenceNumber = pSCLFrame [WB_SCL_HDR_SEQ_NUM_OFFSET];

    /* Unpack message type */
    pMsgHeader->iMessageType = pSCLFrame [WB_SCL_HDR_MSG_TYPE_OFFSET];

    /* Unpack the length of the SCL payload excluding the header and CRC */
    pMsgHeader->iPayloadLength = pSCLFrame [WB_SCL_HDR_PAYLOAD_LEN_OFFSET];
}

static void wb_scl_WriteSCLHeader (uint8_t * const pSCLFrame,
                                   wb_msg_header_t const * const pMsgHeader)
{
    /* Pack source device ID for outgoing messages from the host controller */
    pSCLFrame [WB_SCL_HDR_SOURCE_ID_OFFSET] = WB_WIL_DEV_HOST_CONTROLLER;

    /* Pack destination device ID */
    pSCLFrame [WB_SCL_HDR_DEST_ID_OFFSET] = pMsgHeader->iSourceDeviceId;

    /* Pack sequence number of the message */
    pSCLFrame [WB_SCL_HDR_SEQ_NUM_OFFSET] = pMsgHeader->iSequenceNumber;

    /* Pack message type */
    pSCLFrame [WB_SCL_HDR_MSG_TYPE_OFFSET] = pMsgHeader->iMessageType;

    /* Pack the length of the SCL payload excluding the header and CRC */
    pSCLFrame [WB_SCL_HDR_PAYLOAD_LEN_OFFSET] = pMsgHeader->iPayloadLength;
}

static bool wb_scl_ValidateIncomingMessageHeader (wb_msg_header_t const * const pMsgHeader,
                                                  uint8_t iDestDeviceId,
                                                  uint8_t iSourceDeviceId)
{
    /* Returns true if all parameters are within valid range */
    /* Checks if SCL frame header is valid for an incoming message from a
     * safety CPU */
    /* Ensure SCL payload length is in valid range */
    /* Verify that device ID in the SCL header matches originating device ID */
    return ((pMsgHeader->iPayloadLength <= WB_SCL_PAYLOAD_LEN_MAX) &&
            (iDestDeviceId == WB_WIL_DEV_HOST_CONTROLLER) &&
            (pMsgHeader->iSourceDeviceId == iSourceDeviceId));
}

static bool wb_scl_ValidateOutgoingMessageHeader (wb_msg_header_t const * const pMsgHeader)
{
    /* Returns true if all parameters are within valid range */
    /* Checks if message header is valid for an outgoing message to one or all
     * safety CPUs */
    /* Checks if message is addressed to a valid node ID */
    /* Checks if SCL payload length exceeds maximum value */
    return (((pMsgHeader->iSourceDeviceId < ADI_WIL_MAX_NODES) || (pMsgHeader->iSourceDeviceId == WB_WIL_DEV_ALL_NODES)) &&
             (pMsgHeader->iPayloadLength <= WB_SCL_PAYLOAD_LEN_MAX));
}

static bool wb_scl_ValidateHandleSCLFrameParameters (adi_wil_safety_internals_t const * const pInternals,
                                                     uint8_t iDeviceId,
                                                     uint8_t iLength,
                                                     uint8_t const * const pData)
{
    /* Returns true if all parameters are within valid range */
    /* Ensure input pointers are not NULL */
    /* Ensure source device is a valid node or manager */
    /* Ensure length is within valid range */
    return ((pInternals != (void *) 0) &&
            (pData != (void *) 0) &&
            ((iDeviceId < ADI_WIL_MAX_NODES) || (iDeviceId == WB_SCL_MANAGER_0_DEVICE_ID) || (iDeviceId == WB_SCL_MANAGER_1_DEVICE_ID)) &&
            (iLength >= WB_SCL_FRAME_LEN_MIN) &&
            (iLength <= WB_SCL_FRAME_LEN_MAX));
}

static uint32_t wb_scl_BitReverseCrc (uint32_t iCrc)
{
    /* Look-up table containing the inverted bits for each possible value */
    static const uint32_t BitReverseLUT [] =
    {
      0x00u, 0x80u, 0x40u, 0xC0u, 0x20u, 0xA0u, 0x60u, 0xE0u,
      0x10u, 0x90u, 0x50u, 0xD0u, 0x30u, 0xB0u, 0x70u, 0xF0u,
      0x08u, 0x88u, 0x48u, 0xC8u, 0x28u, 0xA8u, 0x68u, 0xE8u,
      0x18u, 0x98u, 0x58u, 0xD8u, 0x38u, 0xB8u, 0x78u, 0xF8u,
      0x04u, 0x84u, 0x44u, 0xC4u, 0x24u, 0xA4u, 0x64u, 0xE4u,
      0x14u, 0x94u, 0x54u, 0xD4u, 0x34u, 0xB4u, 0x74u, 0xF4u,
      0x0Cu, 0x8Cu, 0x4Cu, 0xCCu, 0x2Cu, 0xACu, 0x6Cu, 0xECu,
      0x1Cu, 0x9Cu, 0x5Cu, 0xDCu, 0x3Cu, 0xBCu, 0x7Cu, 0xFCu,
      0x02u, 0x82u, 0x42u, 0xC2u, 0x22u, 0xA2u, 0x62u, 0xE2u,
      0x12u, 0x92u, 0x52u, 0xD2u, 0x32u, 0xB2u, 0x72u, 0xF2u,
      0x0Au, 0x8Au, 0x4Au, 0xCAu, 0x2Au, 0xAAu, 0x6Au, 0xEAu,
      0x1Au, 0x9Au, 0x5Au, 0xDAu, 0x3Au, 0xBAu, 0x7Au, 0xFAu,
      0x06u, 0x86u, 0x46u, 0xC6u, 0x26u, 0xA6u, 0x66u, 0xE6u,
      0x16u, 0x96u, 0x56u, 0xD6u, 0x36u, 0xB6u, 0x76u, 0xF6u,
      0x0Eu, 0x8Eu, 0x4Eu, 0xCEu, 0x2Eu, 0xAEu, 0x6Eu, 0xEEu,
      0x1Eu, 0x9Eu, 0x5Eu, 0xDEu, 0x3Eu, 0xBEu, 0x7Eu, 0xFEu,
      0x01u, 0x81u, 0x41u, 0xC1u, 0x21u, 0xA1u, 0x61u, 0xE1u,
      0x11u, 0x91u, 0x51u, 0xD1u, 0x31u, 0xB1u, 0x71u, 0xF1u,
      0x09u, 0x89u, 0x49u, 0xC9u, 0x29u, 0xA9u, 0x69u, 0xE9u,
      0x19u, 0x99u, 0x59u, 0xD9u, 0x39u, 0xB9u, 0x79u, 0xF9u,
      0x05u, 0x85u, 0x45u, 0xC5u, 0x25u, 0xA5u, 0x65u, 0xE5u,
      0x15u, 0x95u, 0x55u, 0xD5u, 0x35u, 0xB5u, 0x75u, 0xF5u,
      0x0Du, 0x8Du, 0x4Du, 0xCDu, 0x2Du, 0xADu, 0x6Du, 0xEDu,
      0x1Du, 0x9Du, 0x5Du, 0xDDu, 0x3Du, 0xBDu, 0x7Du, 0xFDu,
      0x03u, 0x83u, 0x43u, 0xC3u, 0x23u, 0xA3u, 0x63u, 0xE3u,
      0x13u, 0x93u, 0x53u, 0xD3u, 0x33u, 0xB3u, 0x73u, 0xF3u,
      0x0Bu, 0x8Bu, 0x4Bu, 0xCBu, 0x2Bu, 0xABu, 0x6Bu, 0xEBu,
      0x1Bu, 0x9Bu, 0x5Bu, 0xDBu, 0x3Bu, 0xBBu, 0x7Bu, 0xFBu,
      0x07u, 0x87u, 0x47u, 0xC7u, 0x27u, 0xA7u, 0x67u, 0xE7u,
      0x17u, 0x97u, 0x57u, 0xD7u, 0x37u, 0xB7u, 0x77u, 0xF7u,
      0x0Fu, 0x8Fu, 0x4Fu, 0xCFu, 0x2Fu, 0xAFu, 0x6Fu, 0xEFu,
      0x1Fu, 0x9Fu, 0x5Fu, 0xDFu, 0x3Fu, 0xBFu, 0x7Fu, 0xFFu
    };

    /* Look up each byte of the 32-bit value, invert the bits and put in the
     * reflected position */
    return ((BitReverseLUT [(iCrc >> 0u) & 0xFFu] << 24u) |
            (BitReverseLUT [(iCrc >> 8u) & 0xFFu] << 16u) |
            (BitReverseLUT [(iCrc >> 16u) & 0xFFu] << 8u) |
            (BitReverseLUT [(iCrc >> 24u) & 0xFFu] << 0u));
}

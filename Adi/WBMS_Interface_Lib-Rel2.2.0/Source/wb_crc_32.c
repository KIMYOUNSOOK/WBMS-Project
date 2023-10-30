/******************************************************************************
 * @file    wb_crc_32.c
 *
 * @brielf  CRC16 implementation source
 *
 * @details The CRC Logic used in this implementation was provided by
 *          Bastian Molkenthin (www.bastian-molkenthin.de)
 *
 * Copyright (c) 2019-2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#include "wb_crc_32.h"
#include "wb_crc_config.h"

/******************************************************************************
 * Function Definitions
 *****************************************************************************/

uint32_t wb_crc_ComputeCRC32 (uint8_t const * const pData,
                              uint32_t iLength,
                              uint32_t iSeedValue)
{
    /* Pre-computed CRC32 look up table using polynomial 0x9960034Cu */
    static const uint32_t iLUT [] = {
        0x00000000u, 0xCE58C2B1u, 0xAE7183FBu, 0x6029414Au, 0x6E23016Fu, 0xA07BC3DEu, 0xC0528294u, 0x0E0A4025u,
        0xDC4602DEu, 0x121EC06Fu, 0x72378125u, 0xBC6F4394u, 0xB26503B1u, 0x7C3DC100u, 0x1C14804Au, 0xD24C42FBu,
        0x8A4C0325u, 0x4414C194u, 0x243D80DEu, 0xEA65426Fu, 0xE46F024Au, 0x2A37C0FBu, 0x4A1E81B1u, 0x84464300u,
        0x560A01FBu, 0x9852C34Au, 0xF87B8200u, 0x362340B1u, 0x38290094u, 0xF671C225u, 0x9658836Fu, 0x580041DEu,
        0x265800D3u, 0xE800C262u, 0x88298328u, 0x46714199u, 0x487B01BCu, 0x8623C30Du, 0xE60A8247u, 0x285240F6u,
        0xFA1E020Du, 0x3446C0BCu, 0x546F81F6u, 0x9A374347u, 0x943D0362u, 0x5A65C1D3u, 0x3A4C8099u, 0xF4144228u,
        0xAC1403F6u, 0x624CC147u, 0x0265800Du, 0xCC3D42BCu, 0xC2370299u, 0x0C6FC028u, 0x6C468162u, 0xA21E43D3u,
        0x70520128u, 0xBE0AC399u, 0xDE2382D3u, 0x107B4062u, 0x1E710047u, 0xD029C2F6u, 0xB00083BCu, 0x7E58410Du,
        0x4CB001A6u, 0x82E8C317u, 0xE2C1825Du, 0x2C9940ECu, 0x229300C9u, 0xECCBC278u, 0x8CE28332u, 0x42BA4183u,
        0x90F60378u, 0x5EAEC1C9u, 0x3E878083u, 0xF0DF4232u, 0xFED50217u, 0x308DC0A6u, 0x50A481ECu, 0x9EFC435Du,
        0xC6FC0283u, 0x08A4C032u, 0x688D8178u, 0xA6D543C9u, 0xA8DF03ECu, 0x6687C15Du, 0x06AE8017u, 0xC8F642A6u,
        0x1ABA005Du, 0xD4E2C2ECu, 0xB4CB83A6u, 0x7A934117u, 0x74990132u, 0xBAC1C383u, 0xDAE882C9u, 0x14B04078u,
        0x6AE80175u, 0xA4B0C3C4u, 0xC499828Eu, 0x0AC1403Fu, 0x04CB001Au, 0xCA93C2ABu, 0xAABA83E1u, 0x64E24150u,
        0xB6AE03ABu, 0x78F6C11Au, 0x18DF8050u, 0xD68742E1u, 0xD88D02C4u, 0x16D5C075u, 0x76FC813Fu, 0xB8A4438Eu,
        0xE0A40250u, 0x2EFCC0E1u, 0x4ED581ABu, 0x808D431Au, 0x8E87033Fu, 0x40DFC18Eu, 0x20F680C4u, 0xEEAE4275u,
        0x3CE2008Eu, 0xF2BAC23Fu, 0x92938375u, 0x5CCB41C4u, 0x52C101E1u, 0x9C99C350u, 0xFCB0821Au, 0x32E840ABu,
        0x9960034Cu, 0x5738C1FDu, 0x371180B7u, 0xF9494206u, 0xF7430223u, 0x391BC092u, 0x593281D8u, 0x976A4369u,
        0x45260192u, 0x8B7EC323u, 0xEB578269u, 0x250F40D8u, 0x2B0500FDu, 0xE55DC24Cu, 0x85748306u, 0x4B2C41B7u,
        0x132C0069u, 0xDD74C2D8u, 0xBD5D8392u, 0x73054123u, 0x7D0F0106u, 0xB357C3B7u, 0xD37E82FDu, 0x1D26404Cu,
        0xCF6A02B7u, 0x0132C006u, 0x611B814Cu, 0xAF4343FDu, 0xA14903D8u, 0x6F11C169u, 0x0F388023u, 0xC1604292u,
        0xBF38039Fu, 0x7160C12Eu, 0x11498064u, 0xDF1142D5u, 0xD11B02F0u, 0x1F43C041u, 0x7F6A810Bu, 0xB13243BAu,
        0x637E0141u, 0xAD26C3F0u, 0xCD0F82BAu, 0x0357400Bu, 0x0D5D002Eu, 0xC305C29Fu, 0xA32C83D5u, 0x6D744164u,
        0x357400BAu, 0xFB2CC20Bu, 0x9B058341u, 0x555D41F0u, 0x5B5701D5u, 0x950FC364u, 0xF526822Eu, 0x3B7E409Fu,
        0xE9320264u, 0x276AC0D5u, 0x4743819Fu, 0x891B432Eu, 0x8711030Bu, 0x4949C1BAu, 0x296080F0u, 0xE7384241u,
        0xD5D002EAu, 0x1B88C05Bu, 0x7BA18111u, 0xB5F943A0u, 0xBBF30385u, 0x75ABC134u, 0x1582807Eu, 0xDBDA42CFu,
        0x09960034u, 0xC7CEC285u, 0xA7E783CFu, 0x69BF417Eu, 0x67B5015Bu, 0xA9EDC3EAu, 0xC9C482A0u, 0x079C4011u,
        0x5F9C01CFu, 0x91C4C37Eu, 0xF1ED8234u, 0x3FB54085u, 0x31BF00A0u, 0xFFE7C211u, 0x9FCE835Bu, 0x519641EAu,
        0x83DA0311u, 0x4D82C1A0u, 0x2DAB80EAu, 0xE3F3425Bu, 0xEDF9027Eu, 0x23A1C0CFu, 0x43888185u, 0x8DD04334u,
        0xF3880239u, 0x3DD0C088u, 0x5DF981C2u, 0x93A14373u, 0x9DAB0356u, 0x53F3C1E7u, 0x33DA80ADu, 0xFD82421Cu,
        0x2FCE00E7u, 0xE196C256u, 0x81BF831Cu, 0x4FE741ADu, 0x41ED0188u, 0x8FB5C339u, 0xEF9C8273u, 0x21C440C2u,
        0x79C4011Cu, 0xB79CC3ADu, 0xD7B582E7u, 0x19ED4056u, 0x17E70073u, 0xD9BFC2C2u, 0xB9968388u, 0x77CE4139u,
        0xA58203C2u, 0x6BDAC173u, 0x0BF38039u, 0xC5AB4288u, 0xCBA102ADu, 0x05F9C01Cu, 0x65D08156u, 0xAB8843E7u
    };

    /* Storage for return value of this function */
    uint32_t iValue;

    /* Initialise the CRC value with the initial seed */
    iValue = iSeedValue;
    
    /* Validate input parameters */
    if ((void *) 0 != pData)
    {
        /* LSB configuration for CRC */
        for (uint32_t i = 0u; i < iLength; i++)
        {
            /* XOR-in next input byte into LSB of crc and get this LSB, that's our new intermediate divident */
            /* Shift out the LSB used for division per look-up table and XOR with the remainder */
            iValue = (iValue >> 8) ^ iLUT [((iValue ^ pData [i]) & 0xFFu)];
        }
    }

    /* Return calculated CRC value */
    return iValue;
}

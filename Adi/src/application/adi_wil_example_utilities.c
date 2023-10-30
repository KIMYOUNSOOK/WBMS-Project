/******************************************************************************
 * @brief   Collection of print utility functions
 *
 * @details Collection of helper functions for printing
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#include "adi_wil_example_utilities.h"
#include <stdio.h>
#include "adi_wil_version.h"


const char *adi_wil_ErrorToString(adi_wil_err_t err_code)
{
    const char *err_code_string;

    switch (err_code)
    {
    case ADI_WIL_ERR_SUCCESS:
        err_code_string = "ADI_WIL_ERR_SUCCESS";
        break;
    case ADI_WIL_ERR_FAIL:
        err_code_string = "ADI_WIL_ERR_FAIL";
        break;
    case ADI_WIL_ERR_API_IN_PROGRESS:
        err_code_string = "ADI_WIL_ERR_API_IN_PROGRESS";
        break;
    case ADI_WIL_ERR_TIMEOUT:
        err_code_string = "ADI_WIL_ERR_TIMEOUT";
        break;
    case ADI_WIL_ERR_NOT_CONNECTED:
        err_code_string = "ADI_WIL_ERR_NOT_CONNECTED";
        break;
    case ADI_WIL_ERR_INVALID_PARAMETER:
        err_code_string = "ADI_WIL_ERR_INVALID_PARAMETER";
        break;
    case ADI_WIL_ERR_INVALID_STATE:
        err_code_string = "ADI_WIL_ERR_INVALID_STATE";
        break;
    case ADI_WIL_ERR_NOT_SUPPORTED:
        err_code_string = "ADI_WIL_ERR_NOT_SUPPORTED";
        break;
    case ADI_WIL_ERR_EXTERNAL:
        err_code_string = "ADI_WIL_ERR_EXTERNAL";
        break;
    case ADI_WIL_ERR_INVALID_MODE:
        err_code_string = "ADI_WIL_ERR_INVALID_MODE";
        break;
    case ADI_WIL_ERR_IN_PROGRESS:
        err_code_string = "ADI_WIL_ERR_IN_PROGRESS";
        break;
    case ADI_WIL_ERR_CONFIGURATION_MISMATCH:
        err_code_string = "ADI_WIL_ERR_CONFIGURATION_MISMATCH";
        break;
    case ADI_WIL_ERR_CRC:
        err_code_string = "ADI_WIL_ERR_CRC";
        break;
    case ADI_WIL_ERR_FILE_REJECTED:
        err_code_string = "ADI_WIL_ERR_FILE_REJECTED";
        break;
    case ADI_WIL_ERR_PARTIAL_SUCCESS:
        err_code_string = "ADI_WIL_ERR_PARTIAL_SUCCESS";
        break;
    default:
        err_code_string = "ERR CODE NOT IMPLEMENTED ";
        break;
    }

    return err_code_string;
}


const char *adi_wil_SystemModeToString(adi_wil_mode_t sysmode)
{
    const char *sysmode_string;

    switch (sysmode)
    {
    case ADI_WIL_MODE_STANDBY:
        sysmode_string = "ADI_WIL_MODE_STANDBY";
        break;
    case ADI_WIL_MODE_ACTIVE:
        sysmode_string = "ADI_WIL_MODE_ACTIVE";
        break;
    case ADI_WIL_MODE_COMMISSIONING:
        sysmode_string = "ADI_WIL_MODE_COMMISSIONING";
        break;
    case ADI_WIL_MODE_MONITORING:
        sysmode_string = "ADI_WIL_MODE_MONITORING";
        break;
    default:
        sysmode_string = "UNKNOWN MODE";
        break;
    }

    return sysmode_string;
}


const char *adi_wil_DeviceToString(adi_wil_device_t eDeviceId)
{
    const char *device_id_string;

    switch (eDeviceId)
    {
        case 0u:
            device_id_string = "ADI_WIL_DEV_NODE_0";
            break;
        case 1u:
            device_id_string = "ADI_WIL_DEV_NODE_1";
            break;
        case 2u:
            device_id_string = "ADI_WIL_DEV_NODE_2";
            break;
        case 3u:
            device_id_string = "ADI_WIL_DEV_NODE_3";
            break;
        case 4u:
            device_id_string = "ADI_WIL_DEV_NODE_4";
            break;
        case 5u:
            device_id_string = "ADI_WIL_DEV_NODE_5";
            break;
        case 6u:
            device_id_string = "ADI_WIL_DEV_NODE_6";
            break;
        case 7u:
            device_id_string = "ADI_WIL_DEV_NODE_7";
            break;
        case 8u:
            device_id_string = "ADI_WIL_DEV_NODE_8";
            break;
        case 9u:
            device_id_string = "ADI_WIL_DEV_NODE_9";
            break;
        case 10u:
            device_id_string = "ADI_WIL_DEV_NODE_10";
            break;
        case 11u:
            device_id_string = "ADI_WIL_DEV_NODE_11";
            break;
        case 12u:
            device_id_string = "ADI_WIL_DEV_NODE_12";
            break;
        case 13u:
            device_id_string = "ADI_WIL_DEV_NODE_13";
            break;
        case 14u:
            device_id_string = "ADI_WIL_DEV_NODE_14";
            break;
        case 15u:
            device_id_string = "ADI_WIL_DEV_NODE_15";
            break;
        case 16u:
            device_id_string = "ADI_WIL_DEV_NODE_16";
            break;
        case 17u:
            device_id_string = "ADI_WIL_DEV_NODE_17";
            break;
        case 18u:
            device_id_string = "ADI_WIL_DEV_NODE_18";
            break;
        case 19u:
            device_id_string = "ADI_WIL_DEV_NODE_19";
            break;
        case 20u:
            device_id_string = "ADI_WIL_DEV_NODE_20";
            break;
        case 21u:
            device_id_string = "ADI_WIL_DEV_NODE_21";
            break;
        case 22u:
            device_id_string = "ADI_WIL_DEV_NODE_22";
            break;
        case 23u:
            device_id_string = "ADI_WIL_DEV_NODE_23";
            break;
        case 24u:
            device_id_string = "ADI_WIL_DEV_NODE_24";
            break;
        case 25u:
            device_id_string = "ADI_WIL_DEV_NODE_25";
            break;
        case 26u:
            device_id_string = "ADI_WIL_DEV_NODE_26";
            break;
        case 27u:
            device_id_string = "ADI_WIL_DEV_NODE_27";
            break;
        case 28u:
            device_id_string = "ADI_WIL_DEV_NODE_28";
            break;
        case 29u:
            device_id_string = "ADI_WIL_DEV_NODE_29";
            break;
        case 30u:
            device_id_string = "ADI_WIL_DEV_NODE_30";
            break;
        case 31u:
            device_id_string = "ADI_WIL_DEV_NODE_31";
            break;
        case 32u:
            device_id_string = "ADI_WIL_DEV_NODE_32";
            break;
        case 33u:
            device_id_string = "ADI_WIL_DEV_NODE_33";
            break;
        case 34u:
            device_id_string = "ADI_WIL_DEV_NODE_34";
            break;
        case 35u:
            device_id_string = "ADI_WIL_DEV_NODE_35";
            break;
        case 36u:
            device_id_string = "ADI_WIL_DEV_NODE_36";
            break;
        case 37u:
            device_id_string = "ADI_WIL_DEV_NODE_37";
            break;
        case 38u:
            device_id_string = "ADI_WIL_DEV_NODE_38";
            break;
        case 39u:
            device_id_string = "ADI_WIL_DEV_NODE_39";
            break;
        case 40u:
            device_id_string = "ADI_WIL_DEV_NODE_40";
            break;
        case 41u:
            device_id_string = "ADI_WIL_DEV_NODE_41";
            break;
        case 42u:
            device_id_string = "ADI_WIL_DEV_NODE_42";
            break;
        case 43u:
            device_id_string = "ADI_WIL_DEV_NODE_43";
            break;
        case 44u:
            device_id_string = "ADI_WIL_DEV_NODE_44";
            break;
        case 45u:
            device_id_string = "ADI_WIL_DEV_NODE_45";
            break;
        case 46u:
            device_id_string = "ADI_WIL_DEV_NODE_46";
            break;
        case 47u:
            device_id_string = "ADI_WIL_DEV_NODE_47";
            break;
        case 48u:
            device_id_string = "ADI_WIL_DEV_NODE_48";
            break;
        case 49u:
            device_id_string = "ADI_WIL_DEV_NODE_49";
            break;
        case 50u:
            device_id_string = "ADI_WIL_DEV_NODE_50";
            break;
        case 51u:
            device_id_string = "ADI_WIL_DEV_NODE_51";
            break;
        case 52u:
            device_id_string = "ADI_WIL_DEV_NODE_52";
            break;
        case 53u:
            device_id_string = "ADI_WIL_DEV_NODE_53";
            break;
        case 54u:
            device_id_string = "ADI_WIL_DEV_NODE_54";
            break;
        case 55u:
            device_id_string = "ADI_WIL_DEV_NODE_55";
            break;
        case 56u:
            device_id_string = "ADI_WIL_DEV_NODE_56";
            break;
        case 57u:
            device_id_string = "ADI_WIL_DEV_NODE_57";
            break;
        case 58u:
            device_id_string = "ADI_WIL_DEV_NODE_58";
            break;
        case 59u:
            device_id_string = "ADI_WIL_DEV_NODE_59";
            break;
        case 60u:
            device_id_string = "ADI_WIL_DEV_NODE_60";
            break;
        case 61u:
            device_id_string = "ADI_WIL_DEV_NODE_61";
            break;
        case 62u:
            device_id_string = "ADI_WIL_DEV_NODE_62";
            break;
        case 63u:
            device_id_string = "ADI_WIL_DEV_NODE_63";
            break;
        case 240u:
            device_id_string = "ADI_WIL_DEV_MANAGER_0";
            break;
        case 241u:
            device_id_string = "ADI_WIL_DEV_MANAGER_1";
            break;
        case 254u:
            device_id_string = "ADI_WIL_DEV_ALL_MANAGERS";
            break;
        case 255u:
            device_id_string = "ADI_WIL_DEV_ALL_NODES";
            break;
        default:
            device_id_string = "UNKNOWN DEVICE";
            break;
    }

    return device_id_string;
}


/* writes a given version to a string buffer using sprintf */
static void sprintVersion(char * str, adi_wil_version_t * const pVersion)
{
    sprintf(str, "%u.%u.%u.%u", pVersion->iVersionMajor, pVersion->iVersionMinor, pVersion->iVersionPatch, pVersion->iVersionBuild);
}


/* Prints a version buffer */
const char * adi_wil_VersionToString(adi_wil_version_t * const pVersion)
{
    static char versionBuffer[24];
    sprintVersion(versionBuffer, pVersion);
    return versionBuffer;
}


/* writes a given mac to a string buffer using sprintf */
void adi_wil_SprintMAC(char * str, uint8_t const mac[])
{
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);
}

uint8_t adi_cnt1_8bit(uint8_t val)
{
    uint8_t cnt = 0;
    while (val != 0){
        cnt += val & 1;
        val >>= 1;
    }
    return cnt;
}
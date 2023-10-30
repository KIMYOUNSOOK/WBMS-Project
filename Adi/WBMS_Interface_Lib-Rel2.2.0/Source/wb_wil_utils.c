/********************************************************************************
 * @file    wb_wil_utils.c
 *
 * @brief   Functions to convert values to enumerated types and similar
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *******************************************************************************/

#include "wb_wil_utils.h"
#include "wbms_cmd_defs.h"
#include "wbms_cmd_mgr_defs.h"
#include "wbms_cmd_node_defs.h"
#include "wb_req_set_customer_identifier.h"
#include "adi_wil_osal.h"
#include "wb_wil_device.h"

/******************************************************************************
 * Protocol version compilation checks
 *****************************************************************************/

#define ADI_WIL_SPI_PROTOCOL_VERSION (5u)

#if (ADI_WIL_SPI_PROTOCOL_VERSION != WBMS_SPI_PROTOCOL_VERSION)
#error "Invalid SPI protocol version detected."
#endif

#if ((ADI_WIL_DEV_NODE_0 != (1ULL << 0u)) || \
     (ADI_WIL_DEV_NODE_1 != (1ULL << 1u)) || \
     (ADI_WIL_DEV_NODE_2 != (1ULL << 2u)) || \
     (ADI_WIL_DEV_NODE_3 != (1ULL << 3u)) || \
     (ADI_WIL_DEV_NODE_4 != (1ULL << 4u)) || \
     (ADI_WIL_DEV_NODE_5 != (1ULL << 5u)) || \
     (ADI_WIL_DEV_NODE_6 != (1ULL << 6u)) || \
     (ADI_WIL_DEV_NODE_7 != (1ULL << 7u)) || \
     (ADI_WIL_DEV_NODE_8 != (1ULL << 8u)) || \
     (ADI_WIL_DEV_NODE_9 != (1ULL << 9u)) || \
     (ADI_WIL_DEV_NODE_7 != (1ULL << 7u)) || \
     (ADI_WIL_DEV_NODE_8 != (1ULL << 8u)) || \
     (ADI_WIL_DEV_NODE_9 != (1ULL << 9u)) || \
     (ADI_WIL_DEV_NODE_10 != (1ULL << 10u)) || \
     (ADI_WIL_DEV_NODE_11 != (1ULL << 11u)) || \
     (ADI_WIL_DEV_NODE_12 != (1ULL << 12u)) || \
     (ADI_WIL_DEV_NODE_13 != (1ULL << 13u)) || \
     (ADI_WIL_DEV_NODE_14 != (1ULL << 14u)) || \
     (ADI_WIL_DEV_NODE_15 != (1ULL << 15u)) || \
     (ADI_WIL_DEV_NODE_16 != (1ULL << 16u)) || \
     (ADI_WIL_DEV_NODE_17 != (1ULL << 17u)) || \
     (ADI_WIL_DEV_NODE_18 != (1ULL << 18u)) || \
     (ADI_WIL_DEV_NODE_19 != (1ULL << 19u)) || \
     (ADI_WIL_DEV_NODE_20 != (1ULL << 20u)) || \
     (ADI_WIL_DEV_NODE_21 != (1ULL << 21u)) || \
     (ADI_WIL_DEV_NODE_22 != (1ULL << 22u)) || \
     (ADI_WIL_DEV_NODE_23 != (1ULL << 23u)) || \
     (ADI_WIL_DEV_NODE_24 != (1ULL << 24u)) || \
     (ADI_WIL_DEV_NODE_25 != (1ULL << 25u)) || \
     (ADI_WIL_DEV_NODE_26 != (1ULL << 26u)) || \
     (ADI_WIL_DEV_NODE_27 != (1ULL << 27u)) || \
     (ADI_WIL_DEV_NODE_28 != (1ULL << 28u)) || \
     (ADI_WIL_DEV_NODE_29 != (1ULL << 29u)) || \
     (ADI_WIL_DEV_NODE_30 != (1ULL << 30u)) || \
     (ADI_WIL_DEV_NODE_31 != (1ULL << 31u)) || \
     (ADI_WIL_DEV_NODE_32 != (1ULL << 32u)) || \
     (ADI_WIL_DEV_NODE_33 != (1ULL << 33u)) || \
     (ADI_WIL_DEV_NODE_34 != (1ULL << 34u)) || \
     (ADI_WIL_DEV_NODE_35 != (1ULL << 35u)) || \
     (ADI_WIL_DEV_NODE_36 != (1ULL << 36u)) || \
     (ADI_WIL_DEV_NODE_37 != (1ULL << 37u)) || \
     (ADI_WIL_DEV_NODE_38 != (1ULL << 38u)) || \
     (ADI_WIL_DEV_NODE_39 != (1ULL << 39u)) || \
     (ADI_WIL_DEV_NODE_40 != (1ULL << 40u)) || \
     (ADI_WIL_DEV_NODE_41 != (1ULL << 41u)) || \
     (ADI_WIL_DEV_NODE_42 != (1ULL << 42u)) || \
     (ADI_WIL_DEV_NODE_43 != (1ULL << 43u)) || \
     (ADI_WIL_DEV_NODE_44 != (1ULL << 44u)) || \
     (ADI_WIL_DEV_NODE_45 != (1ULL << 45u)) || \
     (ADI_WIL_DEV_NODE_46 != (1ULL << 46u)) || \
     (ADI_WIL_DEV_NODE_47 != (1ULL << 47u)) || \
     (ADI_WIL_DEV_NODE_48 != (1ULL << 48u)) || \
     (ADI_WIL_DEV_NODE_49 != (1ULL << 49u)) || \
     (ADI_WIL_DEV_NODE_50 != (1ULL << 50u)) || \
     (ADI_WIL_DEV_NODE_51 != (1ULL << 51u)) || \
     (ADI_WIL_DEV_NODE_52 != (1ULL << 52u)) || \
     (ADI_WIL_DEV_NODE_53 != (1ULL << 53u)) || \
     (ADI_WIL_DEV_NODE_54 != (1ULL << 54u)) || \
     (ADI_WIL_DEV_NODE_55 != (1ULL << 55u)) || \
     (ADI_WIL_DEV_NODE_56 != (1ULL << 56u)) || \
     (ADI_WIL_DEV_NODE_57 != (1ULL << 57u)) || \
     (ADI_WIL_DEV_NODE_58 != (1ULL << 58u)) || \
     (ADI_WIL_DEV_NODE_59 != (1ULL << 59u)) || \
     (ADI_WIL_DEV_NODE_60 != (1ULL << 60u)) || \
     (ADI_WIL_DEV_NODE_61 != (1ULL << 61u)) || \
     (ADI_WIL_DEV_MANAGER_0 != (1ULL << 62u)) || \
     (ADI_WIL_DEV_MANAGER_1 != (1ULL << 63u)) || \
     (ADI_WIL_DEV_ALL_MANAGERS != (0xC000000000000000u)) || \
     (ADI_WIL_DEV_ALL_NODES != (0x3FFFFFFFFFFFFFFFu)))
#error "Invalid Device ID definitions."
#endif

#if (ADI_WIL_MAX_SCRIPT_CHANGE_DATA_LENGTH != WBMS_MAX_MODIFY_SCRIPT_LEN)   || \
    (ADI_WIL_CONTEXTUAL_DATA_SIZE != WBMS_MAX_CONTEXTUAL_DATA_LEN)          || \
    (ADI_WIL_LOADFILE_DATA_SIZE != WBMS_OTAP_BLOCK_SIZE)                    || \
    (ADI_WIL_MAX_CUSTOMER_IDENTIFIER_DATA_LEN != WB_MAX_CUSTOMER_ID_SIZE)   || \
    (ADI_WIL_GETFILE_DATA_OFFSET != WB_WIL_OTAP_FILE_HEADER_LEN)            || \
    (ADI_WIL_DEVICE_MASK_LEN != WBMS_NODE_BITMAP_SIZE)
#error "Invalid protocol payload length."
#endif

#if (ADI_WIL_FAULT_MASK_PLAUSIBILITY != WBMS_FAULT_PLAUSABILITY)              || \
    (ADI_WIL_FAULT_MASK_SENSOR_COMMS != WBMS_FAULT_SENSOR_COMMS)              || \
    (ADI_WIL_FAULT_MASK_CONTROL_FLOW != WBMS_FAULT_CONTROL_FLOW)              || \
    (ADI_WIL_FAULT_MASK_M2M_COMMS != WBMS_FAULT_M2M_COMMS)                    || \
    (ADI_WIL_FAULT_MASK_SERVICE_DISABLE != WBMS_FAULT_SERVICE_DISABLE)        || \
    (ADI_WIL_FAULT_MASK_PERIODIC_ASSERTION != WBMS_FAULT_PERIODIC_ASSERTION)  || \
    (ADI_WIL_FAULT_MASK_NETWORK_COMMS != WBMS_FAULT_NETWORK_COMMS)
#error "Invalid protocol version detected for fault types."
#endif

/******************************************************************************
 * Public functions
 *****************************************************************************/

adi_wil_err_t wb_wil_SensorIdToUint (adi_wil_sensor_id_t eSensorId, uint8_t * pSensorId)
{
    adi_wil_err_t rc = ADI_WIL_ERR_SUCCESS;

    /* Validate the input parameter */
    if (pSensorId != (void *) 0)
    {
        /* Map sensor type from adi_wil_sensor_id_t to uint8_t* */
        if (eSensorId == ADI_WIL_SENSOR_ID_BMS)
        {
            *pSensorId = WBMS_SENSOR_ID_BMS;
        }
        else if (eSensorId == ADI_WIL_SENSOR_ID_PMS)
        {
            *pSensorId = WBMS_SENSOR_ID_PMS;
        }
        else if (eSensorId == ADI_WIL_SENSOR_ID_EMS)
        {
            *pSensorId = WBMS_SENSOR_ID_EMS;
        }
        else
        {
            /* If neither of the above match then, that input is not supported.
             * Hence send a return code of INVALID PARAMETER */
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
        }
    }
    else
    {
        /* Input is invalid, so return code as INVALID PARAMETER */
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }

    return rc;
}

uint8_t wb_wil_GPIOtoUint (adi_wil_gpio_id_t GPIOPin)
{
    /* This snippet supports only 6 GPIO pin conversions */
    uint8_t iGPIOPin;

    /* Map GPIO value from adi_wil_gpio_id_t to uint8_t */
    if (ADI_WIL_GPIO_0 == GPIOPin)
    {
        iGPIOPin = 0u;
    }
    else if (ADI_WIL_GPIO_2 == GPIOPin)
    {
        iGPIOPin = 2u;
    }
    else if (ADI_WIL_GPIO_7 == GPIOPin)
    {
        iGPIOPin = 7u;
    }
    else if (ADI_WIL_GPIO_8 == GPIOPin)
    {
        iGPIOPin = 8u;
    }
    else if (ADI_WIL_GPIO_9 == GPIOPin)
    {
        iGPIOPin = 9u;
    }
    else
    {
        /* If neither of the above match, then, default to 10u */
        iGPIOPin = 10u;
    }

    return iGPIOPin;
}

adi_wil_err_t wb_wil_GetGpioValueFromUint (uint8_t iGpioValue, adi_wil_gpio_value_t * const pGPIOValue)
{
    adi_wil_err_t rc;

    rc = ADI_WIL_ERR_SUCCESS;

    /* Validate the input parameter */
    if (pGPIOValue != (void *) 0)
    {
        /* Map GPIO value from uint8_t to adi_wil_gpio_value_t* */
        switch (iGpioValue)
        {
            case 0u:
                *pGPIOValue = ADI_WIL_GPIO_LOW;
                break;
            case 1u:
                *pGPIOValue = ADI_WIL_GPIO_HIGH;
                break;
            default:
                rc = ADI_WIL_ERR_FAIL;
                break;
        }
    }
    else {
        /* Input is invalid, so return code as INVALID PARAMETER */
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }

    return rc;
}

adi_wil_err_t wb_wil_ContextualIDToUint (adi_wil_contextual_id_t eContextualId, uint8_t * pContextualId)
{
    adi_wil_err_t rc = ADI_WIL_ERR_SUCCESS;

    /* Validate the input parameter */
    if (pContextualId != (void *) 0)
    {
        /* Map contextual type from adi_wil_contextual_id_t to uint8_t* */
        switch (eContextualId)
        {
            case ADI_WIL_CONTEXTUAL_ID_0:
                *pContextualId = WBMS_CONTEXTUAL_ID_0;
                break;
            case ADI_WIL_CONTEXTUAL_ID_1:
                *pContextualId = WBMS_CONTEXTUAL_ID_1;
                break;
            case ADI_WIL_CONTEXTUAL_ID_WRITE_ONCE:
                *pContextualId = WBMS_CONTEXTUAL_ID_WRITE_ONCE;
                break;
            default:
                /* If neither of the above match, then, that input is not
                 * supported. Hence send a return code of INVALID PARAMETER */
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
                break;
        }
    }
    else
    {
        /* Input is invalid, so return code as INVALID PARAMETER */
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }

    return rc;
}

uint8_t wb_wil_GetUintFromCustomerIdentifier (adi_wil_customer_identifier_t eCustomerIdentifier)
{
    uint8_t iCustomerIdentifier;

    /* Map type from adi_wil_customer_identifier_t to uint8_t */
    if (ADI_WIL_CUSTOMER_IDENTIFIER_0 == eCustomerIdentifier)
    {
        iCustomerIdentifier = WBMS_CUSTOMER_IDENTIFIER_ID_0;
    }
    else if (ADI_WIL_CUSTOMER_IDENTIFIER_1 == eCustomerIdentifier)
    {
        iCustomerIdentifier = WBMS_CUSTOMER_IDENTIFIER_ID_1;
    }
    else
    {
        /* Because the input customer ID is of type
         * adi_wil_customer_identifier_t, if it is neither ID 0 or 1, it
         * has to be 2 */
        iCustomerIdentifier = WBMS_CUSTOMER_IDENTIFIER_ID_2;
    }

    return iCustomerIdentifier;
}


adi_wil_err_t wb_wil_GetErrFromUint (uint8_t iRC)
{
    adi_wil_err_t rc;

    switch (iRC)
    {
        /* Map return code type from uint8_t to adi_wil_err_t */
        case WBMS_CMD_RC_SUCCESS:
            rc = ADI_WIL_ERR_SUCCESS;
            break;
        case WBMS_CMD_RC_FAILED:
            rc = ADI_WIL_ERR_FAIL;
            break;
        case WBMS_CMD_RC_INVALID_ARGUMENT:
            /* If return code is an invalid argument, then, map it to INVALID
             * PARAMETER */
            rc = ADI_WIL_ERR_INVALID_PARAMETER;
            break;
        case WBMS_CMD_RC_WAIT:
            rc = ADI_WIL_ERR_IN_PROGRESS;
            break;
        case WBMS_CMD_RC_NOT_SUPPORTED:
            rc = ADI_WIL_ERR_NOT_SUPPORTED;
            break;
        case WBMS_CMD_RC_CRC_ERROR:
            rc = ADI_WIL_ERR_CRC;
            break;
        case WBMS_CMD_RC_FILE_REJECTED:
            rc = ADI_WIL_ERR_FILE_REJECTED;
            break;
        case WBMS_CMD_RC_ERR_STATE:
            /* Error state is mapped to INVALID STATE */
            rc = ADI_WIL_ERR_INVALID_STATE;
            break;
        default:
            /* If none of the above match, default it to FAIL */
            rc = ADI_WIL_ERR_FAIL;
            break;
    }

    return rc;
}

void wb_wil_IncrementWithRollover8 (uint8_t * pValue)
{
    /* Validate the input parameter */
    if (pValue != (void *) 0)
    {
        /* 8 bit Input roll over not happened yet */
        if (*pValue < ((uint8_t) UINT8_MAX))
        {
            (*pValue)++;
        }
        else
        {
            /* 8 bit Input rolled over */
            *pValue = 0u;
        }
    }
}

void wb_wil_IncrementWithRollover16 (uint16_t * pValue)
{
    /* Validate the input parameter */
    if (pValue != (void *) 0)
    {
        if (*pValue < ((uint16_t) UINT16_MAX))
        {
            /* 16 bit Input roll over not happened yet */
            (*pValue)++;
        }
        else
        {
            /* 16 bit Input rolled over */
            *pValue = 0u;
        }
    }
}

void wb_wil_IncrementWithRollover32 (uint32_t * pValue)
{
    /* Validate the input parameter */
    if (pValue != (void *) 0)
    {
        if (*pValue < ((uint32_t) UINT32_MAX))
        {
            /* 32 bit Input roll over not happened yet */
            (*pValue)++;
        }
        else
        {
            /* 32 bit Input rolled over */
            *pValue = 0u;
        }
    }
}

uint8_t wb_wil_GetUintFromWILMode (adi_wil_mode_t eMode)
{
    /* This snippet supports only 6 modes */
    uint8_t iMode;

    /* Map mode type from adi_wil_mode_t to uint8_t */
    if (eMode == ADI_WIL_MODE_STANDBY)
    {
        iMode = WBMS_MODE_STANDBY;
    }
    else if (eMode == ADI_WIL_MODE_ACTIVE)
    {
        iMode = WBMS_MODE_ACTIVE;
    }
    else if (eMode == ADI_WIL_MODE_COMMISSIONING)
    {
        iMode = WBMS_MODE_COMMISSIONING;
    }
    else if (eMode == ADI_WIL_MODE_MONITORING)
    {
        iMode = WBMS_MODE_MONITORING;
    }
    else if (eMode == ADI_WIL_MODE_OTAP)
    {
        iMode = WBMS_MODE_OTAP;
    }
    else if (eMode == ADI_WIL_MODE_SLEEP)
    {
        iMode = WBMS_MODE_SLEEP;
    }
    else
    {
        /* If neither of the above match, then default it to
         * WBMS_MODE_STANDBY */
        iMode = WBMS_MODE_STANDBY;
    }

    return iMode;
}


adi_wil_err_t wb_wil_GetUintFromFileType (adi_wil_file_type_t eFileType, uint8_t * pFileType)
{
    adi_wil_err_t rc;

    if ((void *) 0 == pFileType)
    {
        rc = ADI_WIL_ERR_INVALID_PARAMETER;
    }
    else
    {
        rc = ADI_WIL_ERR_SUCCESS;

        switch (eFileType)
        {
            case ADI_WIL_FILE_TYPE_FIRMWARE:
                *pFileType = WBMS_FILE_TYPE_FIRMWARE;
                break;
            case ADI_WIL_FILE_TYPE_BMS_CONTAINER:
                *pFileType = WBMS_FILE_TYPE_BMS_CONTAINER;
                break;
            case ADI_WIL_FILE_TYPE_CONFIGURATION:
                *pFileType = WBMS_FILE_TYPE_CONFIGURATION;
                break;
            case ADI_WIL_FILE_TYPE_BLACK_BOX_LOG:
                *pFileType = WBMS_FILE_TYPE_BLACK_BOX_LOG;
                break;
            case ADI_WIL_FILE_TYPE_INVENTORY_LOG:
                *pFileType = WBMS_FILE_TYPE_INVENTORY_LOG;
                break;
            case ADI_WIL_FILE_TYPE_ENV_MON:
                *pFileType = WBMS_FILE_TYPE_ENV_MON;
                break;
            case ADI_WIL_FILE_TYPE_PMS_CONTAINER:
                *pFileType = WBMS_FILE_TYPE_PMS_CONTAINER;
                break;
            case ADI_WIL_FILE_TYPE_EMS_CONTAINER:
                *pFileType = WBMS_FILE_TYPE_EMS_CONTAINER;
                break;
            default:
                rc = ADI_WIL_ERR_INVALID_PARAMETER;
                break;
        }
    }

    return rc;
}

adi_wil_device_t wb_wil_GetExternalDeviceId (uint8_t iDeviceId)
{
    adi_wil_device_t iDev = 0ULL;

    /* Check if it's a node ID */
    if (iDeviceId < ADI_WIL_MAX_NODES)
    {
        iDev = 1ULL << iDeviceId;
    }
    /* Handle single/all manager scenario */
    else if (iDeviceId == WBMS_MANAGER_0_DEVICE_ID)
    {
        iDev = ADI_WIL_DEV_MANAGER_0;
    }
    else if (iDeviceId == WBMS_MANAGER_1_DEVICE_ID)
    {
        iDev = ADI_WIL_DEV_MANAGER_1;
    }
    else if (iDeviceId == WB_WIL_DEV_ALL_MANAGERS)
    {
        iDev = ADI_WIL_DEV_ALL_MANAGERS;
    }
    else
    {
        /* If none of the above match, then, default to all nodes */
        iDev = ADI_WIL_DEV_ALL_NODES;
    }

    return iDev;
}

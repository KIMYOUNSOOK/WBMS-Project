/******************************************************************************
 * @file     wbms_port_config.h
 *
 * @brief
 *
 * @details
 *           This header is used by the WBMS Interface Library.
 *
 * Copyright (c) 2019 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

/**
 * @file     wbms_port_config.h
 *
 * @brief    WBMSGEN2 configurations
 *
*/

#ifndef WBMS_PORT_CONFIG_H
#define WBMS_PORT_CONFIG_H

/**
 * @brief   Opcmd port ID
 */
#define WB_OPCMD_PORT_ID                    ((uint8_t)10u)

/**
 * @brief   BMS application port ID
 */
#define WB_BMS_PORT_ID                      ((uint8_t)20u)

/**
 * @brief   Device fault port ID
 */
#define WB_DEVMON_PORT_ID                   ((uint8_t)30u)

/**
 * @brief   SCL Port ID
 */
#define WB_SCL_PORT_ID                      ((uint8_t)40u)

/*! @} */
#endif /* WBMS_PORT_CONFIG_H */


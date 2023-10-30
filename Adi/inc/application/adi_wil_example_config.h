/*******************************************************************************
 * @file adi_embedded_user_config.h
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef EMB_WIL_USER_CONFIG_H_
#define EMB_WIL_USER_CONFIG_H_

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/
/*
 * Number of nodes in the network
 * 
 */
#define ADI_NODES_IN_NETWORK                    2
#define ADI_WIL_CONNECT_MAX_RETRIES             10
#define ADI_WIL_SET_MODE_MAX_RETRIES            5
#define ADI_BMS_PACKETS_PER_NODE_PER_INTERVAL   3

#define BMS_DATA_PACKET_COUNT                   48u     /* Total number of BMS data packets in the system */
#define PMS_DATA_PACKET_COUNT                   10u     /* Total number of PMS data packets in the system */
#define EMS_DATA_PACKET_COUNT                   2u      /* Total number of EMS data packets in the system */

/*
 * UART configuration
 **/
#define BMS_PACKETS_PRINTING_ON                  0
#define PMS_PACKETS_PRINTING_ON                  0
#define EMS_PACKETS_PRINTING_ON                  0
#define USART_BAUDRATE                           921600
#define VERBOSE                                  1

/* defines to map physical SPI ports/devices to WIL port_t */
#define PORT0_SPI_DEVICE                        (0)
#define PORT0_CHIP_SELECT                       (0)
#define PORT1_SPI_DEVICE                        (1)
#define PORT1_CHIP_SELECT                       (0)
#define PORT_COUNT                               2u      /* Number of SPI ports in the system */

#ifdef DBG_GET_FILE_TEST
/* Get file buffer configuration */
/* This is the Get File API buffer size that is left for the customers design.
   100 is the current set value since this fits the configuration file perfectly */
#define GET_FILE_BUFFER_COUNT                    1000u
#endif

#endif /* EMB_WIL_USER_CONFIG_H_ */

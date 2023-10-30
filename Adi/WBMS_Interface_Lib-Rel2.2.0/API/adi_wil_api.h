/******************************************************************************
 * @file     adi_wil_api.h
 *
 * @brief    WBMS Interface Library API header.
 *
 * @details  Contains API declarations for the WBMS Interface Library.
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 *****************************************************************************/

#ifndef ADI_WIL_API__H
#define ADI_WIL_API__H

#include <stdint.h>
#include <stdbool.h>
#include "adi_wil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Function Declarations
 *****************************************************************************/
/**
 * @brief   Initialize the WIL
 *
 * @details Initializes the internal variables used by the WIL. This function
 *          only initializes the WIL global resources, any pack instances
 *          are not initialized in this function. This function should be called
 *          at application start up only once before any other WIL APIs are called.
 *          This function is a blocking API.
 *
 * @param   void            No arguments.
 *
 * @return adi_wil_err_t    Error code of the initialization.
 */
adi_wil_err_t adi_wil_Initialize (void);


/**
 * @brief   Terminates the WIL.
 *
 * @details This function is a matching call to adi_wil_Initialize(). It is called
 *          by the client to terminate the WIL. When called, the WIL closes all open
 *          ports and frees any memory allocated during WIL initialization. After calling
 *          the adi_wil_Terminate() function, the adi_wil_Initialize() function must
 *          be called before any other WIL function calls can be called again. Note that
 *          adi_wil_Disconnect function should be called for all the packs before calling
 *          adi_wil_Terminate. This function is a blocking API.
 *
 * @param   void            No arguments.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_Terminate (void);


/**
 * @brief   Connect to the network managers
 *
 * @details This function initiates a WIL pack instance using the given port objects,
 *          sensor data memory and connects to the specified battery pack managers.
 *          This function is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the connection operation ({@link adi_wil_err_t})
 *          - pData: The address of the current network manager configuration data
 *                   (adi_wil_connection_details_t).
 *                   Note that the hash returned in adi_wil_connection_details_t is
 *                   only valid if the operation was successful.
 *
 *          The user application can examine this returned configuration hash and determine
 *          whether the manager(s) are in the correct configuration (compared to a known hash).
 *
 *          API available in all system modes.
 *
 * @param   pPack            Pack handle.
 *
 * @return adi_wil_err_t Error code of the initialization.
 */
adi_wil_err_t adi_wil_Connect (adi_wil_pack_t const * const pPack);


/**
 * @brief   Disconnect from the network managers
 *
 * @details This API disconnects a WIL pack instance from a WBMS network. Once
 *          disconnected, the instance will no longer automatically reconnect.
 *          This function also frees any memory allocated during adi_wil_Connect.
 *          Once disconnected, adi_wil_Connect API will need to be called again
 *          if the application wishes to communicate with the network managers.
 *          This API is blocking. It does not trigger an API callback.
 *
 *          API available in all system modes.
 *
 * @param   pPack           Pack handle.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_Disconnect (adi_wil_pack_t const * const pPack);


/**
 * @brief   Sets the WBMS system operating mode.
 *
 * @details Sets the operating mode of the WBMS. Only the following mode
 *          transitions are allowed:
 *          - Standby to Commissioning
 *          - Commissioning to Standby
 *          - Standby to Active
 *          - Active to Standby
 *          - Standby to Monitoring
 *          - Monitoring to Standby
 *          - Standby to OTAP
 *          - OTAP to Standby
 *          - Standby to Sleep
 *          - Sleep to Standby
 *          This API is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:  The result of the operation {@link adi_wil_err_t} to indicate (either success
 *                 or fail).
 *
 *          API available in all system modes.
 *
 * @param   pPack           Pack handle.
 * @param   eMode           Desired operating system mode
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SetMode (adi_wil_pack_t const * const pPack,
                               adi_wil_mode_t eMode);


/**
 * @brief   Retrieves the current system mode of the WBMS. This function is a blocking
 *          API.
 *
 * @details Returns the current system mode. This function is a blocking API, so no API
 *          callback is generated.
 *
 *          API available in all system modes.
 *
 * @param   pPack           Pack handle.
 * @param   pMode           Pointer to where the current mode of WBMS is returned
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetMode (adi_wil_pack_t const * const pPack,
                               adi_wil_mode_t * const pMode);

/**
 * @brief   Sets the ACL for the network
 *
 * @details This function takes a pointer to an user provided ACL and configures the
 *          network managers to use the given ACL. Note that the given ACL data does
 *          not need to be kept until the callback occurs as the WIL keeps an internal
 *          copy of this list.
 *          This API is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param pPack             Pack handle.
 * @param pData             Pointer to buffer containing up to 64 8-byte MACs
 * @param iCount            Number of 8-byte MACs in the list
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SetACL (adi_wil_pack_t const * const pPack,
                              uint8_t const * const pData,
                              uint8_t iCount);

/**
 * @brief   Retrieves the current ACL for the network
 *
 * @details This function returns the current ACL on the network managers.
 *          This API is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: Address of the retrieved ACL list of type {@link adi_wil_acl_t}
 *
 *          API available in all system modes.
 *
 * @param   pPack           Pack handle.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetACL (adi_wil_pack_t const * const pPack);


/**
 * @brief   Retrieves the current configuration details of the manager device
 *          connected to the given SPI port. The information returned can be
 *          used to determine how the manager is configured.
 *
 * @details In the case that the host is connecting to a pack where the
 *          configuration is not known, the user can invoke this API to
 *          interrogate the manager on each SPI port for its current
 *          configuration, prior to an attempt to connect to the managers. This
 *          allows the user to allocate an appropriately sized sensor data buffer
 *          and connect in an appropriate manner using the adi_wil_Connect API.
 *          By reviewing the configuration data returned from each SPI port,
 *          the user can determine if the connect function should be invoked in
 *          single or dual manager mode, the correct number of sensor data buffers
 *          to allocate to receive data from the WBMS.
 *          This API is a non-blocking API.
 *
 *          Returned in the QueryDevice callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: Address of an {@link adi_wil_configuration_t} object that
 *                   holds the retrieved configuration. The data in the
 *                   structure is only valid if the operation was successful.
 *
 * @param   pPort           Pointer to port structure to query configuration on
 *
 * @return adi_wil_err_t    Operation error code.
 */
adi_wil_err_t adi_wil_QueryDevice (adi_wil_port_t * const pPort);

/**
 * @brief   Returns the connection status of the WBMS nodes.
 *
 * @details This API returns the current network status. The information is returned in the
 *          user provided adi_wil_network_status_t structure pointed to by pStatus. The
 *          status information returned is an array of flags each corresponding to a node
 *          in the ACL. The nodes in this flag array is in the same order as they are in the
 *          ACL. Each flag represents the current connection status of a node, i.e. true if
 *          connected, false if not. This function is a blocking API hence it does not generate
 *          an API callback.
 *
 *          API available in all system modes.
 *
 * @param   pPack           Pack handle.
 * @param   pStatus         Pointer to store network status
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetNetworkStatus (adi_wil_pack_t const * const pPack,
                                        adi_wil_network_status_t * const pStatus);


/**
 * @brief   Loads a file onto one or more devices.
 *
 * @details This API loads files such as BMS/PMS/Environmental monitoring containers,
 *          configuration, and firmware files onto the specified device(s).
 *          The specified files are transferred in fixed size blocks. Every time this
 *          API is invoked, the user passes a block of data (pointed to by pData) to be
 *          transferred to the device(s). Upon the sending of the data block, an API
 *          notification callback is generated by the WIL, indicating the offset of the
 *          next block of data (in relation to the start of the file being transferred)
 *          to be provided on the next invocation of this API.
 *          This API is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}. An error code
 *                   ADI_WIL_ERR_IN_PROGRESS is returned when the downloading is still
 *                   in progress, at the end of the download, either ADI_WIL_ERR_SUCCESS 
 *                   or ADI_WIL_ERR_PARTIAL_SUCCESS is returned.
 *          - pData: Address of a {@link adi_wil_loadfile_status_t} struct.
 *
 *          API available in Standby mode and OTAP mode only. Note that in OTAP mode, only
 *          ADI_WIL_FILE_TYPE_FIRMWARE file type is allowed.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 * @param   eFileType       File type being loaded. Valid file types are:
 *                          ADI_WIL_FILE_TYPE_FIRMWARE
 *                          ADI_WIL_FILE_TYPE_BMS_CONTAINER
 *                          ADI_WIL_FILE_TYPE_CONFIGURATION
 *                          ADI_WIL_FILE_TYPE_PMS_CONTAINER
 *                          ADI_WIL_FILE_TYPE_EMS_CONTAINER
 * @param   pData           Pointer to a block of data from the file to be transferred
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_LoadFile (adi_wil_pack_t const * const pPack,
                                adi_wil_device_t eDeviceId,
                                adi_wil_file_type_t eFileType,
                                uint8_t const * const pData);


/**
 * @brief   Erases a specified file that resides on one or more devices.
 *
 * @details This API erases a file on the target device(s).
 *          This function is a non-blocking API and upon completion of this command,
 *          an API notification callback is generated to return an error code which
 *          indicates whether the operation was successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 * @param   eFileType       File type being loaded. Valid file types are:
 *                          ADI_WIL_FILE_TYPE_BMS_CONTAINER
 *                          ADI_WIL_FILE_TYPE_CONFIGURATION
 *                          ADI_WIL_FILE_TYPE_BLACK_BOX_LOG
 *                          ADI_WIL_FILE_TYPE_INVENTORY_LOG
 *                          ADI_WIL_FILE_TYPE_ENV_MON
 *                          ADI_WIL_FILE_TYPE_PMS_CONTAINER
 *                          ADI_WIL_FILE_TYPE_EMS_CONTAINER
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_EraseFile (adi_wil_pack_t const * const pPack,
                                 adi_wil_device_t eDeviceId,
                                 adi_wil_file_type_t eFileType);


/**
 * @brief   Retrieves the version number of the software and silicon revision from a
 *          specific device.
 *
 * @details This API retrieves the software/silicon versions and life cycle information
 *          on the target device.
 *          This function is a non-blocking API and upon completion of this command,
 *          an API notification callback is generated to return an error code which
 *          indicates whether the operation was successful and the address of a
 *          adi_wil_dev_version_t object that holds the version information is also
 *          returned.
 *          This is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of the version structure {@link adi_wil_dev_version_t}
 *                   Note that the version data is only valid if the operation was successful.
 *
 *          API available in Standby mode and OTAP mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetDeviceVersion (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId);


/**
 * @brief   Get the CRC of the given file type
 *
 * @details This API retrieves the CRC for a file loaded on a device.
 *          This is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of the File CRC information structure {@link adi_wil_file_crc_list_t}
 *                   Note that the CRC information structure is only valid if the operation was successful.
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 * @param   eFileType       File type to retrieve CRC for
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetFileCRC (adi_wil_pack_t const * const pPack,
                                  adi_wil_device_t eDeviceId,
                                  adi_wil_file_type_t eFileType);


/**
 * @brief   Sets a GPIO to the specified level (low/high) on the specified devices.
 *
 * @details This API sets the given GPIO pin on the remote device(s) to the specified value.
 *          The GPIO pin on the target device is set to be an output pin before its value is set.
 *          Note that the GPIO pin is set to output mode only, therefore it cannot be set to
 *          a value then re-read back by using adi_wil_GetGPIO since adi_wil_GetGPIO will
 *          reconfigure the pin to input mode in order to be read.
 *          This function is a non-blocking API and upon completion of this command,
 *          an API notification callback is generated to return an error code which
 *          indicates whether the operation was successful.
 *          This is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby, Active and Monitor modes only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eGPIOId         GPIO ID of the GPIO pin to set
 * @param   eGPIOValue      GPIO pin state to set to
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SetGPIO (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_gpio_id_t eGPIOId,
                               adi_wil_gpio_value_t eGPIOValue);


/**
 * @brief   Returns the value of a GPIO pin from a specific device.
 *
 * @details This API reads the given GPIO pin on the remote device(s).
 *          The GPIO pin on the target device is set as an input pin before its value is read.
 *          This function is a non-blocking API and upon completion of this command,
 *          an API notification callback is generated to return an error code which
 *          indicates whether the operation was successful as well as the address of a
 *          adi_wil_gpio_value_t object.
 *          This is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of the retrieved GPIO value {@link adi_wil_gpio_value_t}
 *                   Note that the GPIO value is only valid if the operation was successful.
 *
 *          API available in Standby, Active and Monitor modes only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eGPIOId         GPIO ID of the GPIO pin to read
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetGPIO (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_gpio_id_t eGPIOId);


/**
 * @brief   Instructs the specified device(s) which script to execute within a
 *          script container.
 *
 * @details This API selects a target script of a sensor (specified by eSensorId)
 *          on a device or devices to execute. This is a non-blocking API and upon
 *          completion of this command, an API notification callback is generated
 *          to return an error code which indicates whether the operation was successful.
 *          Note that a success return code only means the end device has accepted the
 *          command, the actual script switching may not have yet taken place. The
 *          application needs to examine the sensor data packets to determine whether
 *          a script switch has happened.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Active mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 * @param   eSensorId       Sensor ID to select script on
 * @param   iScriptId       Unique script ID
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SelectScript (adi_wil_pack_t const * const pPack,
                                    adi_wil_device_t eDeviceId,
                                    adi_wil_sensor_id_t eSensorId,
                                    uint8_t iScriptId);


/**
 * @brief   Instructs the specified device to modify the currently selected script.
 *
 * @details This API gives modification data to a device to modify a particular script
 *          entry in the container of a sensor with eSensorId. pData points to a block
 *          of the change data the user wishes to send. This is a non-blocking API and upon
 *          completion of this command, an API notification callback is generated
 *          to return an error code which indicates whether the operation was successful.
 *          Note that a successful return code only means the end device has accepted the
 *          command, it does not mean the running of the modified script has occurred. The
 *          application needs to examine the sensor data packet to determine whether the
 *          modified script has been executed after the activation time.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Active mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eSensorId       Sensor ID running script to modify
 * @param   pData           Pointer to script change structure to be sent
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_ModifyScript (adi_wil_pack_t const * const pPack,
                                    adi_wil_device_t eDeviceId,
                                    adi_wil_sensor_id_t eSensorId,
                                    adi_wil_script_change_t const * const pData);


/**
 * @brief   Instructs the specified node(s) to enable/disable inventory state.
 *
 * @details This API instructs the node(s) to enter (bEnable = TRUE) or exit (bEnable = FALSE)
 *          inventory mode. The current time (specified in Unix time is passed in to give
 *          the nodes the inventory start/stop time). This API is non-blocking, and upon
 *          completion of this command, an API callback is generated to return an error code
 *          which indicates whether the operation was successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}

 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 * @param   bEnable         Flag to indicate whether to enter (true) or exit (false)
 *                          inventory state
 * @param   iCurrentTime    Current time in UNIX seconds
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_EnterInventoryState (adi_wil_pack_t const * const pPack,
                                           adi_wil_device_t eDeviceId,
                                           bool bEnable,
                                           uint64_t iCurrentTime);


/**
 * @brief   Retrieves specified file from a manager or a node device.
 *
 * @details This API starts a file upload from a device. On every call of this API, the WIL retrieves
 *          a fixed sized block of data from the device, and the WIL does an API callback
 *          to deliver the data to the user (the address of a adi_wil_file_t object is returned).
 *          The adi_wil_file_t structure holds information about the size of the data, the byte offset
 *          of the data from the start of the file, the number of bytes remaining to be retrieved and
 *          the data itself. The user application can use these information to write the data block
 *          to a file if desired. The user should keep calling this API until the callback return code
 *          is ADI_WIL_ERR_SUCCESS, which signals the end of the transfer.
 *          This function is a non-blocking API.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}. Error code ADI_WIL_ERR_IN_PROGRESS
 *                   is returned when the uploading is still in progress, at the end of the upload,
 *                   ADI_WIL_ERR_SUCCESS is returned
 *          - pData: The address of an adi_wil_file_t structure {@link adi_wil_file_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eFileType       File type being retrieved. Valid file types are:
 *                          ADI_WIL_FILE_TYPE_CONFIGURATION
 *                          ADI_WIL_FILE_TYPE_BLACK_BOX_LOG
 *                          ADI_WIL_FILE_TYPE_INVENTORY_LOG
 *                          ADI_WIL_FILE_TYPE_ENV_MON
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetFile (adi_wil_pack_t const * const pPack,
                               adi_wil_device_t eDeviceId,
                               adi_wil_file_type_t eFileType);


/**
 * @brief   Retrieves the current version number of the WIL software.
 *
 * @details This API is blocking, hence no API callback is generated.
 *          API available in any system mode.
 *
 * @param   pVersion        Pointer to the returned version structure.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetWILSoftwareVersion (adi_wil_version_t * const pVersion);


/**
 * @brief   Called periodically to invoke the WIL background processing for a
 *          WBMS system.
 *
 * @details This function does all the WBMS Interface Library processing for the
 *          given pack. It should be run in the main application loop. It is to
 *          be called at the rate of the SPI Tx/Rx rate.
 *          This API is blocking.
 *
 * @param   pPack           Pack handle.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_ProcessTask (adi_wil_pack_t const * const pPack);


/**
 * @brief   Stores/updates contextual data on the specified device.
 *
 * @details This function stores the given contextual data on the specified device.
 *          The contextual data is identified using the contextual data ID. If the
 *          contextual data ID ADI_WIL_CONTEXTUAL_ID_WRITE_ONCE is specified, the given
 *          data is tagged as write once only. This API is non-blocking. Upon completion,
 *          an API callback is generated to return an error code that indicates whether
 *          the operation was successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack               Pack handle.
 * @param   eDeviceId           Destination device ID, valid targets are:
 *                              ADI_WIL_DEV_NODE_x - single node
 *                              ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eContextualDataId   Specifies the ID of the contextual data to be set
 * @param   pData               Pointer to contextual data
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SetContextualData (adi_wil_pack_t const * const pPack,
                                         adi_wil_device_t eDeviceId,
                                         adi_wil_contextual_id_t eContextualDataId,
                                         adi_wil_contextual_data_t const * const pData);


/**
 * @brief   Resets the specified device(s).
 *
 * @details This API is non-blocking. Upon completion, the an API callback is
 *          generated to return an error code that indicates whether the request
 *          was successful. Note that a successful return code only indicates the
 *          acceptance of the command on the target device(s), it does not mean the
 *          target device(s) have carried out the reset.
 *
 *          API available in all system modes.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_ResetDevice (adi_wil_pack_t const * const pPack,
                                   adi_wil_device_t eDeviceId);


/**
 * @brief   Retrieves contextual data from the specified device.
 *
 * @details This function retrieves the contextual data identified by contextual data
 *          ID from the specified device. This API is non-blocking. Upon completion,
 *          an API notification callback is generated to return an error code that
 *          indicates whether the operation was successful. The address of a
 *          adi_wil_contextual_data_t object holding the retrieved contextual data is
 *          also returned in the callback if it was successful.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of the retrieved contextual data structure {@link adi_wil_contextual_data_t}
 *                   The data in this data structure is only valid if the operation was successful.
 *
 *          API available in Standby mode only.
 *
 * @param   pPack               Pack handle.
 * @param   eDeviceId           Destination device ID, valid targets are:
 *                              ADI_WIL_DEV_NODE_x - single node
 *                              ADI_WIL_DEV_MANAGER_x - single manager
 * @param   eContextualDataId   Id of contextual data to retrieve
 *
 * @return  adi_wil_err_t       Operation error code.
 */
adi_wil_err_t adi_wil_GetContextualData (adi_wil_pack_t const * const pPack,
                                         adi_wil_device_t eDeviceId,
                                         adi_wil_contextual_id_t eContextualDataId);


/**
 * @brief   Sets the state of health value on a specific device.
 *
 * @details This API is non-blocking. Upon completion, the API callback
 *          is invoked to return an error code indicating whether the operation was
 *          successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *
 * @param   iPercentage     Health percentage to set [0-100]
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_SetStateOfHealth (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId,
                                        uint8_t iPercentage);


/**
 * @brief   Gets the state of health value from a specific device.
 *
 * @details Upon completion, the API callback is invoked to return an error code
 *          indicating whether the operation was successful and the address of
 *          an uint8_t value holding the retrieved state of health value is also returned.
 *          This API is non-blocking.
 *
 *          Returned in the API callback:
 *          - rc:    The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of the retrieved state of health value (uint8_t). This
 *                   value is only valid if the operation was successful.
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetStateOfHealth (adi_wil_pack_t const * const pPack,
                                        adi_wil_device_t eDeviceId);


/**
 * @brief   Commands the WBMS to enable/disable the servicing of the fault
 *          pin on a manager device.
 *
 * @details This API is non-blocking. Upon completion, the API callback
 *          is invoked to return an error code indicating whether the enabling
 *          or disabling was successful. Note that when the manager is reboot,
 *          by default the fault servicing is disabled. This API should be called
 *          to enable fault servicing prior to entering the Monitoring mode
 *          otherwise false fault notifications will result. Once enabled,
 *          fault servicing remains enabled until this API is called again to
 *          disable.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby and Active modes only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 * @param   bEnable         Flag to enable (if set to true) or disable (if set to
 *                          false) the servicing of the external watchdog timer
 *                          on the specified manager device.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_EnableFaultServicing (adi_wil_pack_t const * const pPack,
                                            adi_wil_device_t eDeviceId,
                                            bool bEnable);


/**
 * @brief   Instructs the WBMS to cycle a given key type
 *
 * @details This API when invoked with ADI_WIL_KEY_JOIN instructs a given node
 *          to save its join certificate, so no certificate exchange needs to be done
 *          in subsequent joining of the network. This speeds up the joining process.
 *          If the ACL is modified, the nodes will need to re-establish the connection
 *          to the network via a certificate exchange process.
 *
 *          ADI_WIL_KEY_SESSION and ADI_WIL_KEY_NETWORK are not supported in
 *          this release.
 *
 *          This API is non-blocking. Upon completion, the API callback is invoked to
 *          return an error code indicating whether the operation was successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eKeyType        Key type to be rotated {@link adi_wil_key_t}
 * @param   eDeviceId       Destination device ID that would have the key rotated.
 *                          Valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_ALL_NODES - all nodes (Currently not supported)
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_RotateKey (adi_wil_pack_t const * const pPack,
                                 adi_wil_key_t eKeyType,
                                 adi_wil_device_t eDeviceId);


/**
 * @brief   Stores/updates customer identifier data for all devices on the
 *          network.
 *
 * @details This function sets a customer identifier data. When selected
 *          the provided advertisement data is used in place of the
 *          specified default advertisement identifier when commissioning
 *          the network.
 *
 *          The maximum length of the customer identifier data is 32 bytes.
 *          Supplying a value of 0 for the length parameter will clear the
 *          existing customer identifier data.
 *
 *          This API is non-blocking. Upon completion, an API callback is
 *          generated to return an error code that indicates whether the
 *          operation was successful.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in Standby mode only.
 *
 * @param   pPack               Pack handle
 * @param   eCustomerIdentifier One of the following:
 *                              ADI_WIL_CUSTOMER_IDENTIFIER_0
 *                              ADI_WIL_CUSTOMER_IDENTIFIER_1
 *                              ADI_WIL_CUSTOMER_IDENTIFIER_2
 * @param   pData               Pointer to buffer containing customer
                                identifier data
 * @param   iLength             Length of the data to set
 *
 * @return  adi_wil_err_t       Operation error code.
 */
adi_wil_err_t adi_wil_SetCustomerIdentifierData (adi_wil_pack_t const * const pPack,
                                                 adi_wil_customer_identifier_t eCustomerIdentifier,
                                                 uint8_t const * const pData,
                                                 uint8_t iLength);

/**
 * @brief   Set up the network metadata buffer to enable the network data capture
 *
 * @details If the user wishes to capture the network metadata from all the received radio
 *          packets, a set of network data buffer needs to be provided by using this function for
 *          the WIL to buffer the network data. This function should be called after a pack instance has
 *          been established after calling adi_wil_Connect. To enable the capture, in addition to
 *          providing a valid set of network data buffers, the bEnable argument needs to be set to true.
 *          Once set up and enabled, the network data is returned whenever this buffer is full via the
 *          event callback using the notification ID ADI_WIL_EVENT_DATA_READY_NETWORK_DATA.
 *          If this function is not called, i.e. the network data capture buffer is not provided
 *          or if the network data buffer is provided but the bEnable argument is set to false, no
 *          network data event notification will be generated. Note once a pack has been disconnected
 *          using the adi_wil_Disconnect API, the network data capture is disabled, and it needs to be
 *          re-enabled after calling adi_wil_Connect API.
 *
 *          Returned in the event callback:
 *          - pData:  Address of the network data buffer structure {@link adi_wil_network_data_buffer_t}
 *
 *          API available in all system modes.
 *
 * @param   pPack            Pack handle
 * @param   pDataBuffer      Network data capture buffer address
 * @param   iDataBufferCount Number of adi_wil_network_data_t entries in pDataBuffer
 * @param   bEnable          Flag to indicate whether to enable network data capture
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_EnableNetworkDataCapture (adi_wil_pack_t const * const pPack,
                                                adi_wil_network_data_t * pDataBuffer,
                                                uint16_t iDataBufferCount,
                                                bool bEnable);


/**
 * @brief   Updates the Monitoring mode parameters on the specified device(s)
 *
 * @details This API replaces the existing environmental monitoring parameter
 *          table used for EMS plausibility checks when in Monitor mode. This
 *          API should be called prior to entering Monitoring mode if desired.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of an 32-bit CRC of the updated table
 *
 *          API available in standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *                          ADI_WIL_DEV_ALL_NODES - all nodes
 *                          ADI_WIL_DEV_ALL_MANAGERS - all managers
 * @param   pData           Address of replacement parameter table
 * @param   iLength         Length of replacement parameter table in bytes
 *
 * @return  adi_wil_err_t   Operation error code.
 */

adi_wil_err_t adi_wil_UpdateMonitorParameters (adi_wil_pack_t const * const pPack,
                                               adi_wil_device_t eDeviceId,
                                               uint8_t * const pData,
                                               uint16_t iLength);


/**
 * @brief   Retrieves the CRC of the Monitoring mode parameters on the specified
 *          device
 *
 * @details This API retrieves the CRC of the table used for EMS plausibility
 *          checks when in monitor mode.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *          - pData: The address of an 32-bit CRC of the updated table
 *
 *          API available in standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eDeviceId       Destination device ID, valid targets are:
 *                          ADI_WIL_DEV_NODE_x - single node
 *                          ADI_WIL_DEV_MANAGER_x - single manager
 *
 * @return  adi_wil_err_t   Operation error code.
 */

adi_wil_err_t adi_wil_GetMonitorParametersCRC (adi_wil_pack_t const * const pPack,
                                               adi_wil_device_t eDeviceId);


/**
 * @brief   Instructs the network managers to begin assessing the network
 *          topology.
 *
 * @details This API is used for initiating a topology assessment for use in
 *          Deterministic Multi Hop (DMH).
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in active mode only.
 *
 * @param   pPack           Pack handle.
 *
 * @return  adi_wil_err_t   Operation error code.
 */

adi_wil_err_t adi_wil_AssessNetworkTopology (adi_wil_pack_t const * const pPack);


/**
 * @brief   Instructs the network managers to apply network
 *          topology.
 *
 * @details This API is used for applying a topology assessment for use in
 *          Deterministic Multi Hop (DMH).
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in standby mode only.
 *
 * @param   pPack           Pack handle.
 * @param   eTopology       Topology type to apply {@link adi_wil_topology_t}
 *
 * @return  adi_wil_err_t   Operation error code.
 */

adi_wil_err_t adi_wil_ApplyNetworkTopology (adi_wil_pack_t const * const pPack,
                                            adi_wil_topology_t eTopology);

/**
 * @brief   API to configure the cell balancing on an individual
 *          safety CPU device on the network.
 *
 * @details This API is used for applying cell balancing settings on a specified
 *          node. The cell balance setting consists of a discharge duty cycle and
 *          a duration for which the setting is to be applied for.
 *          This API is applicable only to safety CPUs.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in active mode only.
 *
 * @param   pPack                Pack handle.
 * @param   eDeviceId            Destination device ID, valid targets are:
 *                               ADI_WIL_DEV_NODE_x - single node
 * @param   pDischargeDutyCycle  Pointer to array of cell balancing
 *                               duty-cycle values to be transmitted
 * @param   iDuration            Duration (in minutes) to apply cell balancing
 *                               for. A value of 0 indicates to apply the
 *                               cell balancing indefinitely
 * @param   iUVThreshold         Undervoltage threshold (in microvolts) to apply
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_ConfigureCellBalancing (adi_wil_pack_t const * const pPack,
                                              adi_wil_device_t eDeviceId,
                                              adi_wil_ddc_t const * const pDischargeDutyCycle,
                                              uint16_t iDuration,
                                              uint32_t iUVThreshold);

/**
 * @brief   API to reteive the cell balancing status of all safety CPU devices
 *          on the network.
 *
 * @details This API is used to retrieve the cell balancing status of each
 *          connected device in the pack. Each response generates an
 *          ADI_WIL_EVENT_CELL_BALANCING_STATUS event with a pointer to an
 *          {@link adi_wil_cell_balancing_status_t} data structure. No data is
 *          returned via the API callback.
 *
 *          Returned in the API callback:
 *          - rc:   The result of the operation {@link adi_wil_err_t}
 *
 *          API available in active mode only.
 *
 * @param   pPack                Pack handle.
 *
 * @return  adi_wil_err_t   Operation error code.
 */
adi_wil_err_t adi_wil_GetCellBalancingStatus (adi_wil_pack_t const * const pPack);

#ifdef __cplusplus
}
#endif
#endif // #ifndef ADI_WIL_API__H

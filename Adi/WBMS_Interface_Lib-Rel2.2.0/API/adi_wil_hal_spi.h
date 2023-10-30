/******************************************************************************
 * @file     adi_wil_hal_spi.h
 *
 * @brief    WBMS Interface Library SPI HAL functions.
 *
 * @details  Contains API declarations for the WBMS Interface Library SPI HAL.
 *
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary to Analog Devices, Inc. and its licensors.
*******************************************************************************/

#ifndef ADI_WIL_HAL_SPI__H
#define ADI_WIL_HAL_SPI__H

#include "adi_wil_hal.h"
#include "adi_wil_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Type defines
 *******************************************************************************/

/**
 * @brief   HAL callback function type
 */
typedef void (*adi_wil_hal_spi_cb_t)(uint8_t iSPIDevice, uint8_t iChipSelect);

/******************************************************************************
 * Function Declarations
 *******************************************************************************/
/**
 * @brief   Initialize a SPI port.
  *
 * @details This function initializes the underlying SPI hardware. Perform any
 *          driver specific initialization here. The iSPIDevice specifies the SPI
 *          port which the underlying user code can associate with a physical SPI
 *          port.
 *
 * @param iSPIDevice            SPI device ID to initialize
 * @param pfCB                  Callback function to be called after completing
 *                              a SPI transaction
 *
 * @return adi_wil_hal_err_t   Error code of the initialization.
 */
adi_wil_hal_err_t adi_wil_hal_SpiInit(uint8_t iSPIDevice, adi_wil_hal_spi_cb_t pfCb);

/**
 * @brief   Perform a full duplex SPI transaction on the given port.
 *
 * @details This starts a SPI transaction on the given iApi_port. Based on the
 *          SPI port set during SPI configuration, this HAL function should start
 *          the transmission on the physical SPI port that is associated with the
 *          specified SPI Device ID and the chip select.
 *
 * @param iSPIDevice            SPI device ID to use for transmit
 * @param iChipSelect           The chip select to use for SPI transmission
 * @param pTx                   The address of the Tx buffer.
 * @param pRx                   The address of the Rx buffer.
 * @param iLength               The size of the SPI transaction in bytes.
 *
 * @return  adi_wil_hal_err_t   Error code of the transaction operation.

 */
adi_wil_hal_err_t adi_wil_hal_SpiTransmit(uint8_t iSPIDevice,
                                          uint8_t iChipSelect,
                                          uint8_t * const pTx,
                                          uint8_t * const pRx,
                                          uint16_t iLength);

/**
 * @brief   Close the specified SPI port.
 *
 * @details Close the SPI device mapped to a given SPI device ID.
 *
 iManagerId
 * @param iSPIDevice            The SPI device ID to close
 *
 * @return  adi_wil_hal_err_t   Error code of the closing operation.
 *
 */
adi_wil_hal_err_t adi_wil_hal_SpiClose(uint8_t iSPIDevice);

#ifdef __cplusplus
}
#endif
#endif // #ifndef ADI_WIL_HAL_SPI__H

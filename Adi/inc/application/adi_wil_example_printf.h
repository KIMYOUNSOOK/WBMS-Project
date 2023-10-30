/*******************************************************************************
* @brief    Print utilities
*
* @details  Set of printf-like functions which send their output to host device's UART-to_USB bridge
*
* Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#ifndef ADI_WIL_EXAMPLE_PRINTF_H
#define ADI_WIL_EXAMPLE_PRINTF_H

#include <stdio.h>
#include <stdbool.h>

#define LINE_END        "\r\n"

/*******************************************************************************
 * Functions
 *******************************************************************************/
void adi_wil_ex_printfInit(void);
int adi_wil_ex_printf(const char * format, ...) __format__(printf,1,2);
int adi_wil_ex_info(const char * format, ...) __format__(printf,1,2);
int adi_wil_ex_error(const char * format, ...) __format__(printf,1,2);
int adi_wil_ex_fatal(const char * format, ...) __format__(printf,1,2);

bool adi_debug_hal_Getch(char * const gotten);
bool adi_debug_hal_IsGetch(char * const gotten);

#endif  /*  ADI_WIL_EXAMPLE_PRINTF_H  */

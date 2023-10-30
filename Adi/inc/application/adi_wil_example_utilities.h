/******************************************************************************
 * @brief   Collection of print utility functions
 *
 * @details Collection of helper functions for printing
 *
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 * This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
 *******************************************************************************/
#ifndef ADI_WIL_EXAMPLE_UTILITIES_H
#define ADI_WIL_EXAMPLE_UTILITIES_H

#include <stdint.h>
#include "adi_wil_types.h"
#include "adi_wil_example_printf.h"
#include "adi_wil_example_functions.h"

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************/
/* #defines                                                                    */
/*******************************************************************************/
#define fatal(...) do { \
        adi_wil_ex_printf(__VA_ARGS__);\
        adi_wil_ex_printf("Fatal termination" LINE_END);\
    while(1);\
} while(0)

#define isWilError(error) \
    (ADI_WIL_ERR_SUCCESS != error)

#define fatalError(error) do { \
        adk_debug_Report(DBG_fatal_error, error);\
    while(1);\
} while(0)

#define fatalOnWilError(exp) do { \
    const adi_wil_err_t error = exp; \
    if(isWilError(error)) { \
        fatalError(error); \
    } \
} while(0)

#define returnOnWilError(exp) do { \
    const adi_wil_err_t error = exp; \
    if(isWilError(error)) { \
        adk_debug_Report(DBG_need_investigation, error); \
        return error; \
    } \
} while(0)

#define noreturnOnWilError(exp) do { \
    const adi_wil_err_t error = exp; \
    if(isWilError(error)) { \
        adk_debug_Report(DBG_need_investigation, error); \
    } \
} while(0)


/*******************************************************************************
 * Functions
 *******************************************************************************/
const char *adi_wil_ErrorToString(adi_wil_err_t err_code);
const char *adi_wil_SystemModeToString(adi_wil_mode_t sysmode);
const char *adi_wil_DeviceToString(adi_wil_device_t eDeviceId);
const char *adi_wil_VersionToString(adi_wil_version_t * const version);
void adi_wil_SprintMAC(char * str, uint8_t const mac[]);
uint8_t adi_cnt1_8bit(uint8_t val);

#ifdef __cplusplus
}
#endif
#endif /* ADI_WIL_EXAMPLE_UTILITIES_H */

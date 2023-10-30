/******************************************************************************
 * @file Version.h
 *
 * Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved. This
 * software is proprietary and confidential to Analog Devices, Inc. and its
 * licensors.
 *******************************************************************************/

#ifndef _VERSION_H
#define _VERSION_H

#define _VSTR(x) #x
#define VSTR(x)  _VSTR(x)

#include "adi_wil_example_app_version.h"

#define WBMS_WIL_EMBEDDED_EXAMPLE_VERSION \
    VSTR(MAJOR) "." VSTR(MINOR) "." VSTR(PATCH) "." VSTR(BUILD)

#define WBMS_WIL_OTAP_EXAMPLE_VERSION \
    VSTR(MAJOR) "." VSTR(MINOR) "." VSTR(PATCH) "." VSTR(BUILD)

#endif /* _VERSION_H */

/**
 * \file include/vctool/status_codes.h
 *
 * \brief Status code definitions for vctool.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_HEADER_GUARD
#define VCTOOL_STATUS_CODES_HEADER_GUARD

#include <vctool/components.h>
#include <vctool/status_codes/commandline.h>
#include <vctool/status_codes/file.h>
#include <vctool/status_codes/general.h>
#include <vctool/status_codes/readpassword.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Success status code.
 */
#define VCTOOL_STATUS_SUCCESS 0x00000000

/**
 * \brief Error code macro.
 */
#define VCTOOL_STATUS_ERROR_MACRO(component, reason) \
    ((int32_t)(0x8000000 | ((component & 0xFF) << 16) | (reason & 0xFFFF)))

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_HEADER_GUARD*/

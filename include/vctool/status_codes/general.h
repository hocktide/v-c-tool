/**
 * \file include/vctool/status_codes/general.h
 *
 * \brief General status codes for vctool.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_GENERAL_HEADER_GUARD
#define VCTOOL_STATUS_CODES_GENERAL_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Out of memory error.
 */
#define VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_GENERAL, 0x0001U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_GENERAL_HEADER_GUARD*/

/**
 * \file include/vctool/status_codes/readpassword.h
 *
 * \brief Status codes for readpassword.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_READPASSWORD_HEADER_GUARD
#define VCTOOL_STATUS_CODES_READPASSWORD_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Standard input is not a terminal.
 */
#define VCTOOL_ERROR_READPASSWORD_STDIN_NOT_A_TERMINAL \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_READPASSWORD, 0x0001U)

/**
 * \brief Error reading a password.
 */
#define VCTOOL_ERROR_READPASSWORD_READ \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_READPASSWORD, 0x0002U)

/**
 * \brief Error setting terminal attributes.
 */
#define VCTOOL_ERROR_READPASSWORD_TCSETATTR \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_READPASSWORD, 0x0003U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_READPASSWORD_HEADER_GUARD*/

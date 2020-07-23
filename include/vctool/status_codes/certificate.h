/**
 * \file include/vctool/status_codes/certificate.h
 *
 * \brief Status codes for the certificate component.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_CERTIFICATE_HEADER_GUARD
#define VCTOOL_STATUS_CODES_CERTIFICATE_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief The encrypted certificate is not at least the minimum size.
 */
#define VCTOOL_ERROR_CERTIFICATE_NOT_MINIMUM_SIZE \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_CERTIFICATE, 0x0001U)

/**
 * \brief The encrypted certificate could not be verified.
 */
#define VCTOOL_ERROR_CERTIFICATE_VERIFICATION \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_CERTIFICATE, 0x0002U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_CERTIFICATE_HEADER_GUARD*/

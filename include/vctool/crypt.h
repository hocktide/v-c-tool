/**
 * \file include/vctool/crypt.h
 *
 * \brief Cryptography utility functions.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_CRYPT_HEADER_GUARD
# define VCTOOL_CRYPT_HEADER_GUARD

#include <vccrypt/suite.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Initialize a cipher and mac instance from a suite, password, salt, and
 * number of key derivation rounds.
 *
 * \param cipher            The stream cipher instance to initialize.
 * \param mac               The mac instance to initialize.
 * \param suite             The crypto suite to use to initialize these
 *                          instances.
 * \param password          The password to use for deriving the private key.
 * \param salt              The salt to use for deriving the private key.
 * \param rounds            The number of rounds to use to derive the private
 *                          key.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int crypt_cipher_mac_init_from_password(
    vccrypt_stream_context_t* cipher, vccrypt_mac_context_t* mac,
    vccrypt_suite_options_t* suite, const vccrypt_buffer_t* password,
    const vccrypt_buffer_t* salt, unsigned int rounds);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_CRYPT_HEADER_GUARD*/

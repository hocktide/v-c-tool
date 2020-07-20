/**
 * \file include/vctool/readpassword.h
 *
 * \brief Function to read a password without echoing.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_READ_PASSWORD_HEADER_GUARD
# define VCTOOL_READ_PASSWORD_HEADER_GUARD

#include <vccrypt/buffer.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Read a password from standard input.
 *
 * \param opts              The command-line options.
 * \param passbuffer        Pointer to a vccrypt_buffer_t to be initialized with
 *                          the password / passphrase that has been read.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int readpassword(commandline_opts* opts, vccrypt_buffer_t* passbuffer);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_READ_PASSWORD_HEADER_GUARD*/

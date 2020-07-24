/**
 * \file include/vctool/certificate.h
 *
 * \brief Certificate utility functions.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_CERTIFICATE_HEADER_GUARD
# define VCTOOL_CERTIFICATE_HEADER_GUARD

#include <vccrypt/buffer.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

#define ENCRYPTED_CERT_MAGIC_SIZE 3
#define ENCRYPTED_CERT_MAGIC_STRING "ENC"

/**
 * \brief Create a keypair certificate based on the provided command-line
 * options.
 *
 * \param opts              The command-line options to use.
 * \param private_cert      Pointer to a vccrypt buffer to be initialized and
 *                          that will hold the computed certificate.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int keypair_certificate_create(
    commandline_opts* opts, vccrypt_buffer_t* private_cert);

/**
 * \brief Create a pubkey certificate based on the provided field values.
 *
 * \param opts              The command-line options to use.
 * \param public_cert       Pointer to a vccrypt buffer to be initialized and
 *                          that will hold the computed certificate.
 * \param uuid              The uuid for this pubkey cert.
 * \param encryption_pubkey The encryption public key for this cert.
 * \param signing_pubkey    The signing public key for this cert.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int pubkey_certificate_create(
    commandline_opts* opts, vccrypt_buffer_t* public_cert,
    const vccrypt_buffer_t* uuid, const vccrypt_buffer_t* encryption_pubkey,
    const vccrypt_buffer_t* signing_pubkey);

/**
 * \brief Encrypt a certificate using the given password.
 *
 * \param opts              The command-line options to use.
 * \param encrypted_cert    Pointer to the pointer to receive an allocated
 *                          vccrypt_buffer_t instance holding the encrypted
 *                          certificate on function success.
 * \param cert              The certificate to encrypt.
 * \param password          The password to use to derive the encryption key.
 * \param rounds            The number of rounds to use for deriving the
 *                          encryption key from the passphrase and salt.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int certificate_encrypt(
    commandline_opts* opts, vccrypt_buffer_t** encrypted_cert,
    const vccrypt_buffer_t* cert, const vccrypt_buffer_t* password,
    unsigned int rounds);

/**
 * \brief Decrypt a certificate using the given password.
 *
 * \param opts              The command-line options to use.
 * \param cert              Pointer to the pointer to receive an allocated
 *                          vccrypt_buffer_t instance holding the decrypted
 *                          certificate on function success.
 * \param encrypted_cert    The encrypted certificate.
 * \param password          The password to use to derive the encryption key.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int certificate_decrypt(
    commandline_opts* opts, vccrypt_buffer_t** cert,
    const vccrypt_buffer_t* encrypted_cert, const vccrypt_buffer_t* password);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_CERTIFICATE_HEADER_GUARD*/

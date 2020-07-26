/**
 * \file certificate/certificate_decrypt.c
 *
 * \brief Decrypt a certificate
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/commandline.h>
#include <vctool/crypt.h>

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
    const vccrypt_buffer_t* encrypted_cert, const vccrypt_buffer_t* password)
{
    int retval;
    uint32_t net_rounds, rounds;
    vccrypt_buffer_t mac_buffer, salt;
    vccrypt_stream_context_t cipher;
    vccrypt_mac_context_t mac;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != cert);
    MODEL_ASSERT(NULL != encrypted_cert);
    MODEL_ASSERT(NULL != password);

    /* create mac buffer. */
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            opts->suite, &mac_buffer, false);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }
            
    /* create the buffer for holding the salt. */
    /* TODO - replace with suite method. */
    retval = 
        vccrypt_buffer_init(
            &salt, opts->suite->alloc_opts,
            opts->suite->stream_cipher_opts.key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* compute the minimum size of the encrypted certificate. */
    size_t iv_size = opts->suite->stream_cipher_opts.IV_size;
    size_t min_encrypted_cert_size =
          ENCRYPTED_CERT_MAGIC_SIZE             /* "ENC" */
        + sizeof(uint32_t)                      /* number of rounds in key. */
        + salt.size                             /* the salt. */
        + iv_size                               /* the iv. */
        + opts->suite->mac_opts.mac_size;       /* the mac. */

    /* verify that the cert is at least this size. */
    if (encrypted_cert->size < min_encrypted_cert_size)
    {
        retval = VCTOOL_ERROR_CERTIFICATE_NOT_MINIMUM_SIZE;
        goto cleanup_salt;
    }

    /* get a byte pointer to the certificate buffer. */
    const uint8_t* bcert = (const uint8_t*)encrypted_cert->data;

    /* verify that the first three bytes are the magic. */
    if (
        crypto_memcmp(
            bcert, ENCRYPTED_CERT_MAGIC_STRING, ENCRYPTED_CERT_MAGIC_SIZE))
    {
        retval = VCTOOL_ERROR_CERTIFICATE_VERIFICATION;
        goto cleanup_salt;
    }
    bcert += ENCRYPTED_CERT_MAGIC_SIZE;

    /* get the number of rounds. */
    memcpy(&net_rounds, bcert, sizeof(net_rounds));
    rounds = ntohl(net_rounds);
    bcert += sizeof(net_rounds);

    /* copy the salt to the salt buffer. */
    memcpy(salt.data, bcert, salt.size);
    bcert += salt.size;

    /* create the mac and cipher instances. */
    retval =
        crypt_cipher_mac_init_from_password(
            &cipher, &mac, opts->suite, password, &salt, rounds);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto cleanup_salt;
    }

    /* allocate space for the decrypted certificate. */
    *cert = (vccrypt_buffer_t*)malloc(sizeof(vccrypt_buffer_t));
    if (NULL == *cert)
    {
        goto cleanup_cipher_mac;
    }

    /* create the decrypted cert. */
    size_t cert_size = encrypted_cert->size - min_encrypted_cert_size;

    retval =
        vccrypt_buffer_init(
            *cert, opts->suite->alloc_opts,
            encrypted_cert->size - min_encrypted_cert_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto free_cert;
    }

    /* mac the whole enchilada before trying to decrypt. */
    retval =
        vccrypt_mac_digest(
            &mac, encrypted_cert->data, encrypted_cert->size - mac_buffer.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_cert;
    }

    /* write the mac. */
    retval = vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_cert;
    }

    /* compare the mac with the saved value. */
    const uint8_t* certmac = (const uint8_t*)encrypted_cert->data;
    certmac += encrypted_cert->size - mac_buffer.size;
    if (crypto_memcmp(certmac, mac_buffer.data, mac_buffer.size))
    {
        retval = VCTOOL_ERROR_CERTIFICATE_VERIFICATION;
        goto cleanup_cert;
    }

    /* start decryption. */
    size_t input_offset = 0;
    retval =
        vccrypt_stream_start_decryption(
            &cipher, bcert, &input_offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_cert;
    }

    /* decrypt the certificate. */
    size_t output_offset = 0;
    retval =
        vccrypt_stream_decrypt(
            &cipher, bcert + input_offset, cert_size,
            (*cert)->data, &output_offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_cert;
    }

    /* success. We want to jump past the cert cleanup, as the cert's ownership
     * transfers to the caller on success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto cleanup_cipher_mac;

cleanup_cert:
    dispose((disposable_t*)*cert);

free_cert:
    free(*cert);
    *cert = NULL;

cleanup_cipher_mac:
    dispose((disposable_t*)&cipher);
    dispose((disposable_t*)&mac);

cleanup_salt:
    dispose((disposable_t*)&salt);

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

done:
    return retval;
}

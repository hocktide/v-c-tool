/**
 * \file certificate/certificate_encrypt.c
 *
 * \brief Encrypt a certificate
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/certificate.h>
#include <vctool/crypt.h>

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
    unsigned int rounds)
{
    int retval;
    vccrypt_stream_context_t cipher;
    vccrypt_mac_context_t mac;
    vccrypt_prng_context_t prng;
    vccrypt_buffer_t salt, iv, mac_buffer;
    uint8_t* benc;
    size_t offset = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != encrypted_cert);
    MODEL_ASSERT(NULL != cert);
    MODEL_ASSERT(NULL != password);
    MODEL_ASSERT(rounds > 0);

    /* create a buffer for holding the salt. */
    /* TODO - replace with suite method. */
    retval =
        vccrypt_buffer_init(
            &salt, opts->suite->alloc_opts,
            opts->suite->stream_cipher_opts.key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create a buffer for holding the iv. */
    /* TODO - replace with suite method. */
    retval =
        vccrypt_buffer_init(
            &iv, opts->suite->alloc_opts,
            opts->suite->stream_cipher_opts.IV_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_salt;
    }

    /* create a buffer for holding the mac. */
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            opts->suite, &mac_buffer, false);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_iv;
    }

    /* create prng instance for getting salt and iv. */
    retval = vccrypt_suite_prng_init(opts->suite, &prng);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac_buffer;
    }

    /* read random bytes into salt buffer. */
    retval = vccrypt_prng_read(&prng, &salt, salt.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* read random bytes into the iv buffer. */
    retval = vccrypt_prng_read(&prng, &iv, iv.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* create the mac and cipher instances. */
    retval =
        crypt_cipher_mac_init_from_password(
            &cipher, &mac, opts->suite, password, &salt, rounds);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* compute the size of the encrypted certificate file. */
    size_t encrypted_cert_size =
          ENCRYPTED_CERT_MAGIC_SIZE             /* "ENC" */
        + sizeof(uint32_t)                      /* number of rounds in key. */
        + salt.size                             /* the salt. */
        + iv.size                               /* the iv. */
        + cert->size                            /* the encrypted certificate. */
        + opts->suite->mac_opts.mac_size;       /* the mac. */

    /* allocate space for the encrypted certificate. */
    *encrypted_cert = (vccrypt_buffer_t*)malloc(sizeof(vccrypt_buffer_t));
    if (NULL == *encrypted_cert)
    {
        goto cleanup_cipher_mac;
    }

    /* create the encrypted cert. */
    retval =
        vccrypt_buffer_init(
            *encrypted_cert, opts->suite->alloc_opts, encrypted_cert_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto free_encrypted_cert;
    }

    /* set the byte buffer to the start of the encrypted cert. */
    benc = (uint8_t*)(*encrypted_cert)->data;

    /* copy the magic to the encrypted certificate. */
    memcpy(benc, ENCRYPTED_CERT_MAGIC_STRING, ENCRYPTED_CERT_MAGIC_SIZE);
    benc += ENCRYPTED_CERT_MAGIC_SIZE;

    /* MAC the magic. */
    retval =
        vccrypt_mac_digest(
            &mac, (const uint8_t*)ENCRYPTED_CERT_MAGIC_STRING,
            ENCRYPTED_CERT_MAGIC_SIZE);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* copy the number of rounds to the encrypted cert. */
    uint32_t net_rounds = htonl(rounds);
    memcpy(benc, &net_rounds, sizeof(net_rounds));
    benc += sizeof(net_rounds);

    /* MAC the rounds. */
    retval =
        vccrypt_mac_digest(
            &mac, (const uint8_t*)&net_rounds, sizeof(net_rounds));
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* copy the salt to the encrypted cert. */
    memcpy(benc, salt.data, salt.size);
    benc += salt.size;

    /* MAC the salt. */
    retval = vccrypt_mac_digest(&mac, salt.data, salt.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* start encryption. */
    retval =
        vccrypt_stream_start_encryption(
            &cipher, iv.data, iv.size, benc, &offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* encrypt the private cert, writing to the buffer. */
    retval =
        vccrypt_stream_encrypt(
            &cipher, cert->data, cert->size, benc, &offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* mac the encrypted data. */
    retval = vccrypt_mac_digest(&mac, benc, offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* increment benc to the mac location. */
    benc += offset;

    /* write the mac. */
    retval = vccrypt_mac_finalize(&mac, &mac_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encrypted_cert;
    }

    /* copy the mac to the encrypted cert. */
    memcpy(benc, mac_buffer.data, mac_buffer.size);

    /* success.  We want to jump past encrypted cert cleanup, as the encrypted
     * cert's ownership transfers to the caller on success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto cleanup_cipher_mac;

cleanup_encrypted_cert:
    dispose((disposable_t*)*encrypted_cert);

free_encrypted_cert:
    free(*encrypted_cert);

cleanup_cipher_mac:
    dispose((disposable_t*)&cipher);
    dispose((disposable_t*)&mac);

cleanup_prng:
    dispose((disposable_t*)&prng);

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_iv:
    dispose((disposable_t*)&iv);

cleanup_salt:
    dispose((disposable_t*)&salt);

done:
    return retval;
}

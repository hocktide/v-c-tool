/**
 * \file crypt/crypt_cipher_mac_init_from_password.c
 *
 * \brief Create a stream cipher and mac from a password.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/crypt.h>

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
    const vccrypt_buffer_t* salt, unsigned int rounds)
{
    int retval;
    vccrypt_buffer_t derived_key;
    vccrypt_key_derivation_context_t key_derivation;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != cipher);
    MODEL_ASSERT(NULL != mac);
    MODEL_ASSERT(NULL != suite);
    MODEL_ASSERT(NULL != password);
    MODEL_ASSERT(NULL != salt);

    /* create a buffer for holding the derived key. */
    /* TODO - replace with suite method. */
    retval =
        vccrypt_buffer_init(
            &derived_key, suite->alloc_opts,
            suite->stream_cipher_opts.key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create key derivation instance. */
    retval = vccrypt_suite_key_derivation_init(&key_derivation, suite);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_derived_key;
    }

    /* derive the key. */
    retval =
        vccrypt_key_derivation_derive_key(
            &derived_key, &key_derivation, password, salt, rounds);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* create the mac instance. */
    retval = vccrypt_suite_mac_init(suite, mac, &derived_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* create the stream cipher instance. */
    retval = vccrypt_suite_stream_init(suite, cipher, &derived_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* success. */
    retval = VCCRYPT_STATUS_SUCCESS;

    /* don't clean up cipher or mac, as the caller owns them on succes. */
    goto cleanup_key_derivation;

cleanup_mac:
    dispose((disposable_t*)mac);

cleanup_key_derivation:
    dispose((disposable_t*)&key_derivation);

cleanup_derived_key:
    dispose((disposable_t*)&derived_key);

done:
    return retval;
}

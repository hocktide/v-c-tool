/**
 * \file certificate/keypair_certificate_create.c
 *
 * \brief Create a keypair certificate.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vccert/certificate_types.h>
#include <vccert/fields.h>
#include <vctool/certificate.h>

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
    commandline_opts* opts, vccrypt_buffer_t* private_cert)
{
    int retval;
    vccrypt_buffer_t uuidbuffer, agreement_privkey, agreement_pubkey,
                     signature_privkey, signature_pubkey;
    vccrypt_prng_context_t prng;
    vccrypt_key_agreement_context_t agreement;
    vccrypt_digital_signature_context_t signature;
    vccert_builder_context_t builder;
    const uint8_t* cert;
    size_t cert_size;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != private_cert);

    /* Open prng. */
    retval = vccrypt_suite_prng_init(opts->suite, &prng);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* allocate buffer for random uuid. */
    retval = vccrypt_suite_buffer_init_for_uuid(opts->suite, &uuidbuffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* Generate random uuid. */
    retval = vccrypt_prng_read(&prng, &uuidbuffer, uuidbuffer.size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuidbuffer;
    }

    /* Create the key agreement instance. */
    retval =
        vccrypt_suite_cipher_key_agreement_init(
            opts->suite, &agreement);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuidbuffer;
    }

    /* Create private key buffer for key agreement. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_private_key(
            opts->suite, &agreement_privkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement;
    }

    /* Create public key buffer for key agreement. */
    retval =
        vccrypt_suite_buffer_init_for_cipher_key_agreement_public_key(
            opts->suite, &agreement_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement_privkey;
    }

    /* Generate key agreement keypair. */
    retval =
        vccrypt_key_agreement_keypair_create(
            &agreement, &agreement_privkey, &agreement_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement_pubkey;
    }

    /* Create signing algorithm instance. */
    retval =
        vccrypt_suite_digital_signature_init(
            opts->suite, &signature);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_agreement_pubkey;
    }

    /* Create private key buffer for signing. */
    retval =
        vccrypt_suite_buffer_init_for_signature_private_key(
            opts->suite, &signature_privkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_signature;
    }

    /* Create public key buffer for signing. */
    retval =
        vccrypt_suite_buffer_init_for_signature_public_key(
            opts->suite, &signature_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_signature_privkey;
    }

    /* Generate signing keypair. */
    retval =
        vccrypt_digital_signature_keypair_create(
            &signature, &signature_privkey, &signature_pubkey);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_signature_pubkey;
    }
    
    /* create a builder instance. */
    retval = vccert_builder_init(opts->builder_opts, &builder, 2048);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_signature_pubkey;
    }
    
    /* Add the certificate version. */
    retval =
        vccert_builder_add_short_uint32(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_VERSION, 0x00010000UL);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* Add the certificate type. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_TYPE,
            vccert_certificate_type_uuid_private_entity,
            sizeof(vccert_certificate_type_uuid_private_entity));
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* Add the crypto suite. */
    /* TODO - this should be pulled from the suite options. */
    retval =
        vccert_builder_add_short_uint16(
            &builder, VCCERT_FIELD_TYPE_CERTIFICATE_CRYPTO_SUITE,
            (uint16_t)VCCRYPT_SUITE_VELO_V1);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* Add the entity id. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_ARTIFACT_ID,
            uuidbuffer.data, uuidbuffer.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the public encryption key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PUBLIC_ENCRYPTION_KEY,
            agreement_pubkey.data, agreement_pubkey.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the private encryption key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PRIVATE_ENCRYPTION_KEY,
            agreement_privkey.data, agreement_privkey.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the public signing key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PUBLIC_SIGNING_KEY,
            signature_pubkey.data, signature_pubkey.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the private signing key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PRIVATE_SIGNING_KEY,
            signature_privkey.data, signature_privkey.size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* emit the certificate. */
    cert = vccert_builder_emit(&builder, &cert_size);

    /* initialize the private cert buffer. */
    retval =
        vccrypt_buffer_init(private_cert, opts->suite->alloc_opts, cert_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* copy the builder data into this private cert. */
    memcpy(private_cert->data, cert, private_cert->size);

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;

    /* fall-through */

cleanup_builder:
    dispose((disposable_t*)&builder);

cleanup_signature_pubkey:
    dispose((disposable_t*)&signature_pubkey);

cleanup_signature_privkey:
    dispose((disposable_t*)&signature_privkey);

cleanup_signature:
    dispose((disposable_t*)&signature);

cleanup_agreement_pubkey:
    dispose((disposable_t*)&agreement_pubkey);

cleanup_agreement_privkey:
    dispose((disposable_t*)&agreement_privkey);

cleanup_agreement:
    dispose((disposable_t*)&agreement);

cleanup_uuidbuffer:
    dispose((disposable_t*)&uuidbuffer);

cleanup_prng:
    dispose((disposable_t*)&prng);

done:
    return retval;
}

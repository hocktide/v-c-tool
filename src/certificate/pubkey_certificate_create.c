/**
 * \file certificate/pubkey_certificate_create.c
 *
 * \brief Create a pubkey certificate.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vccert/certificate_types.h>
#include <vccert/fields.h>
#include <vctool/certificate.h>
#include <vctool/commandline.h>

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
    const vccrypt_buffer_t* signing_pubkey)
{
    int retval;
    vccert_builder_context_t builder;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != public_cert);
    MODEL_ASSERT(NULL != uuid);
    MODEL_ASSERT(NULL != encryption_pubkey);
    MODEL_ASSERT(NULL != signing_pubkey);

    /* create a builder instance. */
    retval = vccert_builder_init(opts->builder_opts, &builder, 2048);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto done;
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
            vccert_certificate_type_uuid_public_entity,
            sizeof(vccert_certificate_type_uuid_public_entity));
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
            uuid->data, uuid->size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the public encryption key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PUBLIC_ENCRYPTION_KEY,
            encryption_pubkey->data, encryption_pubkey->size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* add the public signing key. */
    retval =
        vccert_builder_add_short_buffer(
            &builder, VCCERT_FIELD_TYPE_PUBLIC_SIGNING_KEY,
            signing_pubkey->data, signing_pubkey->size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* emit the certificate. */
    const uint8_t* cert = NULL;
    size_t cert_size = 0U;
    cert = vccert_builder_emit(&builder, &cert_size);

    /* initialize the public cert buffer. */
    retval =
        vccrypt_buffer_init(public_cert, opts->suite->alloc_opts, cert_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_builder;
    }

    /* copy the builder data into this private cert. */
    memcpy(public_cert->data, cert, public_cert->size);

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;

    /* fall-through */

cleanup_builder:
    dispose((disposable_t*)&builder);

done:
    return retval;
}

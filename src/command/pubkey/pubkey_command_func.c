/**
 * \file command/pubkey/pubkey_command_func.c
 *
 * \brief Entry point for the pubkey command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <vccert/fields.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/commandline.h>
#include <vctool/command/pubkey.h>
#include <vctool/command/root.h>
#include <vctool/readpassword.h>
#include <vpr/parameters.h>

/* forward decls. */
static int pubkey_extract_public_fields_from_private_cert(
    commandline_opts* opts, vccrypt_buffer_t* uuid,
    vccrypt_buffer_t* encryption_pubkey, vccrypt_buffer_t* signing_pubkey,
    const vccrypt_buffer_t* cert);
static bool dummy_txn_resolver(
    void*, void*, const uint8_t*, const uint8_t*, vccrypt_buffer_t*, bool*);
static int32_t dummy_artifact_state_resolver(
    void*, void*, const uint8_t*, vccrypt_buffer_t*);
static int dummy_contract_resolver(
    void*, void*, const uint8_t*, const uint8_t*, vccert_contract_closure_t*);
static bool dummy_key_resolver(
    void*, void*, uint64_t, const uint8_t*, vccrypt_buffer_t*,
    vccrypt_buffer_t*);

/**
 * \brief Execute the pubkey command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int pubkey_command_func(commandline_opts* opts)
{
    int retval, fd, out_fd;
    char* output_filename;
    const char* key_filename;
    vccrypt_buffer_t cert, password_buffer, uuid, encryption_pubkey,
                     signing_pubkey, pubcert;
    vccrypt_buffer_t* decrypted_cert = NULL;
    vccrypt_buffer_t* work_cert = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* get pubkey and root command. */
    pubkey_command* pubkey = (pubkey_command*)opts->cmd;
    MODEL_ASSERT(NULL != pubkey);
    root_command* root = (root_command*)pubkey->hdr.next;
    MODEL_ASSERT(NULL != root);

    /* get the key filename. */
    if (NULL != root->key_filename)
    {
        key_filename = root->key_filename;
    }
    else
    {
        retval = VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT;
        fprintf(stderr, "Expecting a key filename (-k keypair.cert).\n");
        goto done;
    }

    /* get the output filename. */
    if (NULL != root->output_filename)
    {
        output_filename = strdup(root->output_filename);
    }
    else
    {
        size_t output_filename_length =
            strlen(key_filename)
          + 4 /* .pub */
          + 1;/* asciiz */

        output_filename = (char*)malloc(output_filename_length);
        memset(output_filename, 0, output_filename_length);
        snprintf(
            output_filename, output_filename_length, "%s.pub", key_filename);
    }

    /* make sure we don't clobber an existing file. */
    file_stat_st fst;
    retval = file_stat(opts->file, output_filename, &fst);
    if (VCTOOL_ERROR_FILE_NO_ENTRY != retval)
    {
        fprintf(
            stderr, "Won't clobber existing file %s.  Stopping.\n",
            output_filename);
        goto free_output_filename;
    }

    /* make sure the key file exists. */
    retval = file_stat(opts->file, key_filename, &fst);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Missing key file %s.\n", key_filename);
        goto free_output_filename;
    }

    /* make sure the permission bits are set appropriately. */
    mode_t bad_bits = S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO;
    if (fst.fst_mode & bad_bits)
    {
        fprintf(
            stderr, "Only user permissions allowed for %s.\n", key_filename);
        goto free_output_filename;
    }
    else if (! (fst.fst_mode & S_IRUSR))
    {
        fprintf(stderr, "Can't read %s.\n", key_filename);
        goto free_output_filename;
    }

    /* create the certificate buffer. */
    size_t file_size = fst.fst_size;
    retval = vccrypt_buffer_init(&cert, opts->suite->alloc_opts, file_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto free_output_filename;
    }

    /* open file. */
    retval =
        file_open(
            opts->file, &fd, key_filename, O_RDONLY, 0);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error opening file %s for read.\n", key_filename);
        goto cleanup_cert;
    }

    /* read contents into certificate buffer. */
    size_t read_bytes;
    retval = file_read(opts->file, fd, cert.data, cert.size, &read_bytes);
    if (VCTOOL_STATUS_SUCCESS != retval || read_bytes != cert.size)
    {
        fprintf(stderr, "Error reading from %s.\n", key_filename);
        goto cleanup_file;
    }

    /* Does it have encryption magic? */
    if (cert.size > ENCRYPTED_CERT_MAGIC_SIZE
     && !crypto_memcmp(
            cert.data, ENCRYPTED_CERT_MAGIC_STRING, ENCRYPTED_CERT_MAGIC_SIZE))
    {
        /* Yes: read password and decrypt. */
        printf("Enter passphrase: ");
        fflush(stdout);
        retval = readpassword(opts, &password_buffer);
        if (VCTOOL_STATUS_SUCCESS != retval)
        {
            printf("Failure.\n");
            goto cleanup_file;
        }
        printf("\n");

        retval =
            certificate_decrypt(opts, &decrypted_cert, &cert, &password_buffer);
        if (VCTOOL_STATUS_SUCCESS != retval)
        {
            fprintf(stderr, "Error decrypting %s.\n", key_filename);
            dispose((disposable_t*)&password_buffer);
            goto cleanup_file;
        }

        dispose((disposable_t*)&password_buffer);
        work_cert = decrypted_cert;
    }
    else
    {
        work_cert = &cert;
    }

    /* extract uuid, public encryption key, and public signing key from cert. */
    retval =
        pubkey_extract_public_fields_from_private_cert(
            opts, &uuid, &encryption_pubkey, &signing_pubkey, work_cert);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(
            stderr, "Error extracting public fields from %s.\n", key_filename);
        goto cleanup_file;
    }

    /* create method for creating pubkey cert with these three items. */
    retval =
        pubkey_certificate_create(
            opts, &pubcert, &uuid, &encryption_pubkey, &signing_pubkey);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating public cert.\n");
        goto cleanup_cert_fields;
    }

    /* open output file. */
    retval =
        file_open(
            opts->file, &out_fd, output_filename, O_CREAT | O_EXCL | O_WRONLY,
            S_IRUSR | S_IWUSR);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error opening output file %s.\n", output_filename);
        goto cleanup_pubcert;
    }

    /* write this cert to the output file. */
    size_t wrote_size;
    retval =
        file_write(
            opts->file, out_fd, pubcert.data, pubcert.size, &wrote_size);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error writing to output file.\n");
        goto cleanup_outfile;
    }
    else if (wrote_size != pubcert.size)
    {
        fprintf(stderr, "Error: file truncated.\n");
        goto cleanup_outfile;
    }

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;
    /* fall-through. */

cleanup_outfile:
    file_close(opts->file, out_fd);

cleanup_pubcert:
    dispose((disposable_t*)&pubcert);

cleanup_cert_fields:
    dispose((disposable_t*)&uuid);
    dispose((disposable_t*)&encryption_pubkey);
    dispose((disposable_t*)&signing_pubkey);

cleanup_file:
    file_close(opts->file, fd);

cleanup_cert:
    dispose((disposable_t*)&cert);
    if (NULL != decrypted_cert)
    {
        dispose((disposable_t*)decrypted_cert);
        free(decrypted_cert);
    }

free_output_filename:
    free(output_filename);

done:
    return retval;
}

/**
 * \brief Extract the public keys from a private keypair certificate.
 *
 * \param opts                  The command-line options to use.
 * \param uuid                  Buffer to be initialized with the uuid. Caller
 *                              owns this buffer on success and must dispose it.
 * \param encryption_pubkey     Buffer to be initialized with the encryption
 *                              public key. Caller owns this buffer on success
 *                              and must dispose it.
 * \param signing_pubkey        Buffer to be initialized with the signing public
 *                              key. Caller owns this buffer on success and must
 *                              dispose it.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int pubkey_extract_public_fields_from_private_cert(
    commandline_opts* opts, vccrypt_buffer_t* uuid,
    vccrypt_buffer_t* encryption_pubkey, vccrypt_buffer_t* signing_pubkey,
    const vccrypt_buffer_t* cert)
{
    int retval;
    vccert_parser_options_t parser_options;
    vccert_parser_context_t parser;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != uuid);
    MODEL_ASSERT(NULL != encryption_pubkey);
    MODEL_ASSERT(NULL != signing_pubkey);
    MODEL_ASSERT(NULL != cert);

    /* create simple parser options. */
    retval =
        vccert_parser_options_init(
            &parser_options, opts->suite->alloc_opts, opts->suite,
            &dummy_txn_resolver, &dummy_artifact_state_resolver,
            &dummy_contract_resolver, &dummy_key_resolver, NULL);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* create parser for cert. */
    retval =
        vccert_parser_init(
            &parser_options, &parser, cert->data, cert->size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser_options;
    }

    /* get the entity id. */
    const uint8_t* entity_id_value = NULL;
    size_t entity_id_size = 0U;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_ARTIFACT_ID,
            &entity_id_value, &entity_id_size);
    if (VCCERT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* verify the entity id. */
    size_t expected_uuid_size = 16;
    if (entity_id_size != expected_uuid_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_parser;
    }

    /* create a uuid buffer. */
    retval =
        vccrypt_buffer_init(uuid, opts->suite->alloc_opts, expected_uuid_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_parser;
    }

    /* copy uuid value to buffer. */
    memcpy(uuid->data, entity_id_value, expected_uuid_size);

    /* get the public encryption key. */
    const uint8_t* public_encryption_key_value = NULL;
    size_t public_encryption_key_size = 0U;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PUBLIC_ENCRYPTION_KEY,
            &public_encryption_key_value, &public_encryption_key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuid;
    }

    /* verify the public encryption key size. */
    size_t expected_pubkey_size = opts->suite->key_cipher_opts.public_key_size;
    if (public_encryption_key_size != expected_pubkey_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_uuid;
    }

    /* create a public encryption key buffer. */
    retval =
        vccrypt_buffer_init(
            encryption_pubkey, opts->suite->alloc_opts, expected_pubkey_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_uuid;
    }

    /* copy public key value to buffer. */
    memcpy(
        encryption_pubkey->data, public_encryption_key_value,
        expected_pubkey_size);

    /* get the public signing key. */
    const uint8_t* public_signing_key_value = NULL;
    size_t public_signing_key_size = 0U;
    retval =
        vccert_parser_find_short(
            &parser, VCCERT_FIELD_TYPE_PUBLIC_SIGNING_KEY,
            &public_signing_key_value, &public_signing_key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encryption_pubkey;
    }

    /* verify the public signing key size. */
    size_t expected_signkey_size = opts->suite->sign_opts.public_key_size;
    if (public_signing_key_size != expected_signkey_size)
    {
        retval = VCCERT_ERROR_PARSER_FIELD_INVALID_FIELD_SIZE;
        goto cleanup_encryption_pubkey;
    }

    /* create a public signing key buffer. */
    retval =
        vccrypt_buffer_init(
            signing_pubkey, opts->suite->alloc_opts, expected_signkey_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_encryption_pubkey;
    }

    /* copy public signing key value to buffer. */
    memcpy(
        signing_pubkey->data, public_signing_key_value, expected_signkey_size);

    /* success. */
    retval = VCCRYPT_STATUS_SUCCESS;
    /* on success, caller owns the three buffers. Jump past their cleanup. */
    goto cleanup_parser;

cleanup_encryption_pubkey:
    dispose((disposable_t*)encryption_pubkey);

cleanup_uuid:
    dispose((disposable_t*)uuid);

cleanup_parser:
    dispose((disposable_t*)&parser);

cleanup_parser_options:
    dispose((disposable_t*)&parser_options);

done:
    return retval;
}

/**
 * \brief Dummy transaction resolver for parser options.
 */
static bool dummy_txn_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    const uint8_t* UNUSED(d), vccrypt_buffer_t* UNUSED(e), bool* UNUSED(f))
{
    return false;
}

/**
 * \brief Dummy artifact state resolver for parser options.
 */
static int32_t dummy_artifact_state_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    vccrypt_buffer_t* UNUSED(d))
{
    return -1;
}

/**
 * \brief Dummy contract resolver for parser options.
 */
static int dummy_contract_resolver(
    void* UNUSED(a), void* UNUSED(b), const uint8_t* UNUSED(c),
    const uint8_t* UNUSED(d), vccert_contract_closure_t* UNUSED(e))
{
    return -1;
}

/**
 * \brief Dummy key resolver for parser options.
 */
static bool dummy_key_resolver(
    void* UNUSED(a), void* UNUSED(b), uint64_t UNUSED(c),
    const uint8_t* UNUSED(d), vccrypt_buffer_t* UNUSED(e),
    vccrypt_buffer_t* UNUSED(f))
{
    return false;
}

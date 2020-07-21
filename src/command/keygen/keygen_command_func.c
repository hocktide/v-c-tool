/**
 * \file command/keygen/keygen_command_func.c
 *
 * \brief Entry point for the keygen command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <arpa/inet.h>
#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <vccert/certificate_types.h>
#include <vccert/fields.h>
#include <vccrypt/compare.h>
#include <vccrypt/suite.h>
#include <vctool/command/keygen.h>
#include <vctool/command/root.h>
#include <vctool/file.h>
#include <vctool/readpassword.h>

/* forward decls. */
static int keygen_create_keypair_cert(
    commandline_opts* opts, vccrypt_buffer_t* private_cert);
static int keygen_encrypt_certificate(
    commandline_opts* opts, vccrypt_buffer_t** encrypted_cert, const
    vccrypt_buffer_t* private_cert, const vccrypt_buffer_t* password);

#define ENCRYPTED_CERT_MAGIC_SIZE 3
#define ENCRYPTED_CERT_MAGIC_STRING "ENC"

/**
 * \brief Execute the keygen command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int keygen_command_func(commandline_opts* opts)
{
    int retval, fd;
    const char* output_filename;
    vccrypt_buffer_t password_buffer;
    vccrypt_buffer_t verify_buffer;
    vccrypt_buffer_t private_cert;
    vccrypt_buffer_t* encrypted_cert = NULL;
    vccrypt_buffer_t* write_cert = NULL;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* get keygen and root command. */
    keygen_command* keygen = (keygen_command*)opts->cmd;
    MODEL_ASSERT(NULL != keygen);
    root_command* root = (root_command*)keygen->hdr.next;
    MODEL_ASSERT(NULL != root);

    /* get the output filename. */
    if (NULL != root->output_filename)
    {
        output_filename = root->output_filename;
    }
    else
    {
        output_filename = "keypair.cert";
    }

    /* make sure we don't clobber an existing file. */
    file_stat_st fst;
    retval = file_stat(opts->file, output_filename, &fst);
    if (VCTOOL_ERROR_FILE_NO_ENTRY != retval)
    {
        fprintf(stderr, "Won't clobber existing file.  Stopping.\n");
        goto done;
    }

    /* get a passphrase for this file. */
    printf("Enter passphrase : ");
    fflush(stdout);
    retval = readpassword(opts, &password_buffer);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        printf("Failure.\n");
        goto done;
    }
    else
    {
        printf("\n");
    }

    /* read verification passphrase. */
    if (password_buffer.size > 0)
    {
        printf("Verify passphrase: ");
        fflush(stdout);
        retval = readpassword(opts, &verify_buffer);
        if (VCCRYPT_STATUS_SUCCESS != retval)
        {
            printf("Failure.\n");
            goto cleanup_password_buffer;
        }
        else
        {
            printf("\n");
        }

        /* verify that the two match. */
        if ( password_buffer.size != verify_buffer.size
          || crypto_memcmp(
                password_buffer.data, verify_buffer.data, password_buffer.size))
        {
            fprintf(stderr, "Passphrases do not match.\n");
            dispose((disposable_t*)&verify_buffer);
            goto cleanup_password_buffer;
        }

        dispose((disposable_t*)&verify_buffer);
    }

    /* generate a private certificate with a generated key. */
    retval = keygen_create_keypair_cert(opts, &private_cert);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error generating key.\n");
        goto cleanup_password_buffer;
    }

    /* should we encrypt this certificate? */
    if (password_buffer.size > 0)
    {
        /* encrypt the certificate. */
        retval =
            keygen_encrypt_certificate(
                opts, &encrypted_cert, &private_cert, &password_buffer);
        if (VCTOOL_STATUS_SUCCESS != retval)
        {
            goto cleanup_private_cert;
        }
    }

    /* open a file readable / writable by user, and no one else. */
    retval =
        file_open(
            opts->file, &fd, output_filename, O_CREAT | O_EXCL | O_WRONLY, 
            S_IRUSR | S_IWUSR);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error opening output file.\n");
        goto cleanup_encrypted_cert;
    }

    /* determine which certificate to write. */
    if (NULL != encrypted_cert)
    {
        write_cert = encrypted_cert;
    }
    else
    {
        write_cert = &private_cert;
    }

    /* write our certificate to the file. */
    size_t wrote_size;
    retval =
        file_write(
            opts->file, fd, write_cert->data, write_cert->size, &wrote_size);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error writing output file.\n");
        goto cleanup_file;
    }
    else if (wrote_size != write_cert->size)
    {
        fprintf(stderr, "Error: file truncated.\n");
        goto cleanup_file;
    }

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;

cleanup_file:
    file_close(opts->file, fd);

cleanup_encrypted_cert:
    if (NULL != encrypted_cert)
    {
        dispose((disposable_t*)encrypted_cert);
        free(encrypted_cert);
    }

cleanup_private_cert:
    dispose((disposable_t*)&private_cert);

cleanup_password_buffer:
    dispose((disposable_t*)&password_buffer);

done:
    return retval;
}

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
static int keygen_create_keypair_cert(
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

/**
 * \brief Encrypt a keypair certificate using the given password.
 *
 * \param opts              The command-line options to use.
 * \param encrypted_cert    Pointer to the pointer to receive an allocated
 *                          vccrypt_buffer_t instance holding the encrypted
 *                          certificate on function success.
 * \param private_cert      The private certificate to encrypt.
 * \param password          The password to use to derive the encryption key.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int keygen_encrypt_certificate(
    commandline_opts* opts, vccrypt_buffer_t** encrypted_cert,
    const vccrypt_buffer_t* private_cert, const vccrypt_buffer_t* password)
{
    int retval;
    vccrypt_stream_context_t cipher;
    vccrypt_mac_context_t mac;
    vccrypt_prng_context_t prng;
    vccrypt_key_derivation_context_t key_derivation;
    vccrypt_buffer_t salt, iv, derived_key, mac_buffer;
    uint8_t* benc;
    size_t offset = 0;

    /* get keygen and root command. */
    keygen_command* keygen = (keygen_command*)opts->cmd;
    MODEL_ASSERT(NULL != keygen);
    root_command* root = (root_command*)keygen->hdr.next;
    MODEL_ASSERT(NULL != root);

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

    /* create a buffer for holding the derived key. */
    /* TODO - replace with suite method. */
    retval =
        vccrypt_buffer_init(
            &derived_key, opts->suite->alloc_opts,
            opts->suite->stream_cipher_opts.key_size);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_iv;
    }

    /* create a buffer for holding the mac. */
    retval =
        vccrypt_suite_buffer_init_for_mac_authentication_code(
            opts->suite, &mac_buffer, false);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_derived_key;
    }

    /* create prng instance for getting salt. */
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

    /* create key derivation instance. */
    retval = vccrypt_suite_key_derivation_init(&key_derivation, opts->suite);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_prng;
    }

    /* derive the key. */
    retval =
        vccrypt_key_derivation_derive_key(
            &derived_key, &key_derivation, password, &salt,
            root->key_derivation_rounds);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* create the mac instance. */
    retval = vccrypt_suite_mac_init(opts->suite, &mac, &derived_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_key_derivation;
    }

    /* create the stream cipher instance. */
    retval = vccrypt_suite_stream_init(opts->suite, &cipher, &derived_key);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_mac;
    }

    /* compute the size of the encrypted certificate file. */
    size_t encrypted_cert_size =
          ENCRYPTED_CERT_MAGIC_SIZE             /* "ENC" */
        + sizeof(uint32_t)                      /* number of rounds in key. */
        + salt.size                             /* the salt. */
        + iv.size                               /* the iv. */
        + private_cert->size                    /* the encrypted certificate. */
        + opts->suite->mac_opts.mac_size;       /* the mac. */

    /* allocate space for the encrypted certificate. */
    *encrypted_cert = (vccrypt_buffer_t*)malloc(sizeof(vccrypt_buffer_t));
    if (NULL == *encrypted_cert)
    {
        goto cleanup_cipher;
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
    uint32_t net_rounds = htonl(root->key_derivation_rounds);
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
            &cipher, private_cert->data, private_cert->size, benc, &offset);
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
    goto cleanup_cipher;

cleanup_encrypted_cert:
    dispose((disposable_t*)*encrypted_cert);

free_encrypted_cert:
    free(*encrypted_cert);

cleanup_cipher:
    dispose((disposable_t*)&cipher);

cleanup_mac:
    dispose((disposable_t*)&mac);

cleanup_key_derivation:
    dispose((disposable_t*)&key_derivation);

cleanup_prng:
    dispose((disposable_t*)&prng);

cleanup_mac_buffer:
    dispose((disposable_t*)&mac_buffer);

cleanup_derived_key:
    dispose((disposable_t*)&derived_key);

cleanup_iv:
    dispose((disposable_t*)&iv);

cleanup_salt:
    dispose((disposable_t*)&salt);

done:
    return retval;
}

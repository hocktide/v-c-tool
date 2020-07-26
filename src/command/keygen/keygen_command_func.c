/**
 * \file command/keygen/keygen_command_func.c
 *
 * \brief Entry point for the keygen command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <vccrypt/compare.h>
#include <vctool/certificate.h>
#include <vctool/commandline.h>
#include <vctool/command/keygen.h>
#include <vctool/command/root.h>
#include <vctool/readpassword.h>

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
    retval = keypair_certificate_create(opts, &private_cert);
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
            certificate_encrypt(
                opts, &encrypted_cert, &private_cert, &password_buffer,
                root->key_derivation_rounds);
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

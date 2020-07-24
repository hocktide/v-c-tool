/**
 * \file command/pubkey/process_pubkey_command.c
 *
 * \brief Process command-line options to build a pubkey command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/pubkey.h>
#include <vctool/command/root.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Process the pubkey command.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int process_pubkey_command(
    commandline_opts* opts, int UNUSED(argc), char* UNUSED(argv[]))
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* allocate memory for a pubkey_command structure. */
    pubkey_command* pubkey = (pubkey_command*)malloc(sizeof(pubkey_command));
    if (NULL == pubkey)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize the structure. */
    retval = pubkey_command_init(pubkey);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto free_pubkey;
    }

    /* set pubkey command as the head of opts command. */
    pubkey->hdr.next = opts->cmd;
    opts->cmd = &pubkey->hdr;

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto done;

free_pubkey:
    free(pubkey);

done:
    return retval;
}

/**
 * \file command/keygen/process_keygen_command.c
 *
 * \brief Process command-line options to build a keygen command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/keygen.h>
#include <vctool/command/root.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>
#include <unistd.h>
#include <vpr/parameters.h>

/**
 * \brief Process the keygen command.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int process_keygen_command(
    commandline_opts* opts, int UNUSED(argc), char* UNUSED(argv[]))
{
    int retval;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* allocate memory for a keygen_command structure. */
    keygen_command* keygen = (keygen_command*)malloc(sizeof(keygen_command));
    if (NULL == keygen)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize the structure. */
    retval = keygen_command_init(keygen);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto free_keygen;
    }

    /* set keygen command as the head of opts command. */
    keygen->hdr.next = opts->cmd;
    opts->cmd = &keygen->hdr;

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;
    goto done;

free_keygen:
    free(keygen);

done:
    return retval;
}

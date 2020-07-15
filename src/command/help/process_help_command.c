/**
 * \file command/help/process_help_command.c
 *
 * \brief Process the help command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <stdio.h>
#include <string.h>
#include <vctool/command/help.h>
#include <vctool/status_codes.h>
#include <vpr/parameters.h>

/* forward decls. */
static void dispose_help_command(void* disp);
static int help_command_func(commandline_opts* opts);

/**
 * \brief Process the help command.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int process_help_command(
    commandline_opts* opts, int UNUSED(argc), char* UNUSED(argv[]))
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* attempt to allocate help command structure. */
    help_command* help = (help_command*)malloc(sizeof(help_command));
    if (NULL == help)
    {
        return VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
    }

    /* clear help command structure. */
    memset(help, 0, sizeof(help_command));

    /* set up help command structure. */
    help->hdr.hdr.dispose = &dispose_help_command;
    help->hdr.next = opts->cmd;
    help->hdr.func = &help_command_func;

    /* set opts command. */
    opts->cmd = &help->hdr;

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of the help command structure.
 *
 * \param disp      Help command structure to dispose.
 */
static void dispose_help_command(void* UNUSED(disp))
{
    /* do nothing. */
}

/**
 * \brief Execute the help command.
 *
 * \param opts      The command-line options for running this command.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - A non-zero error code on failure.
 */
static int help_command_func(commandline_opts* UNUSED(opts))
{
    help_print(stdout);

    return VCTOOL_STATUS_SUCCESS;
}

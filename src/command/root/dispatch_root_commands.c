/**
 * \file command/root/dispatch_root_commands.c
 *
 * \brief Dispatch root commands.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <stdio.h>
#include <string.h>
#include <vctool/command/help.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>

/**
 * \brief Dispatch root level commands.
 *
 * \param opts          The command-line option structure.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int dispatch_root_commands(commandline_opts* opts, int argc, char* argv[])
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* we should have at least one argument. */
    if (argc < 1)
    {
        fprintf(stderr, "Expecting a command.\n");

        return VCTOOL_ERROR_COMMANDLINE_MISSING_ARGUMENT;
    }

    /* decode the command. */
    char* command = argv[0];

    /* is this the help command? */
    if (!strcmp(command, "help"))
    {
        return process_help_command(opts, argc - 1, argv + 1);
    }
    /* handle unknown command. */
    else
    {
        fprintf(stderr, "Unknown command %s.\n", command);

        return VCTOOL_ERROR_COMMANDLINE_UNKNOWN_COMMAND;
    }
}

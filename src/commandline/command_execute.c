/**
 * \file commandline/command_execute.c
 *
 * \brief Execute a command from the options structure, if possible.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <stdio.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>

/**
 * \brief Execute a command.
 *
 * \param opts          The commandline options for this command.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 */
int command_execute(commandline_opts* opts)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));

    /* is there a command to execute? */
    if (NULL != opts->cmd->func)
    {
        return opts->cmd->func(opts);
    }
    else
    {
        fprintf(stderr, "Missing command.\n");

        return VCTOOL_ERROR_COMMANDLINE_MISSING_COMMAND;
    }
}

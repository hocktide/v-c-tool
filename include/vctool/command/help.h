/**
 * \file include/vctool/command/help.h
 *
 * \brief Functions relating to the help command.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMAND_HELP_HEADER_GUARD
# define VCTOOL_COMMAND_HELP_HEADER_GUARD

#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Command structure for the help command.
 */
typedef struct help_command
{
    command hdr;
} help_command;

/**
 * \brief Print help to the given stream.
 *
 * \param out           The output stream to which help is printed.
 */
void help_print(FILE* out);

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
int process_help_command(commandline_opts* opts, int argc, char* argv[]);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMAND_HELP_HEADER_GUARD*/

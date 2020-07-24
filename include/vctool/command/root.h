/**
 * \file include/vctool/command/root.h
 *
 * \brief Root command structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMAND_ROOT_HEADER_GUARD
# define VCTOOL_COMMAND_ROOT_HEADER_GUARD

#include <stdbool.h>
#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/* the default number of rounds to use for deriving a key. */
#define ROOT_COMMAND_DEFAULT_KEY_DERIVATION_ROUNDS      50000

typedef struct root_command
{
    command hdr;
    bool help_requested;
    char* output_filename;
    char* key_filename;
    unsigned int key_derivation_rounds;
} root_command;

/**
 * \brief Initialize a root command structure.
 *
 * \param root          The root command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int root_command_init(root_command* root);

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
int dispatch_root_commands(commandline_opts* opts, int argc, char* argv[]);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMAND_ROOT_HEADER_GUARD*/

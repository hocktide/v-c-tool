/**
 * \file include/vctool/command/pubkey.h
 *
 * \brief Pubkey command structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMAND_PUBKEY_HEADER_GUARD
# define VCTOOL_COMMAND_PUBKEY_HEADER_GUARD

#include <stdbool.h>
#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct pubkey_command
{
    command hdr;
} pubkey_command;

/**
 * \brief Initialize a pubkey command structure.
 *
 * \param pubkey        The pubkey command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int pubkey_command_init(pubkey_command* pubkey);

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
int process_pubkey_command(commandline_opts* opts, int argc, char* argv[]);

/**
 * \brief Execute the pubkey command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int pubkey_command_func(commandline_opts* opts);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMAND_PUBKEY_HEADER_GUARD*/

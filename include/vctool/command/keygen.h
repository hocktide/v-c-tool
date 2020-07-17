/**
 * \file include/vctool/command/keygen.h
 *
 * \brief Keygen command structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMAND_KEYGEN_HEADER_GUARD
# define VCTOOL_COMMAND_KEYGEN_HEADER_GUARD

#include <stdbool.h>
#include <stdio.h>
#include <vctool/commandline.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct keygen_command
{
    command hdr;
} keygen_command;

/**
 * \brief Initialize a keygen command structure.
 *
 * \param keygen        The keygen command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int keygen_command_init(keygen_command* keygen);

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
int process_keygen_command(commandline_opts* opts, int argc, char* argv[]);

/**
 * \brief Execute the keygen command.
 *
 * \param opts          The commandline opts for this operation.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int keygen_command_func(commandline_opts* opts);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMAND_KEYGEN_HEADER_GUARD*/

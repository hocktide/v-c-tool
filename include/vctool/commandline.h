/**
 * \file include/vctool/commandline.h
 *
 * \brief Commandline parsing.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_COMMANDLINE_HEADER_GUARD
# define VCTOOL_COMMANDLINE_HEADER_GUARD

#include <vccrypt/suite.h>
#include <vctool/file.h>
#include <vpr/disposable.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/* forward decls */
typedef struct commandline_opts commandline_opts;
typedef struct command command;

/**
 * \brief commandline options.
 */
struct commandline_opts
{
    /** \brief this is disposable. */
    disposable_t hdr;

    /** \brief the file abstraction layer for this command. */
    file* file;

    /** \brief crypto suite to use with this command. */
    vccrypt_suite_options_t* suite;

    /** \brief command context with config. */
    command* cmd;
};

/**
 * \brief command config.
 */
struct command
{
    disposable_t hdr;
    command* next;
    int (*func)(commandline_opts*);
};

/**
 * \brief Parse command-line options, initializing a commandline_opts structure.
 *
 * \param opts          The commandline_opts structure to initialize.
 * \param file          The file abstraction layer to use.
 * \param suite         The crypto suite to use.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 */
int commandline_opts_init(
    commandline_opts* opts, file* file, vccrypt_suite_options_t* suite,
    int argc, char* argv[]);

/**
 * \brief Execute a command.
 *
 * \param opts          The commandline options for this command.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 */
int command_execute(commandline_opts* opts);

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_COMMANDLINE_HEADER_GUARD*/

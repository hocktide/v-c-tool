/**
 * \file vctool/main.c
 *
 * \brief Main entry point for the vctool utility.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <stdio.h>
#include <vccrypt/suite.h>
#include <vctool/file.h>
#include <vctool/command/help.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>
#include <vpr/allocator/malloc_allocator.h>

/**
 * \brief Main entry point for vctool.
 *
 * \param argc      The number of arguments.
 * \param argv      Arguments to main.
 *
 * \returns 0 on success and non-zero on failure.
 */
int main(int argc, char* argv[])
{
    int retval;
    commandline_opts opts;
    allocator_options_t alloc_opts;
    vccrypt_suite_options_t suite;
    file file;

    /* register the velo v1 suite. */
    vccrypt_suite_register_velo_v1();

    /* initialize the allocator. */
    malloc_allocator_options_init(&alloc_opts);

    /* initialize the vccrypt suite. */
    retval =
        vccrypt_suite_options_init(&suite, &alloc_opts, VCCRYPT_SUITE_VELO_V1);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error initializing crypto suite.\n");
        goto cleanup_allocator;
    }

    /* create OS level file abstraction. */
    retval = file_init(&file);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error creating file abstraction layer.\n");
        goto cleanup_crypto_suite;
    }

    /* parse command-line options. */
    retval = commandline_opts_init(&opts, &file, &suite, argc, argv);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error parsing command-line options.\n\n");
        help_print(stderr);

        goto cleanup_file;
    }

    /* attempt to execute the command. */
    retval = command_execute(&opts);

    /* clean up opts. */
    dispose((disposable_t*)&opts);

cleanup_file:
    dispose((disposable_t*)&file);

cleanup_crypto_suite:
    dispose((disposable_t*)&suite);

cleanup_allocator:
    dispose((disposable_t*)&alloc_opts);

    return retval;
}

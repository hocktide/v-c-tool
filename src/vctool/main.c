/**
 * \file vctool/main.c
 *
 * \brief Main entry point for the vctool utility.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <stdio.h>
#include <vctool/command/help.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>

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

    /* parse command-line options. */
    retval = commandline_opts_init(&opts, argc, argv);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        fprintf(stderr, "Error parsing command-line options.\n\n");
        help_print(stderr);

        goto done;
    }

    /* attempt to execute the command. */
    retval = command_execute(&opts);

    /* clean up opts. */
    dispose((disposable_t*)&opts);

done:
    return retval;
}

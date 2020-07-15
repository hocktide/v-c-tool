/**
 * \file command/help/help_print.c
 *
 * \brief Print the help menu.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <vctool/command/help.h>

/**
 * \brief Print help to the given stream.
 *
 * \param out           The output stream to which help is printed.
 */
void help_print(FILE* out)
{
    fprintf(out, "Usage: vctool [options] command [command-options]\n\n");

    fprintf(out, "Commands:\n");
    fprintf(out, "   %-12s Print this help menu.\n", "help");
}

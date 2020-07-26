/**
 * \file commandline/commandline_opts_init.c
 *
 * \brief Parse the commandline, creating an options structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <unistd.h>
#include <vctool/command/help.h>
#include <vctool/command/root.h>
#include <vctool/commandline.h>
#include <vctool/status_codes.h>

/* forward decls */
static void commandline_opts_dispose(void* disp);

/**
 * \brief Parse command-line options, initializing a commandline_opts structure.
 *
 * \param opts          The commandline_opts structure to initialize.
 * \param file          The file abstraction layer to use.
 * \param suite         The crypto suite to use.
 * \param builder_opts  The certificate builder options to use.
 * \param argc          The argument count.
 * \param argv          The argument vector.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 */
int commandline_opts_init(
    commandline_opts* opts, file* file, vccrypt_suite_options_t* suite,
    vccert_builder_options_t* builder_opts, int argc, char* argv[])
{
    int ch, retval, rounds;

    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != opts);
    MODEL_ASSERT(argc > 0);
    MODEL_ASSERT(NULL != argv);

    /* reset the option indicator. */
    optind = 0;
#ifdef __OpenBSD__
    optreset = 1; /* on OpenBSD, we must also specify optreset. */
#endif

    /* clear the opts structure. */
    memset(opts, 0, sizeof(commandline_opts));

    /* allocate space for the root command context. */
    root_command* root = (root_command*)malloc(sizeof(root_command));
    if (NULL == root)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        goto done;
    }

    /* initialize root command structure. */
    retval = root_command_init(root);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto cleanup_root_command;
    }

    /* set opt fields. */
    opts->hdr.dispose = &commandline_opts_dispose;
    opts->file = file;
    opts->suite = suite;
    opts->builder_opts = builder_opts;
    opts->cmd = (command*)root;

    /* read through command-line options. */
    while ((ch = getopt(argc, argv, "?R:hk:o:")) != -1)
    {
        switch (ch)
        {
            case '?':
            case 'h':
                root->help_requested = true;
                break;

            case 'k':
                if (NULL != root->key_filename)
                {
                    fprintf(stderr, "duplicate option -k %s\n", optarg);
                    retval = VCTOOL_ERROR_COMMANDLINE_DUPLICATE_OPTION;
                    goto dispose_opts;
                }
                root->key_filename = strdup(optarg);
                break;

            case 'o':
                if (NULL != root->output_filename)
                {
                    fprintf(stderr, "duplicate option -o %s\n", optarg);
                    retval = VCTOOL_ERROR_COMMANDLINE_DUPLICATE_OPTION;
                    goto dispose_opts;
                }
                root->output_filename = strdup(optarg);
                break;

            case 'R':
                rounds = atoi(optarg);
                if (rounds <= 0)
                {
                    fprintf(stderr, "Key derivation rounds must be > 0.\n");
                    retval = VCTOOL_ERROR_COMMANDLINE_BAD_KEY_ROUNDS;
                    goto dispose_opts;
                }
                root->key_derivation_rounds = (unsigned int)rounds;
                break;
        }
    }

    /* if help is requested, then set the help command. */
    if (root->help_requested)
    {
        retval = process_help_command(opts, argc - optind, argv + optind);
        if (VCTOOL_STATUS_SUCCESS != retval)
        {
            goto dispose_opts;
        }
        goto done;
    }

    /* otherwise, dispatch a root command. */
    retval = dispatch_root_commands(opts, argc - optind, argv + optind);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto dispose_opts;
    }
    goto done;

dispose_opts:
    dispose((disposable_t*)opts);
    goto done;

cleanup_root_command:
    free(root);

done:
    return retval;
}

/**
 * \brief Dispose of a commandline_opts instance.
 *
 * \param disp      The instance to dispose.
 */
static void commandline_opts_dispose(void* disp)
{
    commandline_opts* opts = (commandline_opts*)disp;

    while (NULL != opts->cmd)
    {
        /* get the next value. */
        command* tmp = opts->cmd->next;

        /* dispose of these options. */
        dispose((disposable_t*)opts->cmd);
        free(opts->cmd);

        /* iterate to the next value. */
        opts->cmd = tmp;
    }
}

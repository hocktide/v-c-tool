/**
 * \file readpassword/readpassword.c
 *
 * \brief Function to read a password without echoing.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <vctool/commandline.h>
#include <vctool/readpassword.h>
#include <vpr/parameters.h>

/* forward decls. */
typedef struct readpassword_savestate readpassword_savestate;
static int readpassword_set_termattrs(struct termios*);
static int readpassword_restore_termattrs(struct termios*);
static int readpassword_install_handlers(readpassword_savestate*);
static int readpassword_restore_handlers(readpassword_savestate*);
static void readpassword_sig_handler(int);

/** \brief signal handler savestate for readpassword. */
struct readpassword_savestate
{
    struct sigaction saved_handlers[32];
    struct sigaction installed_handlers[32];
};

/* globals. */
static struct termios saved_attrs;
static bool exit_loop_error;

/**
 * \brief Read a password from standard input.
 *
 * \param opts              The command-line options.
 * \param passbuffer        Pointer to a vccrypt_buffer_t to be initialized with
 *                          the password / passphrase that has been read.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int readpassword(commandline_opts* opts, vccrypt_buffer_t* passbuffer)
{
    int retval;
    readpassword_savestate state;
    size_t password_offset, password_max;
    char* password_buffer = NULL;
    char ch = 0;

    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_VALID_COMMANDLINE_OPTS(opts));
    MODEL_ASSERT(NULL != passbuffer);

    /* Get current terminal flags. */
    if (tcgetattr(0, &saved_attrs) < 0)
    {
        /* if this fails, it is most likely because fd 0 is not a terminal. */
        retval = VCTOOL_ERROR_READPASSWORD_STDIN_NOT_A_TERMINAL;
        goto done;
    }

    /* install signal handler to restore flags. */
    retval = readpassword_install_handlers(&state);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto done;
    }

    /* set terminal flags to non-canonical / no echo. */
    retval = readpassword_set_termattrs(&saved_attrs);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        readpassword_restore_handlers(&state);
        goto done;
    }

    /* allocate a chunk of memory for holding the passphrase. */
    password_max = 4096;
    password_offset = 0;
    password_buffer = (char*)malloc(password_max);
    if (NULL == password_buffer)
    {
        retval = VCTOOL_ERROR_GENERAL_OUT_OF_MEMORY;
        readpassword_restore_termattrs(&saved_attrs);
        readpassword_restore_handlers(&state);
        goto done;
    }

    /* start loop. */
    exit_loop_error = false;
    while (!exit_loop_error && ch != '\n')
    {
        /* attempt to read a character. */
        if (read(0, &ch, 1) < 0)
        {
            exit_loop_error = true;
        }

        /* append the character if it is not enter. */
        if (ch != '\n')
        {
            password_buffer[password_offset++] = ch;
            if (password_offset >= password_max)
            {
                fprintf(stderr, "password size too large.\n");
                exit_loop_error = true;
            }
        }
    }

    /* restore terminal flags. */
    retval = readpassword_restore_termattrs(&saved_attrs);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        readpassword_restore_handlers(&state);
        goto cleanup_buffer;
    }

    /* uninstall signal handler / restore old signal handler. */
    retval = readpassword_restore_handlers(&state);
    if (VCTOOL_STATUS_SUCCESS != retval)
    {
        goto cleanup_buffer;
    }

    /* exit the loop if an error was encountered. */
    if (exit_loop_error)
    {
        retval = VCTOOL_ERROR_READPASSWORD_READ;
        goto cleanup_buffer;
    }

    /* allocate the password buffer. */
    retval =
        vccrypt_buffer_init(
            passbuffer, opts->suite->alloc_opts, password_offset);
    if (VCCRYPT_STATUS_SUCCESS != retval)
    {
        goto cleanup_buffer;
    }

    /* copy the password into this buffer. */
    memcpy(passbuffer->data, password_buffer, passbuffer->size);

    /* success. */
    retval = VCTOOL_STATUS_SUCCESS;

cleanup_buffer:
    memset(password_buffer, 0, password_max);
    free(password_buffer);

done:
    return retval;
}

/**
 * \brief Using the provided saved terminal attributes, set the terminal
 * attributes for reading a password (non-canonical / no echo).
 *
 * \param sattrs            The saved attributes upon which to base these
 *                          attributes.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int readpassword_set_termattrs(struct termios* sattrs)
{
    struct termios attrs;

    /* copy the saved attributes into our attrtibutes. */
    memcpy(&attrs, sattrs, sizeof(attrs));

    /* disable the canonical and echo flags. */
    attrs.c_lflag &= ~ICANON;
    attrs.c_lflag &= ~ECHO;

    /* read should read one character at a time and block. */
    attrs.c_cc[VMIN] = 1;
    attrs.c_cc[VTIME] = 0;

    /* set the terminal attributes for standard input. */
    if (tcsetattr(0, TCSANOW, &attrs) < 0)
    {
        return VCTOOL_ERROR_READPASSWORD_TCSETATTR;
    }

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Restore the previously saved terminal attributes.
 *
 * \param sattrs            The saved attributes to restore.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int readpassword_restore_termattrs(struct termios* sattrs)
{
    /* set the terminal attributes for standard input. */
    if (tcsetattr(0, TCSANOW, sattrs) < 0)
    {
        return VCTOOL_ERROR_READPASSWORD_TCSETATTR;
    }

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief This signal handler restores terminal attributes and sets the
 * exit_loop_error flag.
 *
 * \param sig               The signal that triggered this handler.
 */
static void readpassword_sig_handler(int UNUSED(sig))
{
    exit_loop_error = true;

    readpassword_restore_termattrs(&saved_attrs);
}

/**
 * \brief Install signal handlers to restore the terminal state and exit with an
 * error when a signal is encountered.
 *
 * \param state             The save state for restoring these handlers.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int readpassword_install_handlers(readpassword_savestate* state)
{
    /* clear the save state. */
    memset(state, 0, sizeof(readpassword_savestate));

    /* iterate through the 32 possible signal handlers, saving their state and
     * installing new handlers. */
    for (int i = 0; i < 32; ++i)
    {
        state->installed_handlers[i].sa_handler = &readpassword_sig_handler;
        sigaction(
            i+1, &state->installed_handlers[i], &state->saved_handlers[i]);
    }

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Restore the previously saved handlers.
 *
 * \param state             The save state for restoring these handlers.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
static int readpassword_restore_handlers(readpassword_savestate* state)
{
    /* iterate through the 32 possible signal handlers, saving their state and
     * installing new handlers. */
    for (int i = 0; i < 32; ++i)
    {
        sigaction(i+1, &state->saved_handlers[i], NULL);
    }

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

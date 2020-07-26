/**
 * \file command/keygen/keygen_command_init.c
 *
 * \brief Initialize a keygen command structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/keygen.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>
#include <vpr/parameters.h>

/* forward decls. */
static void keygen_command_dispose(void* disp);

/**
 * \brief Initialize a keygen command structure.
 *
 * \param keygen        The keygen command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int keygen_command_init(keygen_command* keygen)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != keygen);

    /* clear keygen command structure. */
    memset(keygen, 0, sizeof(keygen_command));

    /* set disposer, func, etc. */
    keygen->hdr.hdr.dispose = &keygen_command_dispose;
    keygen->hdr.func = &keygen_command_func;

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a keygen_command structure.
 *
 * \param disp          The keygen_command structure to dispose.
 */
static void keygen_command_dispose(void* UNUSED(disp))
{
    /* do nothing. */
}

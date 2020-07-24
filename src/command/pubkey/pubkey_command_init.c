/**
 * \file command/pubkey/pubkey_command_init.c
 *
 * \brief Initialize a pubkey command structure.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <string.h>
#include <vctool/command/pubkey.h>
#include <vctool/command/root.h>
#include <vctool/status_codes.h>
#include <vpr/parameters.h>

/* forward decls. */
static void pubkey_command_dispose(void* disp);

/**
 * \brief Initialize a pubkey command structure.
 *
 * \param pubkey        The pubkey command structure to initialize.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int pubkey_command_init(pubkey_command* pubkey)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(NULL != pubkey);

    /* clear pubkey command structure. */
    memset(pubkey, 0, sizeof(pubkey_command));

    /* set disposer, func, etc. */
    pubkey->hdr.hdr.dispose = &pubkey_command_dispose;
    pubkey->hdr.func = &pubkey_command_func;

    /* success. */
    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a pubkey_command structure.
 *
 * \param disp          The pubkey_command structure to dispose.
 */
static void pubkey_command_dispose(void* UNUSED(disp))
{
    /* do nothing. */
}

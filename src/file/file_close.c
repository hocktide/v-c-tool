/**
 * \file file/file_close.c
 *
 * \brief Implementation of file_close.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/**
 * \brief Close a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor to close.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is bad.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation was interrupted by the
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_IO if an I/O error occurred during this operation.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on the device.
 *      - VCTOOL_ERROR_FILE_QUOTA if user quota has been exceeded.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error has occurred.
 */
int file_close(file* f, int d)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);

    return f->file_close_method(f, d);
}

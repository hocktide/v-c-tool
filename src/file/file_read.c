/**
 * \file file/file_read.c
 *
 * \brief Implementation of file_read.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/**
 * \brief Read from a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor from which to read.
 * \param buf       The buffer to read into.
 * \param max       The maximum number of bytes to read.
 * \param rbytes    Pointer to the size_t variable to hold the number of bytes
 *                  read.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if this read would cause the process to
 *        block and no blocking is enabled.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is invalid.
 *      - VCTOOL_ERROR_FILE_FAULT if this operation causes a memory fault.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation is interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if this read operation is invalid due
 *        to the way that this file descriptor was opened or due to the nature
 *        of the file or device.
 *      - VCTOOL_ERROR_FILE_IO if an I/O error has occurred.
 *      - VCTOOL_ERROR_FILE_IS_DIRECTORY if an attempt to perform a normal read
 *        on a directory file descriptor has been made.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error has occurred.
 */
int file_read(file* f, int d, void* buf, size_t max, size_t* rbytes)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != rbytes);

    return f->file_read_method(f, d, buf, max, rbytes);
}

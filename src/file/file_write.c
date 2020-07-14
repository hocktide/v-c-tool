/**
 * \file file/file_write.c
 *
 * \brief Implementation of file_write.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/**
 * \brief Write to a file descriptor.
 *
 * \param f         The file interface.
 * \param d         The descriptor to which data is written.
 * \param buf       The buffer to write from.
 * \param max       The maximum number of bytes to write.
 * \param wbytes    Pointer to the size_t variable to hold the number of bytes
 *                  written.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if the operation would cause the process
 *        to block and no blocking has been set.
 *      - VCTOOL_ERROR_FILE_BAD_DESCRIPTOR if the file descriptor is bad.
 *      - VCTOOL_ERROR_FILE_QUOTA if this operation violates a user quota on
 *        disk space.
 *      - VCTOOL_ERROR_FILE_FAULT if this operation causes a memory fault.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if an attempt is made to write to a file
 *        that exceeds disk or user limits.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation is interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if this operation violates flags set
 *        for this descriptor.
 *      - VCTOOL_ERROR_FILE_IO if a low-level I/O error occurs.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on this device.
 *      - VCTOOL_ERROR_FILE_ACCESS if an access / permission issue occurs.
 *      - VCTOOL_ERROR_FILE_BROKEN_PIPE if the remote end of this descriptor is
 *        disconnected.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error occurs.
 */
int file_write(file* f, int d, const void* buf, size_t max, size_t* wbytes)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(d >= 0);
    MODEL_ASSERT(NULL != buf);
    MODEL_ASSERT(NULL != wbytes);

    return f->file_write_method(f, d, buf, max, wbytes);
}

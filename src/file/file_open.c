/**
 * \file file/file_open.c
 *
 * \brief Implementation of file_open.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/**
 * \brief Open a file for I/O.
 *
 * \param f         The file interface.
 * \param d         Pointer to the descriptor to hold the open file.
 * \param path      Path to the file to open.
 * \param flags     Flags to use when opening the file.
 * \param mode      Mode to use when creating a file.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_ACCESS if permission or options make this file
 *        inacessible for the desired operation or mode.
 *      - VCTOOL_ERROR_FILE_QUOTA if this operation exceeds the quota for this
 *        user.
 *      - VCTOOL_ERROR_FILE_EXISTS if the file already exists and the options
 *        were to create a new file.
 *      - VCTOOL_ERROR_FILE_FAULT if a parameter passed to this method caused a
 *        read / write fault.
 *      - VCTOOL_ERROR_FILE_INTERRUPT if this operation was interrupted by a
 *        signal handler.
 *      - VCTOOL_ERROR_FILE_INVALID_FLAGS if the flags provided were invalid for
 *        this file.
 *      - VCTOOL_ERROR_FILE_IS_DIRECTORY if an attempt was made to treat a
 *        directory as a file.
 *      - VCTOOL_ERROR_FILE_LOOP if too many symlinks were encountered during
 *        this operation.
 *      - VCTOOL_ERROR_FILE_TOO_MANY_FILES if too many files are open for this
 *        process, user, or the whole system.
 *      - VCTOOL_ERROR_FILE_NAME_TOO_LONG if the path name is too long.
 *      - VCTOOL_ERROR_FILE_NO_ENTRY if an attempt was made to access a file
 *        that does not exist.
 *      - VCTOOL_ERROR_FILE_KERNEL_MEMORY if the kernel ran out of memory.
 *      - VCTOOL_ERROR_FILE_NO_SPACE if there is no space left on this device.
 *      - VCTOOL_ERROR_FILE_NOT_DIRECTORY if a component of the path is not a
 *        directory.
 *      - VCTOOL_ERROR_FILE_NOT_SUPPORTED if the options or name are not
 *        supported on this device.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if an overflow would occur in userspace due
 *        to this operation.
 *      - VCTOOL_ERROR_FILE_WOULD_BLOCK if this operation would block the
 *        process and no blocking is enabled.
 *      - VCTOOL_ERROR_FILE_UNKNOWN if an unknown error occurred.
 */
int file_open(file* f, int* d, const char* path, int flags, mode_t mode)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(NULL != d);
    MODEL_ASSERT(NULL != path);

    return f->file_open_method(f, d, path, flags, mode);
}

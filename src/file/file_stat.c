/**
 * \file file/file_stat.c
 *
 * \brief Implementation of file_stat.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <cbmc/model_assert.h>
#include <vctool/file.h>
#include <vpr/parameters.h>

/**
 * \brief Get file stats.
 *
 * \param f         The file interface to use to gather stats.
 * \param path      Path to the file to stat.
 * \param filestat  Structure to populate with stat information.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - VCTOOL_ERROR_FILE_ACCESS if this failed due to permissions.
 *      - VCTOOL_ERROR_FILE_LOOP if too many symlinks were encountered.
 *      - VCTOOL_ERROR_FILE_NAME_TOO_LONG if the pathname is too long.
 *      - VCTOOL_ERROR_FILE_NO_ENTRY if a component of the pathname does not
 *        exist.
 *      - VCTOOL_ERROR_FILE_KERNEL_MEMORY if the kernel ran out of memory.
 *      - VCTOOL_ERROR_FILE_NOT_DIRECTORY if a component of the file path is not
 *        a directory.
 *      - VCTOOL_ERROR_FILE_OVERFLOW if some value caused overflow.
 *      - VCTOOL_ERROR_FILE_UNKNOWN some unknown error occurred.
 */
int file_stat(file* f, const char* path, file_stat_st* filestat)
{
    /* parameter sanity checks. */
    MODEL_ASSERT(PROP_FILE_VALID(f));
    MODEL_ASSERT(NULL != path);
    MODEL_ASSERT(NULL != filestat);

    return f->file_stat_method(f, path, filestat);
}

/**
 * \file include/vctool/status_codes/file.h
 *
 * \brief file component status codes.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef VCTOOL_STATUS_CODES_FILE_HEADER_GUARD
#define VCTOOL_STATUS_CODES_FILE_HEADER_GUARD

#include <vctool/status_codes.h>

/* make this header C++ friendly. */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief File access / permission error.
 */
#define VCTOOL_ERROR_FILE_ACCESS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0001U)

/**
 * \brief Too many symlinks encountered trying to resolve file path.
 */
#define VCTOOL_ERROR_FILE_LOOP \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0002U)

/**
 * \brief The path name was too long.
 */
#define VCTOOL_ERROR_FILE_NAME_TOO_LONG \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0003U)

/**
 * \brief A component of the path was not found.
 */
#define VCTOOL_ERROR_FILE_NO_ENTRY \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0004U)

/**
 * \brief The kernel ran out of memory performing this task.
 */
#define VCTOOL_ERROR_FILE_KERNEL_MEMORY \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0005U)

/**
 * \brief A component of the path was not a directory and needed to be.
 */
#define VCTOOL_ERROR_FILE_NOT_DIRECTORY \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0006U)

/**
 * \brief A value would cause overflow when entered into a user space structure.
 */
#define VCTOOL_ERROR_FILE_OVERFLOW \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0007U)

/**
 * \brief An unknown error occurred.
 */
#define VCTOOL_ERROR_FILE_UNKNOWN \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0008U)

/**
 * \brief A disk quota violation was tripped.
 */
#define VCTOOL_ERROR_FILE_QUOTA \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0009U)

/**
 * \brief The file already exists.
 */
#define VCTOOL_ERROR_FILE_EXISTS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000AU)

/**
 * \brief A memory fault was encountered when processing this request.
 */
#define VCTOOL_ERROR_FILE_FAULT \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000BU)

/**
 * \brief A blocking call was interrupted by the signal handler.
 */
#define VCTOOL_ERROR_FILE_INTERRUPT \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000CU)

/**
 * \brief Invalid flags were passed to the method.
 */
#define VCTOOL_ERROR_FILE_INVALID_FLAGS \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000DU)

/**
 * \brief An attempt was made to treat a directory as a file.
 */
#define VCTOOL_ERROR_FILE_IS_DIRECTORY \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000EU)

/**
 * \brief Too many files are open.
 */
#define VCTOOL_ERROR_FILE_TOO_MANY_FILES \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x000FU)

/**
 * \brief There is no space left on the device.
 */
#define VCTOOL_ERROR_FILE_NO_SPACE \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0010U)

/**
 * \brief The given name, temporary name, or option is not supported on this
 * filesystem.
 */
#define VCTOOL_ERROR_FILE_NOT_SUPPORTED \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0011U)

/**
 * \brief This operation would block the process and no blocking has been
 * enabled.
 */
#define VCTOOL_ERROR_FILE_WOULD_BLOCK \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0012U)

/**
 * \brief The file descriptor is bad.
 */
#define VCTOOL_ERROR_FILE_BAD_DESCRIPTOR \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0013U)

/**
 * \brief An I/O error has occurred.
 */
#define VCTOOL_ERROR_FILE_IO \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0014U)

/**
 * \brief The remote end of this pipe or socket was broken.
 */
#define VCTOOL_ERROR_FILE_BROKEN_PIPE \
    VCTOOL_STATUS_ERROR_MACRO(VCTOOL_COMPONENT_FILE, 0x0015U)

/* make this header C++ friendly. */
#ifdef __cplusplus
}
#endif

#endif /*VCTOOL_STATUS_CODES_FILE_HEADER_GUARD*/

/**
 * \file test/file/mock_file.h
 *
 * \brief Mock for file I/O.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#ifndef  VCTOOL_TEST_FILE_MOCK_HEADER_GUARD
# define VCTOOL_TEST_FILE_MOCK_HEADER_GUARD

#include <vctool/file.h>

/* Require C++. */
#ifndef __cplusplus
#error C++ required for this header.
#endif

#include <functional>

struct mock_file
{
    std::function<int (file*, const char*, file_stat_st*)> mockstat;
    std::function<int (file*, int*, const char*, int, mode_t)> mockopen;
    std::function<int (file*, int)> mockclose;
    std::function<int (file*, int, void*, size_t, size_t*)> mockread;
    std::function<int (file*, int, const void*, size_t, size_t*)> mockwrite;
};

extern const
std::function<int (file*, const char*, file_stat_st*)> stubstat;
extern const
std::function<int (file*, int*, const char*, int, mode_t)> stubopen;
extern const
std::function<int (file*, int)> stubclose;
extern const
std::function<int (file*, int, void*, size_t, size_t*)> stubread;
extern const
std::function<int (file*, int, const void*, size_t, size_t*)> stubwrite;

/**
 * \brief Initialize a mock file interface.
 *
 * \param f             The file interface to initialize.
 * \param mockstat      The mock stat function.
 * \param mockopen      The mock open function.
 * \param mockclose     The mock close function.
 * \param mockread      The mock read function.
 * \param mockwrite     The mock write function.
 *
 * \returns a status code indicating success or failure.
 *      - VCTOOL_STATUS_SUCCESS on success.
 *      - a non-zero error code on failure.
 */
int file_mock_init(
    file* f,
    std::function<int (file*, const char*, file_stat_st*)> mockstat,
    std::function<int (file*, int*, const char*, int, mode_t)> mockopen,
    std::function<int (file*, int)> mockclose,
    std::function<int (file*, int, void*, size_t, size_t*)> mockread,
    std::function<int (file*, int, const void*, size_t, size_t*)> mockwrite);

#endif /*VCTOOL_TEST_FILE_MOCK_HEADER_GUARD*/

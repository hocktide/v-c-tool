/**
 * \file test/file/mock_file.cpp
 *
 * \brief Implementation of mock for file I/O.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <string.h>

#include "mock_file.h"

using namespace std;

/* forward decls. */
static void mock_file_dispose(void*);
static int mock_file_stat(file*, const char*, file_stat_st*);
static int mock_file_open(file*, int*, const char*, int, mode_t);
static int mock_file_close(file*, int);
static int mock_file_read(file*, int, void*, size_t, size_t*);
static int mock_file_write(file*, int, const void*, size_t, size_t*);

/**
 * \brief Stub for stat.
 */
const function<int (file*, const char*, file_stat_st*)> stubstat =
    [](file*, const char*, file_stat_st*)
    {
        return VCTOOL_ERROR_FILE_UNKNOWN;
    };

/**
 * \brief Stub for open.
 */
const function<int (file*, int*, const char*, int, mode_t)> stubopen =
    [](file*, int*, const char*, int, mode_t)
    {
        return VCTOOL_ERROR_FILE_UNKNOWN;
    };

/**
 * \brief Stub for close.
 */
const function<int (file*, int)> stubclose =
    [](file*, int)
    {
        return VCTOOL_ERROR_FILE_UNKNOWN;
    };

/**
 * \brief Stub for read.
 */
const function<int (file*, int, void*, size_t, size_t*)> stubread =
    [](file*, int, void*, size_t, size_t*)
    {
        return VCTOOL_ERROR_FILE_UNKNOWN;
    };

/**
 * \brief Stub for write.
 */
const function<int (file*, int, const void*, size_t, size_t*)> stubwrite =
    [](file*, int, const void*, size_t, size_t*)
    {
        return VCTOOL_ERROR_FILE_UNKNOWN;
    };

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
    std::function<int (file*, int, const void*, size_t, size_t*)> mockwrite)
{
    mock_file* ctx = new mock_file;

    ctx->mockstat = mockstat;
    ctx->mockopen = mockopen;
    ctx->mockclose = mockclose;
    ctx->mockread = mockread;
    ctx->mockwrite = mockwrite;

    memset(f, 0, sizeof(file));

    f->hdr.dispose = &mock_file_dispose;
    f->file_stat_method = &mock_file_stat;
    f->file_open_method = &mock_file_open;
    f->file_close_method = &mock_file_close;
    f->file_read_method = &mock_file_read;
    f->file_write_method = &mock_file_write;
    f->context = (void*)ctx;

    return VCTOOL_STATUS_SUCCESS;
}

/**
 * \brief Dispose of a mock file instance.
 */
static void mock_file_dispose(void* disp)
{
    file* f = (file*)disp;
    mock_file* ctx = (mock_file*)f->context;

    delete ctx;
}

/**
 * \brief Run the mock for this file stat.
 */
static int mock_file_stat(file* f, const char* path, file_stat_st* fst)
{
    mock_file* ctx = (mock_file*)f->context;

    return ctx->mockstat(f, path, fst);
}

/**
 * \brief Run the mock for this file open.
 */
static int mock_file_open(
    file* f, int* d, const char* path, int flags, mode_t mode)
{
    mock_file* ctx = (mock_file*)f->context;

    return ctx->mockopen(f, d, path, flags, mode);
}

/**
 * \brief Run the mock for this file close.
 */
static int mock_file_close(file* f , int d)
{
    mock_file* ctx = (mock_file*)f->context;

    return ctx->mockclose(f, d);
}

/**
 * \brief Run the mock for this file read.
 */
static int mock_file_read(file* f, int d, void* buf, size_t sz, size_t* psz)
{
    mock_file* ctx = (mock_file*)f->context;

    return ctx->mockread(f, d, buf, sz, psz);
}

/**
 * \brief Run the mock for this file write.
 */
static int mock_file_write(
    file* f, int d, const void* buf, size_t sz, size_t* psz)
{
    mock_file* ctx = (mock_file*)f->context;

    return ctx->mockwrite(f, d, buf, sz, psz);
}

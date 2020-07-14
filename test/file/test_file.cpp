/**
 * \file test/file/test_file.cpp
 *
 * \brief Unit tests for file.
 *
 * \copyright 2020 Velo Payments.  See License.txt for license terms.
 */

#include <minunit/minunit.h>
#include <string.h>
#include <vctool/file.h>

#include "mock_file.h"

/* start of the file test suite. */
TEST_SUITE(file);

/* We can create a file interface with file_init(). */
TEST(file_init_basics)
{
    file f;

    /* empty out file. */
    memset(&f, 0, sizeof(f));

    /* preconditions: all methods are null. */
    TEST_EXPECT(nullptr == f.hdr.dispose);
    TEST_EXPECT(nullptr == f.file_stat_method);
    TEST_EXPECT(nullptr == f.file_open_method);
    TEST_EXPECT(nullptr == f.file_close_method);
    TEST_EXPECT(nullptr == f.file_read_method);
    TEST_EXPECT(nullptr == f.file_write_method);
    TEST_EXPECT(nullptr == f.context);

    /* initialize should succeed. */
    TEST_ASSERT(VCTOOL_STATUS_SUCCESS == file_init(&f));

    /* postconditions: all methods are set. */
    TEST_ASSERT(nullptr != f.hdr.dispose);
    TEST_EXPECT(nullptr != f.file_stat_method);
    TEST_EXPECT(nullptr != f.file_open_method);
    TEST_EXPECT(nullptr != f.file_close_method);
    TEST_EXPECT(nullptr != f.file_read_method);
    TEST_EXPECT(nullptr != f.file_write_method);
    TEST_EXPECT(nullptr == f.context);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* We can create a mock with all stubs, and it works as expected. */
TEST(file_mock_init)
{
    file f;
    int d;
    char buf[10];
    size_t size;
    file_stat_st fst;

    /* empty out file. */
    memset(&f, 0, sizeof(f));

    /* preconditions: all methods are null. */
    TEST_EXPECT(nullptr == f.hdr.dispose);
    TEST_EXPECT(nullptr == f.file_stat_method);
    TEST_EXPECT(nullptr == f.file_open_method);
    TEST_EXPECT(nullptr == f.file_close_method);
    TEST_EXPECT(nullptr == f.file_read_method);
    TEST_EXPECT(nullptr == f.file_write_method);
    TEST_EXPECT(nullptr == f.context);

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, stubstat, stubopen, stubclose, stubread, stubwrite));

    /* postconditions: all methods are set. */
    TEST_ASSERT(nullptr != f.hdr.dispose);
    TEST_EXPECT(nullptr != f.file_stat_method);
    TEST_EXPECT(nullptr != f.file_open_method);
    TEST_EXPECT(nullptr != f.file_close_method);
    TEST_EXPECT(nullptr != f.file_read_method);
    TEST_EXPECT(nullptr != f.file_write_method);
    TEST_EXPECT(nullptr != f.context);

    /* calling file_stat returns VCTOOL_ERROR_FILE_UNKNOWN. */
    TEST_EXPECT(VCTOOL_ERROR_FILE_UNKNOWN == file_stat(&f, "test", &fst));

    /* calling file_open returns VCTOOL_ERROR_FILE_UNKNOWN. */
    TEST_EXPECT(
        VCTOOL_ERROR_FILE_UNKNOWN == file_open(&f, &d, "test", 0, 0));

    /* calling file_close returns VCTOOL_ERROR_FILE_UNKNOWN. */
    TEST_EXPECT(
        VCTOOL_ERROR_FILE_UNKNOWN == file_close(&f, d));

    /* calling file_read returns VCTOOL_ERROR_FILE_UNKNOWN. */
    TEST_EXPECT(
        VCTOOL_ERROR_FILE_UNKNOWN == file_read(&f, d, buf, sizeof(buf), &size));

    /* calling file_write returns VCTOOL_ERROR_FILE_UNKNOWN. */
    TEST_EXPECT(
        VCTOOL_ERROR_FILE_UNKNOWN ==
            file_write(&f, d, buf, sizeof(buf), &size));

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* file_stat passes all parameters and returns the value of its impl. */
TEST(file_stat)
{
    file f;
    const char* EXPECTED_PATH="./test.txt";
    file_stat_st fst;
    int EXPECTED_RETURN_CODE = 27;

    file* got_f = nullptr;
    const char* got_path = nullptr;
    file_stat_st* got_filestat = nullptr;

    /* mock stat. */
    auto statmock = [&](file* f, const char* path, file_stat_st* filestat)
    {
        got_f = f;
        got_path = path;
        got_filestat = filestat;

        return EXPECTED_RETURN_CODE;
    };

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, statmock, stubopen, stubclose, stubread, stubwrite));

    /* calling file_stat returns our code. */
    TEST_EXPECT(EXPECTED_RETURN_CODE == file_stat(&f, EXPECTED_PATH, &fst));
    TEST_EXPECT(got_f == &f);
    TEST_EXPECT(got_path == EXPECTED_PATH);
    TEST_EXPECT(got_filestat == &fst);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* file_open passes all parameters and returns the value of its impl. */
TEST(file_open)
{
    file f;
    const char* EXPECTED_PATH="./test.txt";
    int d;
    int EXPECTED_RETURN_CODE = 27;
    int EXPECTED_FLAGS = 98;
    mode_t EXPECTED_MODE = 127;

    file* got_f = nullptr;
    int* got_d = nullptr;
    const char* got_path = nullptr;
    int got_flags = 0;
    mode_t got_mode = 0;

    /* mock stat. */
    auto openmock = [&](
        file* f, int* d, const char* path, int flags, mode_t mode)
    {
        got_f = f;
        got_d = d;
        got_path = path;
        got_flags = flags;
        got_mode = mode;

        return EXPECTED_RETURN_CODE;
    };

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, stubstat, openmock, stubclose, stubread, stubwrite));

    /* calling file_stat returns our code. */
    TEST_EXPECT(
        EXPECTED_RETURN_CODE ==
            file_open(&f, &d, EXPECTED_PATH, EXPECTED_FLAGS, EXPECTED_MODE));
    TEST_EXPECT(got_f == &f);
    TEST_EXPECT(got_d == &d);
    TEST_EXPECT(got_path == EXPECTED_PATH);
    TEST_EXPECT(got_flags == EXPECTED_FLAGS);
    TEST_EXPECT(got_mode == EXPECTED_MODE);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* file_close passes all parameters and returns the value of its impl. */
TEST(file_close)
{
    file f;
    int EXPECTED_DESCRIPTOR=993;
    int EXPECTED_RETURN_CODE = 27;

    file* got_f = nullptr;
    int got_d = 0;

    /* mock stat. */
    auto closemock = [&](file* f, int d)
    {
        got_f = f;
        got_d = d;

        return EXPECTED_RETURN_CODE;
    };

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, stubstat, stubopen, closemock, stubread, stubwrite));

    /* calling file_stat returns our code. */
    TEST_EXPECT(
        EXPECTED_RETURN_CODE ==
            file_close(&f, EXPECTED_DESCRIPTOR));
    TEST_EXPECT(got_f == &f);
    TEST_EXPECT(got_d == EXPECTED_DESCRIPTOR);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* file_read passes all parameters and returns the value of its impl. */
TEST(file_read)
{
    file f;
    int EXPECTED_DESCRIPTOR = 993;
    char EXPECTED_BUFFER[43];
    int EXPECTED_RETURN_CODE = 27;
    size_t EXPECTED_RBYTES;

    file* got_f = nullptr;
    int got_d = 0;
    void* got_buf = nullptr;
    size_t got_max = 0;
    size_t* got_rbytes = nullptr;

    /* mock stat. */
    auto readmock = [&](
        file* f, int d, void* buf, size_t max, size_t* rbytes)
    {
        got_f = f;
        got_d = d;
        got_buf = buf;
        got_max = max;
        got_rbytes = rbytes;

        return EXPECTED_RETURN_CODE;
    };

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, stubstat, stubopen, stubclose, readmock, stubwrite));

    /* calling file_stat returns our code. */
    TEST_EXPECT(
        EXPECTED_RETURN_CODE ==
            file_read(
                &f, EXPECTED_DESCRIPTOR, EXPECTED_BUFFER,
                sizeof(EXPECTED_BUFFER), &EXPECTED_RBYTES));
    TEST_EXPECT(got_f == &f);
    TEST_EXPECT(got_d == EXPECTED_DESCRIPTOR);
    TEST_EXPECT(got_buf == EXPECTED_BUFFER);
    TEST_EXPECT(got_max == sizeof(EXPECTED_BUFFER));
    TEST_EXPECT(got_rbytes == &EXPECTED_RBYTES);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

/* file_write passes all parameters and returns the value of its impl. */
TEST(file_write)
{
    file f;
    int EXPECTED_DESCRIPTOR = 993;
    char EXPECTED_BUFFER[43];
    int EXPECTED_RETURN_CODE = 27;
    size_t EXPECTED_WBYTES;

    file* got_f = nullptr;
    int got_d = 0;
    const void* got_buf = nullptr;
    size_t got_max = 0;
    size_t* got_wbytes = nullptr;

    /* mock stat. */
    auto writemock = [&](
        file* f, int d, const void* buf, size_t max, size_t* wbytes)
    {
        got_f = f;
        got_d = d;
        got_buf = buf;
        got_max = max;
        got_wbytes = wbytes;

        return EXPECTED_RETURN_CODE;
    };

    /* initialize should succeed. */
    TEST_ASSERT(
        VCTOOL_STATUS_SUCCESS ==
            file_mock_init(
                &f, stubstat, stubopen, stubclose, stubread, writemock));

    /* calling file_stat returns our code. */
    TEST_EXPECT(
        EXPECTED_RETURN_CODE ==
            file_write(
                &f, EXPECTED_DESCRIPTOR, EXPECTED_BUFFER,
                sizeof(EXPECTED_BUFFER), &EXPECTED_WBYTES));
    TEST_EXPECT(got_f == &f);
    TEST_EXPECT(got_d == EXPECTED_DESCRIPTOR);
    TEST_EXPECT(got_buf == EXPECTED_BUFFER);
    TEST_EXPECT(got_max == sizeof(EXPECTED_BUFFER));
    TEST_EXPECT(got_wbytes == &EXPECTED_WBYTES);

    /* dispose the file interface. */
    dispose((disposable_t*)&f);
}

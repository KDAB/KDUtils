/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/file.h>
using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("File")
{

    TEST_CASE("init")
    {
        static_assert(std::is_destructible<File>{}, "File should be destructible");
        static_assert(!std::is_default_constructible<File>{}, "File should not be default constructible");
        static_assert(!std::is_copy_constructible<File>{}, "File should not be copy constructible");
        static_assert(!std::is_copy_assignable<File>{}, "File should not be copy assignable");
        static_assert(!std::is_move_constructible<File>{}, "File should not be move constructible");
        static_assert(!std::is_move_assignable<File>{}, "File should not be move assignable");
    }

    TEST_CASE("checkExists")
    {
        {
            // GIVEN
            File f("some_path_that_doesn_t_exist");

            // THEN
            CHECK(!f.exists());
        }
        {
            // GIVEN
            File f(TST_DIR "tst_file.cpp");

            // THEN
            CHECK(f.exists());
        }
    }

    TEST_CASE("checkSize")
    {
        {
            // GIVEN
            File f("some_path_that_doesn_t_exist");

            // THEN
            CHECK(f.size() == std::uintmax_t(-1));
        }
        {
            // GIVEN
            File f(TST_DIR "tst_file.cpp");
            const auto expectedSize = std::filesystem::file_size(std::filesystem::path(TST_DIR).append("tst_file.cpp"));

            // THEN
            CHECK(f.size() == expectedSize);
        }
    }

    TEST_CASE("checkFileName")
    {
        // GIVEN
        File f(TST_DIR "tst_file.cpp");

        // THEN
        CHECK(f.fileName() == "tst_file.cpp");
    }

    TEST_CASE("checkCanOpenAndReadExistingFile")
    {
        // GIVEN
        File f(TST_DIR "tst_file.cpp");

        // THEN
        CHECK(f.exists());

        // WHEN
        const bool wasOpened = f.open(std::ios::in);

        // THEN
        CHECK(wasOpened);
        CHECK(f.isOpen());

        // WHEN
        const ByteArray fileData = f.readAll();

        // THEN
        CHECK(fileData.size() == f.size());

        // WHEN
        f.close();

        // THEN
        CHECK(!f.isOpen());
    }

    TEST_CASE("checkCanCreateAndWriteFile")
    {
        // GIVEN
        File f(TST_DIR "tst_file_file_create_test.gltf");

        // THEN
        CHECK(!f.exists());

        // WHEN
        const bool wasOpened = f.open(std::ios::out);

        // THEN
        CHECK(wasOpened);
        CHECK(f.isOpen());
        CHECK(f.exists());

        // WHEN
        f.write(ByteArray("{ }"));
        f.flush();

        // THEN
        CHECK(f.size() == 3);

        // WHEN
        f.close();

        // THEN
        CHECK(!f.isOpen());

        // WHEN
        f.write(ByteArray("some_more_content"));
        f.flush();

        // THEN -> Nothing was written
        CHECK(f.size() == 3);

        // WHEN
        f.remove();

        // THEN
        CHECK(!f.exists());
    }
}

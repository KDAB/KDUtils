/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/dir.h>

using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Dir")
{

    TEST_CASE("init")
    {
        static_assert(std::is_destructible<Dir>{}, "Dir should be destructible");
        static_assert(std::is_default_constructible<Dir>{}, "Dir should be default constructible");
        static_assert(std::is_copy_constructible<Dir>{}, "Dir should be copy constructible");
        static_assert(std::is_copy_assignable<Dir>{}, "Dir should copy assignable");
        static_assert(std::is_move_constructible<Dir>{}, "Dir should  move constructible");
        static_assert(std::is_move_assignable<Dir>{}, "Dir should move assignable");
    }

    TEST_CASE("checkExists")
    {
        {
            // GIVEN
            Dir d("some_path_that_doesn_t_exist");

            // THEN
            CHECK(!d.exists());
        }
        {
            // GIVEN
            Dir d(TST_DIR);

            // THEN
            CHECK(d.exists());
        }
    }

    TEST_CASE("checkDirName")
    {
        // GIVEN
        Dir d(TST_DIR);

        // THEN
        CHECK(d.dirName() == "bin");
    }

    TEST_CASE("checkAbsoluteFilePath")
    {
        // GIVEN
        Dir d(TST_DIR);

        // WHEN
        const std::string fileName = "some_file.txt";
        const std::string absPath = d.absoluteFilePath(fileName);

        // THEN
        CHECK(absPath.size() >= (fileName.size() + d.dirName().size() + 1));
        CHECK(std::equal(fileName.rbegin(), fileName.rend(), absPath.rbegin()));
    }

    TEST_CASE("checkCanCreateDir")
    {
        // GIVEN
        Dir d(TST_DIR "/new_test_dir");

        // THEN
        CHECK(!d.exists());

        // WHEN
        bool wasCreated = d.mkdir();

        // THEN
        CHECK(wasCreated);
        CHECK(d.exists());

        // WHEN
        wasCreated = d.mkdir();

        // THEN
        CHECK(!wasCreated);
    }

    TEST_CASE("checkCanRemoveDir")
    {
        // GIVEN
        Dir d(TST_DIR "/new_test_dir");

        // THEN
        CHECK(d.exists());

        // WHEN
        bool wasRemoved = d.rmdir();

        // THEN
        CHECK(wasRemoved);
        CHECK(!d.exists());

        // WHEN
        wasRemoved = d.rmdir();

        // THEN
        CHECK(!wasRemoved);
    }

    TEST_CASE("checkApplicationDir")
    {
        // WHEN
        const Dir appDir = Dir::applicationDir();

        // THEN
        CHECK(appDir == Dir(TST_DIR));
    }
}

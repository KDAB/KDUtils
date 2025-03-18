/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

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
            const Dir d("some_path_that_doesn_t_exist");

            // THEN
            CHECK(!d.exists());
        }
        {
            // GIVEN
            const Dir d(TST_DIR);

            // THEN
            CHECK(d.exists());
        }
    }

    TEST_CASE("checkDirName")
    {
        // GIVEN
        const Dir d(TST_DIR);

        // THEN
        CHECK(d.dirName() == "bin");
    }

    TEST_CASE("checkAbsoluteFilePath")
    {
        // GIVEN
        const Dir d(TST_DIR);

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
        CHECK(appDir == Dir(EXECUTABLE_DIR));
    }

    TEST_CASE("checkStripTrailingSeparator")
    {
        // GIVEN
        const Dir d(TST_DIR "/");

        // THEN
        CHECK(d.path() == TST_DIR);
    }

    TEST_CASE("checkParent")
    {
        // GIVEN
        const Dir d(TST_DIR "/subdir");

        // WHEN
        const Dir parent = d.parent();

        // THEN
        CHECK(parent.path() == TST_DIR);
        CHECK(parent.absoluteFilePath(d.dirName()) == d.path());
    }

    TEST_CASE("checkHasParent")
    {
        // GIVEN
        const Dir absoluteDir(TST_DIR "/subdir1");
        const Dir rootDir("/");
        const Dir relativeDir("subdir2");

        // THEN
        CHECK(absoluteDir.hasParent());
        CHECK(rootDir.hasParent() == false);
        CHECK(relativeDir.hasParent());
    }

    TEST_CASE("checkCanCreateWithParents")
    {
        // GIVEN
        Dir d(TST_DIR "/subdir1/subdir2");

        // THEN
        CHECK(!d.exists());
        CHECK(!d.parent().exists());

        // WHEN
        const auto wasCreatedAlone = d.mkdir({ false });

        // THEN
        CHECK(!wasCreatedAlone);
        CHECK(!d.exists());
        CHECK(!d.parent().exists());

        // WHEN
        const auto wasCreatedWithParent = d.mkdir({ true });

        // THEN
        CHECK(wasCreatedWithParent);
        CHECK(d.exists());
        CHECK(d.parent().exists());

        // cleanup
        CHECK(d.rmdir());
        CHECK(d.parent().rmdir());
    }

    TEST_CASE("checkMultibyteDirName")
    {
        // GIVEN
        Dir d(TST_DIR "/donkey 🫏 test");

        // THEN
        CHECK(!d.exists());

        // WHEN
        bool wasCreated = d.mkdir();

        // THEN
        CHECK(wasCreated);
        CHECK(d.exists());

        // WHEN
        bool wasRemoved = d.rmdir();

        // THEN
        CHECK(wasRemoved);
        CHECK(!d.exists());
    }
}

/*
    tst_dir.cpp

    This file is part of Kuesa.

    Copyright (C) 2018-2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Paul Lemire <paul.lemire@kdab.com>

    Licensees holding valid proprietary KDAB Kuesa licenses may use this file in
    accordance with the Kuesa Enterprise License Agreement provided with the Software in the
    LICENSE.KUESA.ENTERPRISE file.

    Contact info@kdab.com if any conditions of this licensing are not clear to you.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <KDCore/dir.h>

using namespace KDCore;

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

/*
    tst_file.cpp

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

#include <KDCore/file.h>
using namespace KDCore;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("File")
{

    TEST_CASE("init")
    {
        static_assert(std::is_destructible<File>{}, "File should be destructible");
        static_assert(!std::is_default_constructible<File>{}, "File should not be default constructible");
        static_assert(!std::is_copy_constructible<File>{}, "File should not be be copy constructible");
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
            File f(TST_DIR "tst_file");

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
            File f(TST_DIR "tst_file");
            const auto expectedSize = std::filesystem::file_size(std::filesystem::path(TST_DIR).append("tst_file"));

            // THEN
            CHECK(f.size() == expectedSize);
        }
    }

    TEST_CASE("checkFileName")
    {
        // GIVEN
        File f(TST_DIR "tst_file");

        // THEN
        CHECK(f.fileName() == "tst_file");
    }

    TEST_CASE("checkCanOpenAndReadExistingFile")
    {
        // GIVEN
        File f(TST_DIR "tst_file");

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

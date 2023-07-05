/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/file.h>
#include <KDUtils/file_mapper.h>
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

    TEST_CASE("tryMapNonexistentFile")
    {
        auto mapper = FileMapper(File("notreal.txt"));
        const uint8_t *data = mapper.map();
        CHECK(data == nullptr);
    }

    TEST_CASE("checkByteArrayAndMappingAreIdentical")
    {
        ByteArray copied;
        std::string path(TST_DIR "tst_file.cpp");
        File f(path);

        // copy bytes into memory
        f.open(std::ios::in | std::ios::binary);
        copied = f.readAll();
        f.close();

        // now map instead
        FileMapper mapped(std::move(f));
        // read-only map in this case by assigning to const pointer
        const uint8_t *mapped_data = mapped.map();
        const uint8_t *copied_data = copied.data();

        for (int i = 0; i < mapped.size(); i++) {
            CHECK(mapped_data[i] == copied_data[i]);
        }
    }

    TEST_CASE("checkMappingDoesNotModify")
    {
        std::string path(TST_DIR "tst_file.cpp");
        ByteArray readBefore;
        ByteArray readMapped;
        ByteArray readWriteMapped;
        ByteArray readAfter;

        {
            File f(path);
            f.open(std::ios::in | std::ios::binary);
            readBefore = f.readAll();
            f.close();
        }
        {
            // create a read-only mapping by declaring the mapper as const
            const auto mapper = FileMapper(File(path));
            const uint8_t *data = mapper.map();
            readMapped = ByteArray(data, mapper.size());
            CHECK(readMapped.size() == mapper.size());
        }
        {
            auto mapper = FileMapper(File(path));
            const uint8_t *data = mapper.map();
            readWriteMapped = ByteArray(data, mapper.size());
        }
        {
            File f(path);
            f.open(std::ios::in | std::ios::binary);
            readAfter = f.readAll();
            f.close();
        }

        // check that there was no modification before the read-only mapping occurred
        CHECK(readMapped.vector() == readBefore.vector());
        // no modification between read-only and writeable mappings
        CHECK(readMapped.vector() == readWriteMapped.vector());
        // no modification at any point
        CHECK(readBefore.vector() == readAfter.vector());
    }

    TEST_CASE("checkFileSizeMatchWithMapping")
    {
        std::string path(TST_DIR "tst_file.cpp");
        File f(path);
        size_t fileSize = f.size();
        auto mapper = FileMapper(File(path));
        mapper.map(); // just map it so there is a mapping to query the size of

        CHECK(mapper.size() == f.size());
    }

    TEST_CASE("mappedFileWriting")
    {
        std::string path(TST_DIR "tst_file.cpp");
        ByteArray copiedContents;
        ByteArray originalContents; // copy for restoring file state at the end
        {
            auto mapper = FileMapper(File(path));
            auto *mapping = mapper.map();
            copiedContents = ByteArray(mapping, mapper.size());
            originalContents = copiedContents;

            // perform identical write to both
            std::string intro = "hello, I am the new start to this file\n";
            CHECK(intro.size() < mapper.size()); // no overwrite
            std::memcpy(mapping, intro.data(), intro.size());
            std::memcpy(copiedContents.data(), intro.data(), intro.size());
        }

        // at this point, the intro string should be written to both
        // copiedContents and the file on disk

        // now get file contents to compare
        File f(path);
        f.open(std::ios::in | std::ios::binary);
        ByteArray fileContents = f.readAll();
        f.close();

        // main comparison of this test
        for (size_t i = 0; i < copiedContents.size(); i++) {
            CHECK(fileContents.data()[i] == copiedContents.data()[i]);
        }

        // we're done, now restore original file state
        {
            auto mapper = FileMapper(File(path));
            auto *mapping = mapper.map();
            std::memcpy(mapping, originalContents.data(), originalContents.size());
        }

        // make sure restore went correctly
        {
            f.open(std::ios::in | std::ios::binary);
            ByteArray restored = f.readAll();
            f.close();

            CHECK(restored.vector() == originalContents.vector());
        }
    }
}

/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDUtils/bytearray.h>
#include <cstring>

using namespace KDUtils;

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("ByteArray")
{

    TEST_CASE("init")
    {
        static_assert(std::is_destructible<ByteArray>{}, "ByteArray should be destructible");
        static_assert(std::is_default_constructible<ByteArray>{}, "ByteArray should be default constructible");
        static_assert(std::is_copy_constructible<ByteArray>{}, "ByteArray should be copy constructible");
        static_assert(std::is_copy_assignable<ByteArray>{}, "ByteArray should be copy assignable");
        static_assert(std::is_move_constructible<ByteArray>{}, "ByteArray should be move constructible");
        static_assert(std::is_move_assignable<ByteArray>{}, "ByteArray should be move assignable");
    }

    TEST_CASE("checkEmptyCtor")
    {
        // GIVEN
        const ByteArray b;

        // THEN
        CHECK(b.size() == size_t(0));
    }

    TEST_CASE("checkConstCharPtrCtor")
    {
        // GIVEN
        const char *s = "test";
        const ByteArray b(s);

        // THEN
        CHECK(b.size() == size_t(4));
        CHECK(std::strncmp(s, reinterpret_cast<const char *>(b.constData()), 4) == 0);

        // WHEN
        const ByteArray b2(s, 2);

        // THEN
        CHECK(b2.size() == size_t(2));
        CHECK(std::strncmp(s, reinterpret_cast<const char *>(b2.constData()), 2) == 0);
    }

    TEST_CASE("checkVectorCtor")
    {
        // GIVEN
        std::vector<uint8_t> rawData = { 0, 1, 3, 2 };
        const ByteArray b(rawData);

        // THEN
        CHECK(b.size() == size_t(4));
        CHECK(b.vector() == rawData);
    }

    TEST_CASE("checkSizeAndValueCtor")
    {
        // GIVEN
        const ByteArray b(4, 2);

        // THEN
        CHECK(b.size() == size_t(4));
        const std::vector<uint8_t> expectedData{ 2, 2, 2, 2 };
        CHECK(b.vector() == expectedData);
    }

    TEST_CASE("checkCopyCtor")
    {
        // GIVEN
        ByteArray b(4, 2);
        ByteArray b2(b);

        // THEN
        CHECK(b == b2);
    }

    TEST_CASE("checkMoveCtor")
    {
        // GIVEN
        const ByteArray b(4, 2);
        const ByteArray b2(b); // NOLINT(performance-unnecessary-copy-initialization)

        // THEN
        const std::vector<uint8_t> expectedData{ 2, 2, 2, 2 };
        CHECK(b2.vector() == expectedData);
    }

    TEST_CASE("checkCopyAssigmentOperator")
    {
        // GIVEN
        ByteArray b(4, 2);
        ByteArray b2 = b;

        // THEN
        CHECK(b == b2);
    }

    TEST_CASE("checkMoveAssigmentOperator")
    {
        // GIVEN
        ByteArray b(4, 2);
        const ByteArray b2 = std::move(b);

        // THEN
        const std::vector<uint8_t> expectedData{ 2, 2, 2, 2 };
        CHECK(b2.vector() == expectedData);
        CHECK(b.vector() == std::vector<uint8_t>());
    }

    TEST_CASE("checkSize")
    {
        // GIVEN
        const ByteArray b(4, 2);

        // THEN
        CHECK(b.size() == size_t(4));
    }

    TEST_CASE("checkReserve")
    {
        // GIVEN
        ByteArray b;

        // WHEN
        b.reserve(883);

        // THEN
        CHECK(b.size() == size_t(0));
    }

    TEST_CASE("checkResize")
    {
        // GIVEN
        ByteArray b;

        // WHEN
        b.resize(883);

        // THEN
        CHECK(b.size() == size_t(883));
    }

    TEST_CASE("checkData")
    {
        // GIVEN
        const char *s = "test";
        ByteArray b(s);

        // THEN
        CHECK(b.data()[0] == 't');
        CHECK(b.data()[1] == 'e');
        CHECK(b.data()[2] == 's');
        CHECK(b.data()[3] == 't');
    }

    TEST_CASE("checkConstData")
    {
        // GIVEN
        const char *s = "test";
        const ByteArray b(s);

        // THEN
        CHECK(b.constData()[0] == 't');
        CHECK(b.constData()[1] == 'e');
        CHECK(b.constData()[2] == 's');
        CHECK(b.constData()[3] == 't');
    }

    TEST_CASE("checkComparison")
    {
        // GIVEN
        ByteArray a("good");
        ByteArray b("bad");

        // THEN
        CHECK(a == a);
        CHECK(b == b);
        CHECK(a != b);
        CHECK(!(a == b));
    }

    TEST_CASE("checkStartsWith")
    {
        // GIVEN
        const ByteArray a("test");
        const ByteArray b("te");
        const ByteArray c("st");

        // THEN
        CHECK(a.startsWith(b));
        CHECK(!a.startsWith(c));
        CHECK(!b.startsWith(a));
    }

    TEST_CASE("checkEndsWith")
    {
        // GIVEN
        const ByteArray a("test");
        const ByteArray b("te");
        const ByteArray c("st");

        // THEN
        CHECK(a.endsWith(c));
        CHECK(!a.endsWith(b));
        CHECK(!c.endsWith(a));
    }

    TEST_CASE("checkMid")
    {
        // GIVEN
        ByteArray s("1234 good-apples");

        // WHEN
        const ByteArray m1 = s.mid(5, 4);

        // THEN
        CHECK(m1 == ByteArray("good"));

        // WHEN
        const ByteArray m2 = s.mid(5);

        // THEN
        CHECK(m2 == ByteArray("good-apples"));

        // WHEN
        const ByteArray m3 = s.mid(32);

        // THEN
        CHECK(m3.empty());

        // WHEN
        const ByteArray m4 = s.mid(0, 200);

        // THEN
        CHECK(m4 == s);
    }

    TEST_CASE("checkLeft")
    {
        // GIVEN
        ByteArray s("1234 good-apples");

        // WHEN
        const ByteArray l1 = s.left(4);

        // THEN
        CHECK(l1 == ByteArray("1234"));

        // WHEN
        const ByteArray l2 = s.left(200);

        // THEN
        CHECK(l2 == s);
    }

    TEST_CASE("checkClear")
    {
        // GIVEN
        ByteArray b(883);

        // THEN
        CHECK(!b.isEmpty());

        // WHEN
        b.clear();

        // THEN
        CHECK(b.isEmpty());
    }

    TEST_CASE("checkIndexOf")
    {
        // GIVEN
        const ByteArray b("hello");

        // WHEN
        int64_t iu = b.indexOf('u');

        // THEN
        CHECK(iu == -1);

        // WHEN
        int64_t il = b.indexOf('l');

        // THEN
        CHECK(il == 2);
    }

    TEST_CASE("checkRemove")
    {
        // GIVEN
        ByteArray b("chocolate");

        // WHEN
        b.remove(42, 44);

        // THEN -> Nothing
        CHECK(b == ByteArray("chocolate"));

        // WHEN
        b.remove(0, 5);

        // THEN
        CHECK(b == ByteArray("late"));

        // WHEN
        b.remove(0, 1);

        // THEN
        CHECK(b == ByteArray("ate"));

        // WHEN
        b.remove(2, 1);

        // THEN
        CHECK(b == ByteArray("at"));
    }

#ifdef BUILD_WITH_QT
    TEST_CASE("checkFromAndToQByteArray")
    {
        // GIVEN
        QByteArray qB("test");
        ByteArray b(qB);

        // THEN
        CHECK(b.toQByteArray() == qB);
    }

    TEST_CASE("checkAssignmentFromQByteArray")
    {
        // GIVEN
        QByteArray qB("test");
        ByteArray b = qB;

        // THEN
        CHECK(b.toQByteArray() == qB);
    }
#endif

    TEST_CASE("checkToBase64")
    {
        {
            // GIVEN
            const ByteArray t("");

            // WHEN
            const ByteArray b64 = t.toBase64();

            // THEN
            CHECK(b64 == ByteArray(""));
        }
        {
            // GIVEN
            const ByteArray t("f");

            // WHEN
            const ByteArray b64 = t.toBase64();

            // THEN
            CHECK(b64 == ByteArray("Zg=="));
        }
        {
            // GIVEN
            const ByteArray t("fo");

            // WHEN
            const ByteArray b64 = t.toBase64();

            // THEN
            CHECK(b64 == ByteArray("Zm8="));
        }
        {
            // GIVEN
            const ByteArray t("foo");

            // WHEN
            const ByteArray b64 = t.toBase64();

            // THEN
            CHECK(b64 == ByteArray("Zm9v"));
        }
    }

    TEST_CASE("checkFromBase64")
    {
        {
            // GIVEN
            const ByteArray b64("");

            // WHEN
            const ByteArray t = ByteArray::fromBase64(b64);

            // THEN
            CHECK(t == ByteArray(""));
        }
        {
            // GIVEN
            const ByteArray b64("Zg==");

            // WHEN
            const ByteArray t = ByteArray::fromBase64(b64);

            // THEN
            CHECK(t == ByteArray("f"));
        }
        {
            // GIVEN
            const ByteArray b64("Zm8=");

            // WHEN
            const ByteArray t = ByteArray::fromBase64(b64);

            // THEN
            CHECK(t == ByteArray("fo"));
        }
        {
            // GIVEN
            const ByteArray b64("Zm9v");

            // WHEN
            const ByteArray t = ByteArray::fromBase64(b64);

            // THEN
            CHECK(t == ByteArray("foo"));
        }
    }

    TEST_CASE("checkToAndFromBase64")
    {
        auto test = [](const std::vector<uint8_t> &data) {
            // GIVEN
            const ByteArray t(data);

            // WHEN
            const ByteArray b64 = t.toBase64();
            const ByteArray t2 = ByteArray::fromBase64(b64);

            // THEN
            CHECK(t == t2);
        };

        for (int32_t len = 1; len < 128; ++len) {
            std::vector<uint8_t> data;
            data.resize(len);

            for (int32_t j = 0; j < len; ++j) {
                for (int8_t c = 0; c < 127; ++c) {
                    data[j] = c;
                    test(data);
                }
            }
        }
    }

    TEST_CASE("checkOperatorPlusEqual")
    {
        // GIVEN
        ByteArray b("Bruce");

        // WHEN
        b += ByteArray("Willis");

        // THEN
        CHECK(b == ByteArray("BruceWillis"));
    }

    TEST_CASE("checkOperatorPlus")
    {
        // GIVEN
        const ByteArray b = ByteArray("Bruce") + ByteArray("Willis");

        // THEN
        CHECK(b == ByteArray("BruceWillis"));
    }
}

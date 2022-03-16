/*
    tst_flags.cpp

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

#include <KDUtils/flags.h>

using namespace KDUtils;

enum Enum {
    A = (1 << 0),
    B = (1 << 1),
    C = (1 << 2),
    D = A | C,
};
using EFlags = Flags<Enum>;
OPERATORS_FOR_FLAGS(EFlags);

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

TEST_SUITE("Flags")
{

    TEST_CASE("checkStorageType")
    {
        // GIVEN
        enum StandardEnum {
            A_SE = 0,
        };

        enum ClassIntEnum : int {
            A_CIE = 0,
        };

        enum ClassUIntEnum : uint {
            A_CUE = 0,
        };

        // THEN
        CHECK(std::is_signed<Flags<StandardEnum>::FlagsInt>() == std::is_signed<std::underlying_type<StandardEnum>::type>());
        CHECK(std::is_signed<Flags<ClassIntEnum>::FlagsInt>() == std::is_signed<std::underlying_type<ClassIntEnum>::type>());
        CHECK(std::is_signed<Flags<ClassUIntEnum>::FlagsInt>() == std::is_signed<std::underlying_type<ClassUIntEnum>::type>());
    }

    TEST_CASE("checkConstructors")
    {
        // GIVEN
        {
            // WHEN
            const Flags<Enum> f;

            // THEN
            CHECK(!f);
        }

        {
            // WHEN
            const Flags<Enum> f(B);

            // THEN
            CHECK(f);
            CHECK(f.testFlag(B));
        }

        {
            // WHEN
            const Flags<Enum> f = Flags<Enum>::fromInt(4);

            // THEN
            CHECK(f);
            CHECK(f.testFlag(C));
        }

        {
            // WHEN
            const Flags<Enum> f = Flags<Enum>::fromInt(4U);

            // THEN
            CHECK(f);
            CHECK(f.testFlag(C));
        }
    }

    TEST_CASE("checkOperatorBool")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);

        // THEN
        CHECK(!a);
        CHECK(b);
    }

    TEST_CASE("checkOperatorAndEqual")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // WHEN
        a &= b;

        // THEN
        CHECK(!a);

        // WHEN
        b &= c;

        // THEN
        CHECK(b);

        // WHEN
        b &= a;

        // THEN
        CHECK(!b);
    }

    TEST_CASE("checkOperatorAnd")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // WHEN
        const Flags<Enum> aAndB = a & b;

        // THEN
        CHECK(!aAndB);

        // WHEN
        const Flags<Enum> bAndC = b & c;

        // THEN
        CHECK(bAndC);

        // WHEN
        const Flags<Enum> bAndA = b & a;

        // THEN
        CHECK(!bAndA);
    }

    TEST_CASE("checkOperatorOrEqual")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // THEN
        CHECK(a != b);
        CHECK(b == c);

        // WHEN
        a |= b;

        // THEN
        CHECK(a);
        CHECK(a == b);

        // WHEN
        b |= c;

        // THEN
        CHECK(b);
        CHECK(b == c);
    }

    TEST_CASE("checkOperatorOr")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // WHEN
        const Flags<Enum> aOrB = a | b;

        // THEN
        CHECK(aOrB);
        CHECK(aOrB == b);

        // WHEN
        const Flags<Enum> bOrC = b | c;

        // THEN
        CHECK(bOrC);
        CHECK(bOrC == c);
    }

    TEST_CASE("checkOperatorXOrEqual")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // THEN
        CHECK(a != b);
        CHECK(b == c);

        // WHEN
        a ^= b;

        // THEN
        CHECK(a);
        CHECK(a == b);

        // WHEN
        b ^= c;

        // THEN
        CHECK(!b);
        CHECK(b != c);
    }

    TEST_CASE("checkOperatorXOr")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(A);
        Flags<Enum> c(A);

        // WHEN
        const Flags<Enum> aXorB = a ^ b;

        // THEN
        CHECK(aXorB);
        CHECK(aXorB == b);

        const Flags<Enum> cXorA = c ^ a;

        // THEN
        CHECK(cXorA);
        CHECK(cXorA == c);

        // WHEN
        const Flags<Enum> bXorC = b ^ c;

        // THEN
        CHECK(!bXorC);
        CHECK(bXorC != c);
    }

    TEST_CASE("checkOperatorTilde")
    {
        // GIVEN
        Flags<Enum> a(D);

        // THEN
        CHECK(a.testFlag(A));
        CHECK(a.testFlag(C));
        CHECK(a.testFlag(D));
        CHECK(!a.testFlag(B));

        // WHEN
        a = ~a;

        // THEN
        CHECK(!a.testFlag(A));
        CHECK(!a.testFlag(C));
        CHECK(!a.testFlag(D));
        CHECK(a.testFlag(B));
    }

    TEST_CASE("checkSetFlag")
    {
        // GIVEN
        Flags<Enum> a;

        // WHEN
        a.setFlag(B, true);

        // THEN
        CHECK(a);
        CHECK(a.testFlag(B));

        // WHEN
        a.setFlag(B, false);

        // THEN
        CHECK(!a);
        CHECK(!a.testFlag(B));
    }

    TEST_CASE("checkStandaloneOperatorOr")
    {
        // GIVEN
        Flags<Enum> a;

        // WHEN
        a = A | B | C;

        // THEN
        CHECK(a.testFlag(A));
        CHECK(a.testFlag(B));
        CHECK(a.testFlag(C));
    }

    TEST_CASE("checkStandaloneOperatorAnd")
    {
        // GIVEN
        Flags<Enum> a;

        // WHEN
        a = A & D;

        // THEN
        CHECK(a.testFlag(A));
        CHECK(!a.testFlag(D));
        CHECK(!a.testFlag(C));
    }

    TEST_CASE("checkComparisonOperators")
    {
        // GIVEN
        Flags<Enum> a(A);
        Flags<Enum> b(B);
        Flags<Enum> c(C);

        // THEN
        CHECK(a != b);
        CHECK(b != c);
        CHECK(c != a);
        CHECK(a == Flags<Enum>(A));

        CHECK(a == A);
        CHECK(A == a);
        CHECK(B == b);
        CHECK(b == B);
        CHECK(C == c);
        CHECK(c == C);

        CHECK(a != B);
        CHECK(B != a);
        CHECK(c != B);
        CHECK(C != b);
    }

    TEST_CASE("checkExplicitBool")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> b(B);

        // THEN
        CHECK(!a);
        CHECK(bool(b));
    }

    TEST_CASE("checkToAndFromInt")
    {
        // GIVEN
        Flags<Enum> a;
        Flags<Enum> d(D);
        using Int = Flags<Enum>::FlagsInt;

        // WHEN
        a = Flags<Enum>::fromInt(Int(A) | Int(C));

        // THEN
        CHECK(a == d);
        CHECK(a.toInt() == (Int(A) | Int(C)));
    }
}

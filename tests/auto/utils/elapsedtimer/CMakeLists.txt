# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: MIT
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

add_executable(
    tst_elapsedtimer
    tst_elapsedtimer.cpp
)
target_link_libraries(
    tst_elapsedtimer
    PRIVATE KDUtils doctest::doctest
)
get_filename_component(ASSETS_ELAPSEDTIMER ../assets/ ABSOLUTE)
target_compile_definitions(tst_elapsedtimer PRIVATE ASSETS="${ASSETS_ELAPSEDTIMER}/")
doctest_discover_tests(
    tst_elapsedtimer
    ADD_LABELS
    1
    PROPERTIES
    LABELS
    "KDUtils"
)

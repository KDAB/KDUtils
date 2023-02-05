# This file is part of KDUtils.
#
# SPDX-FileCopyrightText: 2021-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Paul Lemire <paul.lemire@kdab.com>
#
# SPDX-License-Identifier: AGPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#

project(test-core-object VERSION 0.1 LANGUAGES CXX)

add_core_test(${PROJECT_NAME}
    tst_object.cpp
)

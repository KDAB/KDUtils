/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/destruction_helpers.h>
#include <KDFoundation/object.h>
#include <signal_spy.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

class Node : public Object
{
public:
    Node()
    {
    }

    // NOLINTBEGIN(bugprone-exception-escape)
    ~Node()
    {
        destroyed.emit(this);
    }
    // NOLINTEND(bugprone-exception-escape)

    KDBindings::Signal<Node *> destroyed;
    KDBindings::Property<Node *> prop{ nullptr };
};

TEST_SUITE("DesctructionHelper")
{
    TEST_CASE("registerDestructionHelper Container & Dependency")
    {
        // GIVEN
        KDFoundation::DestructionHelperManager<Node, Node> destructionManager;
        Node container;
        int dependencyDestroyedNotificationCount = 0;

        // WHEN
        {
            Node dependency;
            registerDestructionHelper(&container, &dependency, &destructionManager, [&]() {
                ++dependencyDestroyedNotificationCount;
            });
        }

        // THEN
        CHECK(dependencyDestroyedNotificationCount == 1);
    }

    TEST_CASE("unregisterDestructionHelper Container & Dependency")
    {
        // GIVEN
        KDFoundation::DestructionHelperManager<Node, Node> destructionManager;
        Node container;
        int dependencyDestroyedNotificationCount = 0;

        // WHEN
        {
            Node dependency;
            registerDestructionHelper(&container, &dependency, &destructionManager, [&]() {
                ++dependencyDestroyedNotificationCount;
            });
            unregisterDestructionHelper(&container, &dependency, &destructionManager);
        }

        // THEN
        CHECK(dependencyDestroyedNotificationCount == 0);
    }

    TEST_CASE("registerDestructionHelper Property")
    {
        // GIVEN
        KDBindings::Property<Node *> dependencyProp{ nullptr };
        KDBindings::ConnectionHandle connection;

        // WHEN
        {
            Node dependency;
            registerDestructionHelper(&dependencyProp, &connection, static_cast<Node *>(nullptr));
            dependencyProp = &dependency;

            // THEN
            CHECK(dependencyProp() == &dependency);
            CHECK(connection.isActive());
        }

        // cppcheck-suppress invalidLifetime
        CHECK(dependencyProp() == nullptr);
        CHECK(!connection.isActive());
    }

    TEST_CASE("registerDestructionHelper Property & Owner")
    {
        // GIVEN
        KDBindings::ConnectionHandle connection;

        // WHEN
        {
            Node container;

            {
                Node dependency;

                // WHEN
                registerDestructionHelper(&container.prop, &connection, &container);
                container.prop = &dependency;

                // THEN -> Dependency is watched
                CHECK(container.prop() == &dependency);
                CHECK(connection.isActive());
            }

            // THEN -> Property was reset since watched node was destroyed
            CHECK(container.prop() == nullptr);
            CHECK(!connection.isActive());
        }

        // THEN -> Connectio resets since owner destroyed
        CHECK(!connection.isActive());
    }
}

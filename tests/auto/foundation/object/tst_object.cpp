/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "KDFoundation/core_application.h"
#include <KDFoundation/object.h>
#include <KDFoundation/event.h>
#include <KDFoundation/kdfoundation_global.h>
#include <signal_spy.h>

#include <kdbindings/signal.h>

#include <numeric>
#include <string>
#include <tuple>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

using namespace KDFoundation;

static_assert(std::is_destructible<Object>{});
static_assert(std::is_default_constructible<Object>{});
static_assert(!std::is_copy_constructible<Object>{});
static_assert(!std::is_copy_assignable<Object>{});
static_assert(std::is_nothrow_move_constructible<Object>{});
static_assert(std::is_nothrow_move_assignable<Object>{});

class IntObject : public Object
{
public:
    IntObject(int value)
        : Object()
        , m_value{ value }
    {
    }

    ~IntObject()
    {
        aboutToBeDestroyed.emit(m_value);
    }

    int value() const { return m_value; }

    KDBindings::Signal<int> aboutToBeDestroyed;

private:
    int m_value;
};

class CountedObject : public Object
{
public:
    CountedObject()
        : Object()
    {
        ++ms_objectCount;
    }

    ~CountedObject()
    {
        --ms_objectCount;
    }

    static int ms_objectCount;
};

int CountedObject::ms_objectCount = 0;

class EventObject : public Object
{
public:
    bool m_userEventDelivered = false;
    bool m_timerEventDelivered = false;

protected:
    void timerEvent(TimerEvent *ev) override
    {
        m_timerEventDelivered = true;
    }

    void userEvent(Event *ev) override
    {
        m_userEventDelivered = true;
    }
};

class UserEvent : public Event
{
public:
    UserEvent()
        : Event(static_cast<Event::Type>(static_cast<uint16_t>(Event::Type::UserType) + 1))
    {
    }
};

TEST_CASE("Object construction")
{
    SUBCASE("can create an object with no parent")
    {
        auto obj = std::make_unique<Object>();

        REQUIRE(obj.get() != nullptr);
        REQUIRE(obj->children().empty() == true);
        REQUIRE(obj->parent() == nullptr);
    }

    SUBCASE("can create an object with a parent")
    {
        // GIVEN
        auto parent = std::make_unique<Object>();
        SignalSpy<Object *, Object *> childAddedSpy(parent->childAdded);

        // THEN
        REQUIRE(childAddedSpy.count() == 0);

        // WHEN
        auto child = parent->createChild<Object>();

        // THEN
        REQUIRE(child != nullptr);
        REQUIRE(child->parent() == parent.get());
        REQUIRE(parent->children().size() == 1);
        REQUIRE(childAddedSpy.count() == 1);
        REQUIRE(std::get<0>(childAddedSpy.args()) == parent.get());
        REQUIRE(std::get<1>(childAddedSpy.args()) == child);
    }

    SUBCASE("can create a subclass of Object taking an argument")
    {
        auto obj = std::make_unique<IntObject>(7);

        REQUIRE(obj.get() != nullptr);
        REQUIRE(obj->children().empty() == true);
        REQUIRE(obj->parent() == nullptr);
        REQUIRE(obj->value() == 7);
    }

    SUBCASE("can create a child of an Object that is of a subclass of Object type")
    {
        // GIVEN
        auto parent = std::make_unique<Object>();
        SignalSpy<Object *, Object *> childAddedSpy(parent->childAdded);

        // THEN
        REQUIRE(childAddedSpy.count() == 0);

        // WHEN
        auto child = parent->createChild<IntObject>(42);

        // THEN
        REQUIRE(child != nullptr);
        REQUIRE(child->parent() == parent.get());
        REQUIRE(parent->children().size() == 1);
        REQUIRE(child->value() == 42);
        REQUIRE(childAddedSpy.count() == 1);
        REQUIRE(std::get<0>(childAddedSpy.args()) == parent.get());
        REQUIRE(std::get<1>(childAddedSpy.args()) == child);
    }
}

TEST_CASE("Parenting")
{
    SUBCASE("can add a parentless object as a child")
    {
        // GIVEN
        auto parent = std::make_unique<Object>();
        auto child = std::make_unique<Object>();
        SignalSpy<Object *, Object *> childAddedSpy(parent->childAdded);
        SignalSpy<Object *, Object *> childParentChangedSpy(child->parentChanged);

        // THEN
        REQUIRE(childAddedSpy.count() == 0);
        REQUIRE(childParentChangedSpy.count() == 0);

        // WHEN
        auto adoptedChild = parent->addChild(std::move(child));

        // THEN
        REQUIRE(adoptedChild != nullptr);
        REQUIRE(adoptedChild->parent() == parent.get());
        REQUIRE(parent->children().size() == 1);
        REQUIRE(childAddedSpy.count() == 1);
        REQUIRE(std::get<0>(childAddedSpy.args()) == parent.get());
        REQUIRE(std::get<1>(childAddedSpy.args()) == adoptedChild);
        REQUIRE(childParentChangedSpy.count() == 1);
        REQUIRE(std::get<0>(childParentChangedSpy.args()) == adoptedChild);
        REQUIRE(std::get<1>(childParentChangedSpy.args()) == parent.get());
    }

    SUBCASE("can reparent a child")
    {
        // GIVEN
        auto oldParent = std::make_unique<Object>();
        auto child = oldParent->createChild<Object>();
        auto newParent = std::make_unique<Object>();
        SignalSpy<Object *, Object *> childAddedSpy(newParent->childAdded);
        SignalSpy<Object *, Object *> childRemovedSpy(oldParent->childRemoved);
        SignalSpy<Object *, Object *> childParentChangedSpy(child->parentChanged);

        // THEN
        REQUIRE(childAddedSpy.count() == 0);
        REQUIRE(childRemovedSpy.count() == 0);
        REQUIRE(childParentChangedSpy.count() == 0);

        // WHEN
        newParent->addChild(oldParent->takeChild(child));

        // THEN
        REQUIRE(childParentChangedSpy.count() == 2); // set to null when taken and then set to newParent when added as a child
        REQUIRE(std::get<0>(childParentChangedSpy.args()) == child);
        REQUIRE(std::get<1>(childParentChangedSpy.args()) == newParent.get());
        REQUIRE(oldParent->children().empty() == true);
        REQUIRE(newParent->children().size() == 1);
        REQUIRE(child->parent() == newParent.get());
        REQUIRE(childAddedSpy.count() == 1);
        REQUIRE(std::get<0>(childAddedSpy.args()) == newParent.get());
        REQUIRE(std::get<1>(childAddedSpy.args()) == child);
        REQUIRE(childRemovedSpy.count() == 1);
        REQUIRE(std::get<0>(childRemovedSpy.args()) == oldParent.get());
        REQUIRE(std::get<1>(childRemovedSpy.args()) == child);
    }

    SUBCASE("parent can have many children")
    {
        // GIVEN
        auto parent = std::make_unique<Object>();
        std::vector<Object *> children;
        SignalSpy childAddedSpy(parent->childAdded);

        // THEN
        REQUIRE(childAddedSpy.count() == 0);

        // WHEN
        for (int i = 0; i < 10; ++i) {
            auto child = parent->createChild<Object>();
            children.push_back(child);
        }

        // THEN
        REQUIRE(childAddedSpy.count() == 10);
        REQUIRE(parent->children().size() == children.size());
        const auto &parentChildren = parent->children();
        for (int i = 0; i < children.size(); ++i) {
            auto c = parentChildren[i].get();
            REQUIRE(children[i] == c);
            REQUIRE(c->parent() == parent.get());
        }
    }
}

TEST_CASE("Object destruction")
{
    SUBCASE("can destroy a parentless and childless object")
    {
        auto obj = new Object();
        delete obj;
    }

    SUBCASE("can destroy a parentless object that has children")
    {
        auto parent = new Object();
        for (int i = 0; i < 10; ++i)
            parent->createChild<CountedObject>();

        REQUIRE(CountedObject::ms_objectCount == 10);

        delete parent;
        REQUIRE(CountedObject::ms_objectCount == 0);
    }

    SUBCASE("children are destroyed in LIFO order")
    {
        // GIVEN
        auto parent = new Object();
        SignalSpy childRemovedSpy(parent->childRemoved);

        std::vector<int> destructionOrder;
        auto onAboutToBeDestroyed = [&destructionOrder](const int &value) {
            destructionOrder.push_back(value);
        };

        for (int i = 0; i < 10; ++i) {
            auto child = parent->createChild<IntObject>(i);
            std::ignore = child->aboutToBeDestroyed.connect(onAboutToBeDestroyed);
        }
        // THEN
        REQUIRE(childRemovedSpy.count() == 0);

        // WHEN
        delete parent;

        // THEN
        REQUIRE(childRemovedSpy.count() == 10);
        std::vector<int> expectedDestructionOrder(10);
        std::iota(expectedDestructionOrder.rbegin(), expectedDestructionOrder.rend(), 0);
        REQUIRE(destructionOrder.size() == 10);
        for (int i = 0; i < destructionOrder.size(); ++i) {
            REQUIRE(expectedDestructionOrder[i] == destructionOrder[i]);
        }
    }

    SUBCASE("destroyed signal is emitted on object destruction")
    {
        auto obj = new Object();
        SignalSpy<Object *> objectDestroyedSpy(obj->destroyed);
        delete obj;
        REQUIRE(objectDestroyedSpy.count() == 1);
        REQUIRE(std::get<0>(objectDestroyedSpy.args()) == obj);
    }
}

TEST_CASE("Deferred object destruction")
{

    SUBCASE("can call deleteLater on object with no CoreApplication instantiated")
    {
        auto obj = new Object();
        obj->deleteLater();
    }

    SUBCASE("can call deleteLater on object with CoreApplication instantiated")
    {
        CoreApplication app;
        auto obj = new Object();
        obj->deleteLater();
    }

    SUBCASE("can call deleteLater multiple times without crash when control returns to event loop")
    {
        CoreApplication app;
        auto obj = new Object();

        obj->deleteLater();
        obj->deleteLater();
        obj->deleteLater();

        app.processEvents();
    }

    SUBCASE("object is deleted when control returns to event loop")
    {
        CoreApplication app;
        auto obj = new Object();

        SignalSpy<Object *> objectDestroyedSpy(obj->destroyed);
        obj->deleteLater();

        REQUIRE(objectDestroyedSpy.count() == 0);
        app.processEvents();
        REQUIRE(objectDestroyedSpy.count() == 1);
        REQUIRE(std::get<0>(objectDestroyedSpy.args()) == obj);
    }

    SUBCASE("object is deleted when CoreApplication is deleted before control returns to event loop")
    {
        auto *app = new CoreApplication();
        auto obj = new Object();

        SignalSpy<Object *> objectDestroyedSpy(obj->destroyed);
        obj->deleteLater();

        REQUIRE(objectDestroyedSpy.count() == 0);
        delete app;
        REQUIRE(objectDestroyedSpy.count() == 1);
        REQUIRE(std::get<0>(objectDestroyedSpy.args()) == obj);
    }
}

TEST_CASE("Event delivery")
{
    SUBCASE("custom user event gets delivered to userEvent virtual function")
    {
        auto ev = std::make_unique<UserEvent>();
        auto object = std::make_unique<EventObject>();
        object->event(object.get(), ev.get());

        REQUIRE(object->m_userEventDelivered == true);
    }

    SUBCASE("timer event gets delivered to timerEvent virtual function")
    {
        auto ev = std::make_unique<TimerEvent>();
        auto object = std::make_unique<EventObject>();
        object->event(object.get(), ev.get());

        REQUIRE(object->m_timerEventDelivered == true);
    }
}

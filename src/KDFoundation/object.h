/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/event_receiver.h>

#include <memory>
#include <string>
#include <vector>

#include <kdbindings/signal.h>

namespace KDFoundation {

class Event;
class TimerEvent;

class KDFOUNDATION_API Object : public EventReceiver
{
public:
    Object();
    /// @warning This destructor emits #destroyed signal and #childRemoved for each child. If any of
    /// the slots connected to these signals throws (which is UB in KDBindings anyway) the exception
    /// will be eaten and error will be logged.
    virtual ~Object();

    // Not copyable
    Object(Object const &other) = delete;
    Object &operator=(Object const &other) = delete;

    // Is movable
    Object(Object &&other) noexcept = default;
    Object &operator=(Object &&other) noexcept = default;

    const Object *parent() const { return m_parent; }
    Object *parent() { return m_parent; }

    const std::vector<std::unique_ptr<Object>> &children() const { return m_children; }

    template<typename T>
    Object *addChild(std::unique_ptr<T> &&child)
    {
        // Caller has to transfer ownership to us so there should not be an old parent
        assert(child->parent() == nullptr);

        child->m_parent = this;
        m_children.push_back(std::move(child));
        Object *childPtr = m_children.back().get();
        childPtr->parentChanged.emit(childPtr, childPtr->m_parent);
        childAdded.emit(this, childPtr);
        return childPtr;
    }

    template<typename T, typename... Ts>
    T *createChild(Ts... args)
    {
        auto child = std::make_unique<T>(std::forward<Ts>(args)...);
        return static_cast<T *>(this->addChild(std::move(child)));
    }

    template<typename T>
    std::unique_ptr<Object> takeChild(T *child)
    {
        // Find the child from the raw pointer
        auto childIt = std::find_if(
                m_children.begin(),
                m_children.end(),
                [child](const auto &v) {
                    return v.get() == child;
                });

        // Didn't find a matching child?
        if (childIt == m_children.end())
            return {};

        // Unparent the child and return it along with ownership!
        auto takenChild = std::move(*childIt);
        takenChild->m_parent = nullptr;
        takenChild->parentChanged.emit(takenChild.get(), takenChild->m_parent);
        m_children.erase(childIt);
        childRemoved.emit(this, takenChild.get());
        return takenChild;
    }

    void deleteLater();

    void setObjectName(const std::string &objectName) { m_objectName = objectName; }
    const std::string &objectName() const { return m_objectName; }

    void event(EventReceiver *target, Event *ev) override;

    KDBindings::Signal<Object *, Object *> parentChanged;
    KDBindings::Signal<Object *, Object *> childAdded;
    KDBindings::Signal<Object *, Object *> childRemoved;
    KDBindings::Signal<Object *> destroyed;

protected:
    virtual void timerEvent(TimerEvent *ev);
    virtual void userEvent(Event *ev);

private:
    Object *m_parent{ nullptr };
    std::vector<std::unique_ptr<Object>> m_children;
    std::string m_objectName;
};

} // namespace KDFoundation

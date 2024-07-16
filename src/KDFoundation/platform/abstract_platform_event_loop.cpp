/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Miłosz Kosobucki <milosz.kosobucki@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDFoundation/platform/abstract_platform_event_loop.h>

namespace KDFoundation {

/**
 * KDUtils-specific connection evaluator that wakes up the owning event loop when slot invocation is
 * enqueued via the loop's connection evaluator.
 */
class ConnectionEvaluator final : public KDBindings::ConnectionEvaluator
{
public:
    ConnectionEvaluator(AbstractPlatformEventLoop *eventLoop)
        : m_eventLoop(eventLoop) { }

    void setEventLoop(AbstractPlatformEventLoop *eventLoop)
    {
        this->m_eventLoop = eventLoop;
    }

protected:
    void onInvocationAdded() override
    {
        if (m_eventLoop) {
            m_eventLoop->wakeUp();
        }
    }

private:
    AbstractPlatformEventLoop *m_eventLoop = nullptr;
};
} // namespace KDFoundation

using namespace KDFoundation;

KDFoundation::AbstractPlatformEventLoop::AbstractPlatformEventLoop()
    : m_connectionEvaluator(new ConnectionEvaluator(this))
{
}

AbstractPlatformEventLoop::~AbstractPlatformEventLoop()
{
    // Make sure that any remaining references to our connection evaluator aren't referring to a dead
    // event loop object.
    if (m_connectionEvaluator) {
        std::static_pointer_cast<KDFoundation::ConnectionEvaluator>(m_connectionEvaluator)->setEventLoop(nullptr);
    }
}

void KDFoundation::AbstractPlatformEventLoop::waitForEvents(int timeout)
{
    waitForEventsImpl(timeout);

    // Possibly we woke up because of deferred slot invocation was posted. Let's (possibly)
    // execute them while we're at it.
    if (m_connectionEvaluator) {
        m_connectionEvaluator->evaluateDeferredConnections();
    }
}

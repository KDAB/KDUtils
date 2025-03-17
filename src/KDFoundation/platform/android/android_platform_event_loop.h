/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/kdfoundation_global.h>
#include <KDFoundation/platform/abstract_platform_event_loop.h>

#include <kdbindings/signal.h>

#include <unordered_map>

#ifdef __cplusplus
extern "C" {
#endif
struct android_app;
struct android_poll_source;
struct AInputEvent;
#ifdef __cplusplus
}
#endif

namespace KDFoundation {

class AbstractPlatformIntegration;
class KDFOUNDATION_API AndroidPlatformEventLoop : public AbstractPlatformEventLoop
{
public:
    AndroidPlatformEventLoop(AbstractPlatformIntegration *platformInitegration);

    // Kick the event loop out of waiting
    void wakeUp() override;

    bool registerNotifier(FileDescriptorNotifier *notifier) override;

    bool unregisterNotifier(FileDescriptorNotifier *notifier) override;

    AbstractPlatformIntegration *androidPlatformIntegration();

    KDBindings::Signal<> windowResize;
    KDBindings::Signal<uint32_t, uint8_t, int32_t, int32_t> keyEvent;
    KDBindings::Signal<int32_t, int64_t, int64_t, int64_t> touchEvent;

protected:
    void waitForEventsImpl(int timeout) override;
    std::unique_ptr<AbstractPlatformTimer> createPlatformTimerImpl(Timer *timer) final;

private:
    static int ALooperCallback(int fd, int events, void *data);
    struct NotifierSet {
        int androidEvents() const;

        std::array<FileDescriptorNotifier *, 3> events{ nullptr, nullptr, nullptr };

        bool isEmpty() const;
    };
    static void androidHandleCmd(android_app *app, int32_t cmd);
    static int32_t androidHandleInputEvent(struct android_app *app, AInputEvent *event);

private:
    std::unordered_map<int, NotifierSet> m_notifiers;
    AbstractPlatformIntegration *m_androidPlatformInitegration;
};

} // namespace KDFoundation

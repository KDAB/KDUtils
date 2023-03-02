/*
  This file is part of KDUtils.

SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
Author: BogDan Vatra <bogdan@kdab.com>

SPDX-License-Identifier: AGPL-3.0-only

Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDGui/kdgui_global.h>

#include <array>
#include <memory>
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

namespace KDGui {

class AndroidPlatformIntegration;
class KDGUI_API AndroidPlatformEventLoop : public KDFoundation::AbstractPlatformEventLoop
{
public:
    AndroidPlatformEventLoop(AndroidPlatformIntegration *androidPlatformInitegration);

    // timeout in msecs
    // -1 means wait forever
    // 0 means do not wait (i.e. poll)
    // +ve number, wait for up to timeout msecs
    void waitForEvents(int timeout) override;

    // Kick the event loop out of waiting
    void wakeUp() override;

    bool registerNotifier(KDFoundation::FileDescriptorNotifier *notifier) override;

    bool unregisterNotifier(KDFoundation::FileDescriptorNotifier *notifier) override;

    AndroidPlatformIntegration *androidPlatformIntegration();

protected:
    std::unique_ptr<KDFoundation::AbstractPlatformTimer> createPlatformTimerImpl(KDFoundation::Timer *timer) final;

private:
    static int ALooperCallback(int fd, int events, void* data);
    struct NotifierSet {
        int androidEvents() const;

        std::array<KDFoundation::FileDescriptorNotifier *, 3> events{ nullptr, nullptr, nullptr };

        bool isEmpty() const;
    };
    static void androidHandleCmd(android_app* app, int32_t cmd);
    static int32_t androidHandleInputEvent(struct android_app* app, AInputEvent* event);

private:
    int m_events;
    std::unordered_map<int, NotifierSet> m_notifiers;
    AndroidPlatformIntegration *m_androidPlatformInitegration;
};

} // namespace KDGui

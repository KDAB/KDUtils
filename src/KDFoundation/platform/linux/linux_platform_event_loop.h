/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/file_descriptor_notifier.h>

#include <KDFoundation/logging.h>

#include <array>
#include <cstdint>
#include <map>

#include <sys/epoll.h>

namespace KDFoundation {

class KDFOUNDATION_API LinuxPlatformEventLoop : public AbstractPlatformEventLoop
{
public:
    LinuxPlatformEventLoop();
    ~LinuxPlatformEventLoop() override;

    LinuxPlatformEventLoop(LinuxPlatformEventLoop const &other) = delete;
    LinuxPlatformEventLoop &operator=(LinuxPlatformEventLoop const &other) = delete;
    LinuxPlatformEventLoop(LinuxPlatformEventLoop &&other) = delete;
    LinuxPlatformEventLoop &operator=(LinuxPlatformEventLoop &&other) = delete;

    void waitForEvents(int timeout) override;
    void wakeUp() override;

    bool registerNotifier(FileDescriptorNotifier *notifier) override;
    bool unregisterNotifier(FileDescriptorNotifier *notifier) override;

    int epollHandle() { return m_epollHandle; }

    bool registerFileDescriptor(int fd, FileDescriptorNotifier::NotificationType type);
    bool unregisterFileDescriptor(int fd, FileDescriptorNotifier::NotificationType type);

    size_t registeredFileDescriptorCount() const { return m_notifiers.size(); }

    int epollEventFromFdPlusType(int fd, FileDescriptorNotifier::NotificationType type);
    int epollEventFromFdMinusType(int fd, FileDescriptorNotifier::NotificationType type);
    int epollEventFromNotifierTypes(bool read, bool write, bool exception);

private:
    std::unique_ptr<AbstractPlatformTimer> createPlatformTimerImpl(Timer *timer) override;

    int m_epollHandle = -1;
    int m_eventfd = -1;

    struct NotifierSet {
        std::array<FileDescriptorNotifier *, 3> events{ nullptr, nullptr, nullptr };

        bool isEmpty() const
        {
            return events[0] == nullptr && events[1] == nullptr && events[2] == nullptr;
        }

        bool wouldBeEmptyIfUnset(FileDescriptorNotifier::NotificationType type)
        {
            switch (type) {
            case FileDescriptorNotifier::NotificationType::Read:
                return events[1] == nullptr && events[2] == nullptr;
            case FileDescriptorNotifier::NotificationType::Write:
                return events[0] == nullptr && events[3] == nullptr;
            case FileDescriptorNotifier::NotificationType::Exception:
                return events[0] == nullptr && events[1] == nullptr;
            }
            spdlog::critical("Error in determining if notifier set is empty");
            return false;
        }
    };
    std::map<int, NotifierSet> m_notifiers;
};

} // namespace KDFoundation

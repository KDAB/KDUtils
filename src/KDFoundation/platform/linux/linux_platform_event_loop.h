/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/platform/abstract_platform_event_loop.h>
#include <KDFoundation/file_descriptor_notifier.h>

#include <KDUtils/logging.h>

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

protected:
    void waitForEventsImpl(int timeout) override;

private:
    std::unique_ptr<AbstractPlatformTimer> createPlatformTimerImpl(Timer *timer) override;

    int m_epollHandle = -1;
    int m_eventfd = -1;

    struct NotifierSet {
        bool isEmpty() const
        {
            return !hasNotifier(FileDescriptorNotifier::NotificationType::Read) &&
                    !hasNotifier(FileDescriptorNotifier::NotificationType::Write) &&
                    !hasNotifier(FileDescriptorNotifier::NotificationType::Exception);
        }

        bool wouldBeEmptyIfUnset(FileDescriptorNotifier::NotificationType type) const
        {
            switch (type) {
            case FileDescriptorNotifier::NotificationType::Read:
                return !hasNotifier(FileDescriptorNotifier::NotificationType::Write) && !hasNotifier(FileDescriptorNotifier::NotificationType::Exception);
            case FileDescriptorNotifier::NotificationType::Write:
                return !hasNotifier(FileDescriptorNotifier::NotificationType::Read) && !hasNotifier(FileDescriptorNotifier::NotificationType::Exception);
            case FileDescriptorNotifier::NotificationType::Exception:
                return !hasNotifier(FileDescriptorNotifier::NotificationType::Read) && !hasNotifier(FileDescriptorNotifier::NotificationType::Write);
            }
            SPDLOG_CRITICAL("Error in determining if notifier set is empty");
            return false;
        }

        bool hasNotifier(const FileDescriptorNotifier::NotificationType type) const
        {
            const auto notifier = getNotifier(type);
            return (notifier != nullptr);
        }

        FileDescriptorNotifier *getNotifier(const FileDescriptorNotifier::NotificationType type) const
        {
            const auto index = eventIndexByNotificationType(type);
            return events[index];
        }

        void resetNotifier(FileDescriptorNotifier::NotificationType type)
        {
            setNotifier(type, nullptr);
        }

        void setNotifier(FileDescriptorNotifier::NotificationType type, FileDescriptorNotifier *fdn)
        {
            const auto index = eventIndexByNotificationType(type);
            events[index] = fdn;
        }

    private:
        int eventIndexByNotificationType(FileDescriptorNotifier::NotificationType type) const
        {
            return static_cast<uint8_t>(type);
        }
        std::array<FileDescriptorNotifier *, 3> events{ nullptr, nullptr, nullptr };
    };
    std::map<int, NotifierSet> m_notifiers;
};

} // namespace KDFoundation

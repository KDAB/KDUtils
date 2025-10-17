/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>
#include <KDFoundation/kdfoundation_global.h>

#include <kdbindings/signal.h>

#include <cstdint>

namespace KDFoundation {

/// @brief Get async notifications about activity on file descriptors
///
/// @warning On Windows, the write notification is edge triggered meaning that
/// it will be raised only if the internal socket buffer is fully filled, not
/// after every successful write. This means that socket should be written to
/// until WSAEWOULDBLOCK is returned from the send-type operation running on the
/// socket to receive next write notification.
class KDFOUNDATION_API FileDescriptorNotifier : public Object
{
public:
    enum class NotificationType : uint8_t {
        Read = 0,
        Write = 1,
        Exception = 2
    };

    explicit FileDescriptorNotifier(int fd, NotificationType type);
    ~FileDescriptorNotifier();

    KDBindings::Signal<int> triggered;

    int fileDescriptor() const
    {
        return m_fd;
    }
    NotificationType type() const { return m_type; }

    bool isEnabled() const noexcept { return m_isEnabled; }
    void setEnabled(bool enabled);

protected:
    void event(EventReceiver *target, Event *ev) override;

private:
    void registerNotifier();
    void unregisterNotifier();

    int m_fd;
    NotificationType m_type;
    bool m_isEnabled{ true }; // Default to enabled
};

} // namespace KDFoundation

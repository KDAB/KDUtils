/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: AGPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDFoundation/object.h>
#include <KDFoundation/kdfoundation_global.h>

#include <kdbindings/signal.h>

#include <cstdint>

namespace KDFoundation {

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

protected:
    void event(EventReceiver *target, Event *ev) override;

private:
    int m_fd;
    NotificationType m_type;
};

} // namespace KDFoundation

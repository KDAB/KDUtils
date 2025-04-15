/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2025 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include <KDNetwork/sse_event.h>

namespace KDNetwork {

SseEvent::SseEvent(const std::string &eventId,
                   const std::string &eventType,
                   const std::string &eventData,
                   std::optional<int> retryMs)
    : m_id(eventId)
    , m_eventType(eventType)
    , m_data(eventData)
    , m_retry(retryMs)
{
}

} // namespace KDNetwork

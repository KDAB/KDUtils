/*
  This file is part of KDUtils.

  SPDX-FileCopyrightText: 2018 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Paul Lemire <paul.lemire@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <kdbindings/property.h>
#include <kdbindings/signal.h>

#include <map>

using namespace KDBindings;

// TODO: Maybe belongs to ecs?

namespace KDFoundation {

template<typename ContainerType, typename DependencyType>
class DestructionHelperManager
{
public:
    ConnectionHandle contToDepConnection(std::pair<ContainerType *, DependencyType *> contDepPair)
    {
        return m_contToDepMap[contDepPair];
    }

    ConnectionHandle depToContConnection(std::pair<DependencyType *, ContainerType *> depContPair)
    {
        return m_depToContMap[depContPair];
    }

    void addContToDepConnection(std::pair<ContainerType *, DependencyType *> contDepPair, ConnectionHandle handle)
    {
        m_contToDepMap[contDepPair] = handle;
    }

    void addDepToContConnection(std::pair<DependencyType *, ContainerType *> depContPair, ConnectionHandle handle)
    {
        m_depToContMap[depContPair] = handle;
    }

    void delContToDepConnection(std::pair<ContainerType *, DependencyType *> contDepPair)
    {
        if (m_contToDepMap.count(contDepPair)) {
            contDepPair.first->destroyed.disconnect(m_contToDepMap[contDepPair]);
            m_contToDepMap.erase(contDepPair);
        }
    }

    void delDepToContConnection(std::pair<DependencyType *, ContainerType *> depContPair)
    {
        if (m_depToContMap.count(depContPair)) {
            depContPair.first->destroyed.disconnect(m_depToContMap[depContPair]);
            m_depToContMap.erase(depContPair);
        }
    }

    void clearDepToContConnctions()
    {
        for (const auto &depConn : m_depToContMap) {
            depConn.first.first->destroyed.disconnect(depConn.second);
        }
        m_depToContMap.clear();
    }

private:
    std::map<std::pair<ContainerType *, DependencyType *>, ConnectionHandle> m_contToDepMap;
    std::map<std::pair<DependencyType *, ContainerType *>, ConnectionHandle> m_depToContMap;
};

template<typename DependencyType, typename ContainerType>
void registerDestructionHelper(Property<DependencyType *> *property, ConnectionHandle *connectionHandle, ContainerType *owner)
{
    property->valueAboutToChange().connect([=](DependencyType *oldVal, DependencyType * /* newVal */) {
        if (!oldVal) {
            return;
        }
        oldVal->destroyed.disconnect(*connectionHandle);
    });
    property->valueChanged().connect([=](DependencyType *newVal) {
        if (!newVal) {
            return;
        }
        *connectionHandle = newVal->destroyed.connect([=]() {
            (*property) = {};
        });
    });
    if (owner) {
        owner->destroyed.connect([=] {
            if ((*property)() && connectionHandle->isActive()) {
                (*property)()->destroyed.disconnect(*connectionHandle);
            }
        });
    }
}

template<typename ContainerType, typename DependencyType, typename OnDepenencyDestructionFunctorType>
void registerDestructionHelper(ContainerType *container, DependencyType *dependency, DestructionHelperManager<ContainerType, DependencyType> *manager, OnDepenencyDestructionFunctorType dependencyCleanupFunctor)
{
    if (dependency) {
        auto depToCont = dependency->destroyed.connect([=]() {
            dependencyCleanupFunctor();
            manager->delDepToContConnection({ dependency, container });
        });

        manager->addDepToContConnection({ dependency, container }, depToCont);
    }

    if (container) {
        auto contToDep = container->destroyed.connect([=] {
            manager->clearDepToContConnctions();
            manager->delContToDepConnection({ container, dependency });
        });

        manager->addContToDepConnection({ container, dependency }, contToDep);
    }
}

template<typename ContainerType, typename DependencyType>
void unregisterDestructionHelper(ContainerType *container, DependencyType *dependency, DestructionHelperManager<ContainerType, DependencyType> *manager)
{
    manager->delDepToContConnection({ dependency, container });
    manager->delContToDepConnection({ container, dependency });
}

} // namespace KDFoundation

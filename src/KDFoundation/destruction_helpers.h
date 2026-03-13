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

// TODO: Maybe belongs to ecs?

namespace KDFoundation {

template<typename ContainerType, typename DependencyType>
class DestructionHelperManager
{
public:
    KDBindings::ConnectionHandle contToDepConnection(std::pair<ContainerType *, DependencyType *> contDepPair)
    {
        return m_contToDepMap[contDepPair];
    }

    KDBindings::ConnectionHandle depToContConnection(std::pair<DependencyType *, ContainerType *> depContPair)
    {
        return m_depToContMap[depContPair];
    }

    void addContToDepConnection(std::pair<ContainerType *, DependencyType *> contDepPair, KDBindings::ConnectionHandle handle)
    {
        m_contToDepMap[contDepPair] = handle;
    }

    void addDepToContConnection(std::pair<DependencyType *, ContainerType *> depContPair, KDBindings::ConnectionHandle handle)
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

    void clearDepToContConnections()
    {
        for (const auto &depConn : m_depToContMap) {
            depConn.first.first->destroyed.disconnect(depConn.second);
        }
        m_depToContMap.clear();
    }

private:
    std::map<std::pair<ContainerType *, DependencyType *>, KDBindings::ConnectionHandle> m_contToDepMap;
    std::map<std::pair<DependencyType *, ContainerType *>, KDBindings::ConnectionHandle> m_depToContMap;
};

// Watch object held by property of type KDBinding::Property<DependencyType *> and resets property value if node is destroyed
// Optional property owner can be passed in
template<typename DependencyType, typename ContainerType>
void registerDestructionHelper(KDBindings::Property<DependencyType *> *property, KDBindings::ConnectionHandle *connectionHandle, ContainerType *owner)
{
    std::ignore = property->valueAboutToChange().connect([=](DependencyType *oldVal, DependencyType * /* newVal */) {
        if (!oldVal) {
            return;
        }
        oldVal->destroyed.disconnect(*connectionHandle);
    });
    std::ignore = property->valueChanged().connect([=](DependencyType *newVal) {
        if (!newVal) {
            return;
        }
        *connectionHandle = newVal->destroyed.connect([=]() {
            (*property) = {};
        });
    });
    if (owner) {
        std::ignore = owner->destroyed.connect([=] {
            if ((*property)() && connectionHandle->isActive()) {
                (*property)()->destroyed.disconnect(*connectionHandle);
            }
        });
    }
}

// Triggers dependencyCleanupFunctor when DependencyType instance referenced by Container is destroyed
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
            manager->clearDepToContConnections();
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

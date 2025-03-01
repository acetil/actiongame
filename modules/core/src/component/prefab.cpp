#include "core/detail/loggers.h"
#include "core/prefab.h"

#include "core/world.h"

using namespace phenyl::core;

static phenyl::Logger LOGGER{"PREFAB", phenyl::core::detail::COMPONENT_LOGGER};

Prefab::Prefab () : prefabId{0} {}
Prefab::Prefab (std::size_t prefabId, std::weak_ptr<PrefabManager> manager) : prefabId{prefabId}, manager{std::move(manager)} {
    PHENYL_DASSERT(this->manager.lock());
}

Prefab::~Prefab () {
    if (prefabId) {
        if (const auto ptr = manager.lock(); ptr) {
            ptr->decrementRefCount(prefabId);
        }
    }
}

Prefab::Prefab (const Prefab& other) : prefabId{other.prefabId}, manager{other.manager} {
    if (other.prefabId) {
        if (const auto ptr = manager.lock(); ptr) {
            ptr->incrementRefCount(other.prefabId);
        }
    }
}

Prefab::Prefab (Prefab&& other) noexcept : prefabId{other.prefabId}, manager{std::move(other.manager)} {
    other.prefabId = 0;
}

Prefab& Prefab::operator= (const Prefab& other) {
    if (&other == this) {
        return *this;
    }

    if (prefabId) {
        if (const auto ptr = manager.lock(); ptr) {
            ptr->decrementRefCount(prefabId);
        }
    }

    prefabId = other.prefabId;
    manager = other.manager;

    if (other.prefabId) {
        if (const auto ptr = manager.lock(); ptr) {
            ptr->incrementRefCount(other.prefabId);
        }
    }

    return *this;
}

Prefab& Prefab::operator= (Prefab&& other) noexcept {
    if (prefabId) {
        if (const auto ptr = manager.lock(); ptr) {
            ptr->decrementRefCount(prefabId);
        }
    }

    prefabId = other.prefabId;
    manager = std::move(other.manager);

    other.prefabId = 0;

    return *this;
}

void Prefab::instantiate (Entity entity) const {
    auto ptr = manager.lock();
    PHENYL_ASSERT_MSG(ptr, "Attempted to create entity with prefab from already deleted PrefabManager!");

    ptr->instantiate(prefabId, entity);
}

PrefabManager::PrefabManager (World& world) : world{world} {}

Prefab PrefabManager::makePrefab (detail::PrefabFactories factories, std::vector<std::size_t> children) {
    for (auto i : children) {
        PHENYL_DASSERT(entries.contains(i));
    }

    auto id = nextPrefabId++;
    entries.emplace(id, PrefabEntry{
        .factories = std::move(factories),
        .childEntries = std::move(children),
        .refCount = 1,
    });

    return Prefab{id, weak_from_this()};
}

PrefabBuilder PrefabManager::makeBuilder () {
    return PrefabBuilder{*this};
}

void PrefabManager::incrementRefCount (std::size_t prefabId) {
    PHENYL_DASSERT(entries.contains(prefabId));
    entries[prefabId].refCount++;
}

void PrefabManager::decrementRefCount (std::size_t prefabId) {
    auto entryIt = entries.find(prefabId);
    PHENYL_DASSERT(entryIt != entries.end());

    auto& entry = entryIt->second;
    if (!(--entry.refCount)) {
        // Delete prefab
        for (auto i : entry.childEntries) {
            decrementRefCount(i);
        }

        entries.erase(entryIt);
    }
}

void PrefabManager::instantiate (std::size_t prefabId, Entity entity) {
    PHENYL_DASSERT(entries.contains(prefabId));

    if (deferring) {
        deferredInstantiations.emplace_back(entity.id(), prefabId);
        incrementRefCount(prefabId); // So prefab doesnt get deleted while deferring
        return;
    }

    const auto& entry = entries[prefabId];
    world.instantiatePrefab(entity.id(), entry.factories);

    for (auto i : entry.childEntries) {
        instantiate(i, world.create(entity.id()));
    }
}

void PrefabManager::defer () {
    PHENYL_DASSERT(!deferring);
    PHENYL_DASSERT(deferredInstantiations.empty());

    deferring = true;
}

void PrefabManager::deferEnd () {
    PHENYL_DASSERT(deferring);
    deferring = false;

    for (auto [id, prefabId] : deferredInstantiations) {
        Entity entity = world.entity(id);

        if (entity.exists()) {
            instantiate(prefabId, entity);
            decrementRefCount(prefabId);
        }
    }
    deferredInstantiations.clear();
}

PrefabBuilder::PrefabBuilder (PrefabManager& manager) : manager{manager} {}

PrefabBuilder& PrefabBuilder::withChild (const Prefab& child) {
    if (child) {
        manager.incrementRefCount(child.prefabId);
        children.emplace_back(child.prefabId);
    } else {
        PHENYL_LOGE(LOGGER, "Attempted to add invalid prefab as child!");
    }

    return *this;
}

Prefab PrefabBuilder::build () {
    return manager.makePrefab(std::move(factories), std::move(children));
}

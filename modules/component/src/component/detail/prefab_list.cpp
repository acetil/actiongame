#include "component/detail/prefab_list.h"
#include "component/component.h"

void component::detail::PrefabList::instantiate (component::ComponentManager* manager, component::EntityId id, std::size_t prefabId) {
    assert(valid(prefabId));

    auto& entry = getPrefabEntry(prefabId);
    for (auto& [set, compId] : entry.comps) {
        set->insertPrefab(id, compId);
    }

    if (entry.inherits) {
        instantiate(manager, id, entry.inherits);
    }

    for (const auto& i : entry.children) {
        auto childId = manager->create(id);
        instantiate(manager, childId.id(), i);
    }
}
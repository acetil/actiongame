#pragma once

#include "component/detail/entity_id_list.h"
#include "component/entity_id.h"
#include "component/detail/relationships.h"
#include "component/detail/loggers.h"

#include "archetype.h"
#include "entity2.h"

namespace phenyl::component {
    class ComponentManager2 : private detail::IArchetypeManager {
    private:
        detail::EntityIdList idList;
        detail::RelationshipManager relationships;

        std::vector<std::unique_ptr<Archetype>> archetypes;
        EmptyArchetype* emptyArchetype;
        std::vector<detail::EntityEntry> entityEntries;

        void removeInt (EntityId id, bool updateParent);

        void addArchetype (std::unique_ptr<Archetype> archetype) override;
        Archetype* findArchetype (const std::vector<std::size_t>& comps) override;
        void updateEntityEntry (EntityId id, Archetype* archetype, std::size_t pos) override;

        friend Entity2;
    public:
        static constexpr std::size_t DEFAULT_CAPACITY = 256;

        explicit ComponentManager2 (std::size_t capacity=DEFAULT_CAPACITY);

        Entity2 create (EntityId parent);
        void remove (EntityId id);
        void reparent (EntityId id, EntityId parent);

        [[nodiscard]] bool exists (EntityId id) const noexcept {
            return idList.check(id);
        }

        [[nodiscard]] Entity2 parent (EntityId id) noexcept;
    };
}

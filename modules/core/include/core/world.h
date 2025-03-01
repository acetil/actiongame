#pragma once

#include "component/detail/entity_id_list.h"
#include "entity_id.h"
#include "component/detail/relationships.h"

#include "component/archetype.h"
#include "component/children_view.h"
#include "entity.h"
#include "component/query.h"
#include "component/detail/component_instance.h"
#include "component/detail/signal_handler.h"
#include "prefab.h"
#include "component/forward.h"

namespace phenyl::core {
    class World : private detail::IArchetypeManager {
    private:
        class EntityIterator {
        private:
            World* world = nullptr;;
            detail::EntityIdList::const_iterator it;

        public:
            using value_type = Entity;

            EntityIterator () = default;
            explicit EntityIterator (World* world, detail::EntityIdList::const_iterator it) : world{world}, it{it} {}

            value_type operator* () const;

            EntityIterator& operator++ ();
            EntityIterator operator++ (int);

            EntityIterator& operator-- ();
            EntityIterator operator-- (int);

            bool operator== (const EntityIterator&) const;
        };

        std::unordered_map<std::size_t, std::unique_ptr<detail::UntypedComponent>> components;

        detail::EntityIdList idList;
        detail::RelationshipManager relationships;

        std::vector<std::unique_ptr<Archetype>> archetypes;
        EmptyArchetype* emptyArchetype;
        std::vector<detail::EntityEntry> entityEntries;

        std::vector<std::weak_ptr<QueryArchetypes>> queryArchetypes;

        std::unordered_map<std::size_t, std::unique_ptr<detail::IHandlerVector>> signalHandlerVectors;

        std::shared_ptr<PrefabManager> prefabManager;

        std::vector<std::pair<EntityId, EntityId>> deferredCreations;
        std::vector<std::pair<EntityId, std::function<void(Entity)>>> deferredApplys;
        std::vector<EntityId> deferredRemovals;

        std::uint32_t deferCount = 0;
        std::uint32_t removeDeferCount = 0;
        std::uint32_t signalDeferCount = 0;

        void completeCreation (EntityId id, EntityId parent);
        void removeInt (EntityId id, bool updateParent);

        std::shared_ptr<QueryArchetypes> makeQueryArchetypes (detail::ArchetypeKey key);
        void cleanupQueryArchetypes ();

        Archetype* findArchetype (const detail::ArchetypeKey& key) override;
        void updateEntityEntry (EntityId id, Archetype* archetype, std::size_t pos) override;

        void onComponentInsert (EntityId id, std::size_t compType, std::byte* ptr) override;
        void onComponentRemove (EntityId id, std::size_t compType, std::byte* ptr) override;

        void deferInsert (EntityId id, std::size_t compType, std::byte* ptr);
        void deferErase (EntityId id, std::size_t compType);
        void deferApply (EntityId id, std::function<void(Entity)> applyFunc);

        void instantiatePrefab (EntityId id, const detail::PrefabFactories& factories);

        void raiseSignal (EntityId id, std::size_t signalType, std::byte* ptr);

        void deferRemove ();
        void deferRemoveEnd ();

        friend Entity;
        friend ChildrenView;
        friend PrefabManager;
    public:
        using iterator = EntityIterator;

        static constexpr std::size_t DEFAULT_CAPACITY = 256;

        explicit World (std::size_t capacity=DEFAULT_CAPACITY);
        ~World () override;

        World (const World&) = delete;
        World (World&& other) = default;

        World& operator= (const World&) = delete;
        World& operator= (World&&) = default;

        template <typename T>
        void addComponent (std::string name) {
            PHENYL_ASSERT_MSG(!components.contains(meta::type_index<T>()), "Attempted to add component \"{}\" twice", name);

            auto comp = std::make_unique<detail::Component<T>>(this, std::move(name));
            auto index = comp->type();
            components.emplace(index, std::move(comp));
        }

        Entity create (EntityId parent = EntityId{});
        void remove (EntityId id);
        void reparent (EntityId id, EntityId parent);

        void clear ();

        [[nodiscard]] bool exists (EntityId id) const noexcept {
            return idList.check(id);
        }

        Entity entity (EntityId id) noexcept;
        ChildrenView root () noexcept;

        [[nodiscard]] Entity parent (EntityId id) noexcept;

        template <typename ...Args>
        Query<Args...> query () {
            return Query<Args...>{makeQueryArchetypes(detail::ArchetypeKey::Make<Args...>()), this};
        }

        template <typename T>
        void addHandler (std::function<void(const OnInsert<T>&, Entity)> handler) {
            auto it = components.find(meta::type_index<T>());
            PHENYL_ASSERT_MSG(it != components.end(), "Failed to find component in addHandler()");

            auto& component = static_cast<detail::Component<T>&>(*it->second);
            component.addHandler(std::move(handler));
        }

        template <typename T>
        void addHandler (std::function<void(const OnRemove<T>&, Entity)> handler) {
            auto it = components.find(meta::type_index<T>());
            PHENYL_ASSERT_MSG(it != components.end(), "Failed to find component in addHandler()");

            auto& component = static_cast<detail::Component<T>&>(*it->second);
            component.addHandler(std::move(handler));
        }

        template <typename Signal, typename ...Args>
        void addHandler (std::function<void(const Signal&, const Bundle<Args...>& bundle)> handler) {
            detail::SignalHandlerVector<Signal>* handlerVec;
            auto vecIt = signalHandlerVectors.find(meta::type_index<Signal>());
            if (vecIt != signalHandlerVectors.end()) {
                handlerVec = static_cast<detail::SignalHandlerVector<Signal>*>(vecIt->second.get());
            } else {
                auto newVec = std::make_unique<detail::SignalHandlerVector<Signal>>(*this);
                handlerVec = newVec.get();
                signalHandlerVectors.emplace(meta::type_index<Signal>(), std::move(newVec));
            }

            handlerVec->addHandler(std::make_unique<detail::SignalHandler2<Signal, Args...>>(query<Args...>(), std::move(handler)));
        }

        template <typename Signal, typename ...Args>
        void addHandler (auto&& fn) requires meta::callable<decltype(fn), void, const Signal&, const Bundle<Args...>&> {
            addHandler<Signal, Args...>(std::function<void(const Signal&, const Bundle<Args...>&)>{std::forward<decltype(fn)>(fn)});
        }

        template <typename Signal, typename ...Args>
        void addHandler (std::function<void(const Signal&, std::remove_reference_t<Args>&... args)> handler) {
            addHandler(std::function<void(const Signal&, const Bundle<Args...>&)>{[handler = std::move(handler)] (const Signal& signal, const Bundle<Args...>& bundle) {
                handler(signal, bundle.template get<Args>()...);
            }});
        }

        template <typename Signal, typename ...Args>
        void addHandler (auto&& fn) requires meta::callable<decltype(fn), void, const Signal&, std::remove_reference_t<Args>&...> {
            addHandler<Signal, Args...>(std::function<void(const Signal&, std::remove_reference_t<Args>&...)>{std::forward<decltype(fn)>(fn)});
        }

        void defer ();
        void deferEnd ();
        void deferSignals ();
        void deferSignalsEnd ();

        PrefabBuilder buildPrefab ();

        iterator begin ();
        iterator end ();
    };
}

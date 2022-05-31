#pragma once

#include "component/component.h"
#include "event/event.h"
#include "component/view/game_view.h"
#include <string>
#include <utility>

namespace event {
    class EntityCollisionEvent : public Event<EntityCollisionEvent> {
    public:
        component::EntityId entityId;
        component::EntityId otherId;
        unsigned int collisionLayers;
        inline static const std::string name = "entity_collision";
        component::EntityComponentManager::SharedPtr componentManager;
        EventBus::SharedPtr eventBus; // TODO: find better way
        view::GameView& gameView;
        //EntityCollisionEvent () : entityId(0), otherId(0), collisionLayers(0), componentManager(), eventBus{}, gameView(view::GameView(nullptr)) {}
        EntityCollisionEvent (component::EntityId _entityId, component::EntityId _otherId, unsigned int _collisionLayers,
                              component::EntityComponentManager::SharedPtr compManager, EventBus::SharedPtr bus, view::GameView& _gameView) : entityId(_entityId),
                                    otherId(_otherId), collisionLayers(_collisionLayers), componentManager(std::move(compManager)), eventBus(std::move(bus)), gameView(_gameView) {};
    };
}
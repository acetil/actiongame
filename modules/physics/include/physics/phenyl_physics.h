#pragma once

#include <memory>

#include "component/forward.h"
#include "event/forward.h"

namespace view {
    class GameView;
}

namespace physics {
    class IPhysics;
    class PhenylPhysics {
    private:
        IPhysics* physicsObj;
    public:
        explicit PhenylPhysics (IPhysics* physicsObj);
        void updatePhysics (component::EntityComponentManager& componentManager);
        void checkCollisions (component::EntityComponentManager& componentManager, const std::shared_ptr<event::EventBus>& eventBus, view::GameView& gameView);

        void updateEntityPosition (const component::EntityComponentManager& componentManager, const std::shared_ptr<event::EventBus>& eventBus, view::GameView& gameView);
    };
}
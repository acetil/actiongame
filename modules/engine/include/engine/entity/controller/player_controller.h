#pragma once

#include "common/events/player_movement_change.h"
#include "engine/entity/controller/entity_controller.h"
#include "common/events/cursor_position_change.h"
#include "common/events/player_shoot_change.h"
#include "event/event_scope.h"

namespace game {
    class PlayerController : public EntityController {
    private:
        //float deltaXForce;
        //float deltaYForce;
        glm::vec2 currForce{0,0};
        glm::vec2 cursorScreenPos = {0, 0};
        bool doShoot = false;
        bool hasShot = false;
        event::EventScope eventScope;
    public:
        PlayerController () : EntityController("test_entity") {}
        void updateMovement (event::PlayerMovementChangeEvent& event);
        void controlEntityPrePhysics (component::EntityView& entityView, view::GameView& gameView) override;
        void controlEntityPostPhysics(component::EntityView &entityView, view::GameView &gameView) override;
        void updateCursorPos (event::CursorPosChangeEvent& event);
        void updateDoShoot (event::PlayerShootChangeEvent& event);

        event::EventScope& getEventScope () {
            // TODO remove
            return eventScope;
        }
    };
}

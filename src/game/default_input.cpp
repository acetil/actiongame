#include "default_input.h"

#include "common/events/player_movement_change.h"
#include "common/events/player_shoot_change.h"
#include "common/events/debug/debug_step.h"

#include "util/debug_console.h"

#define FORCE_COMPONENT 6.5

static common::InputAction KeyUp;
static common::InputAction KeyDown;
static common::InputAction KeyLeft;
static common::InputAction KeyRight;

void setupDefaultInput (game::GameInput& gameInput, const event::EventBus::SharedPtr& eventBus) {
    auto upKey = gameInput.mapInput("move_up", "key_w");
    auto downKey = gameInput.mapInput("move_down", "key_s");
    auto leftKey = gameInput.mapInput("move_left", "key_a");
    auto rightKey = gameInput.mapInput("move_right", "key_d");

    auto shootKey = gameInput.mapInput("player_shoot", "mouse_left");

    auto debugKey = gameInput.mapInput("debug_console", "key_f12");
    auto stepKey = gameInput.mapInput("debug_step", "key_f7");

    gameInput.addInputEvent(upKey, event::PlayerMovementChangeEvent(0, FORCE_COMPONENT), event::PlayerMovementChangeEvent(0, -FORCE_COMPONENT));
    gameInput.addInputEvent(downKey, event::PlayerMovementChangeEvent(0, -FORCE_COMPONENT), event::PlayerMovementChangeEvent(0, FORCE_COMPONENT));
    gameInput.addInputEvent(leftKey, event::PlayerMovementChangeEvent(-FORCE_COMPONENT, 0), event::PlayerMovementChangeEvent(FORCE_COMPONENT, 0));
    gameInput.addInputEvent(rightKey, event::PlayerMovementChangeEvent(FORCE_COMPONENT, 0), event::PlayerMovementChangeEvent(-FORCE_COMPONENT, 0));

    gameInput.addInputEvent(shootKey, event::PlayerShootChangeEvent(true), event::PlayerShootChangeEvent(false));

    gameInput.addInputEvent(debugKey, game::EmptyEventAction(), util::DebugConsoleEvent{eventBus});
    gameInput.addInputEvent(stepKey, game::EmptyEventAction(), event::DebugStepEvent{.status=event::DebugStepStatus::DO_STEP});

    eventBus->subscribeUnscoped(util::doDebugConsole);
}

void frameInput (component::ComponentManager& manager) {

}
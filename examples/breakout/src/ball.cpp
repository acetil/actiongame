#include <phenyl/components/physics/2D/rigid_body.h>
#include <phenyl/signals/physics.h>

#include "ball.h"
#include "breakout.h"

static constexpr std::uint32_t FLOOR_LAYER = 1 << 2;

using namespace breakout;

static void BallUpdateSystem (const Ball& ball, phenyl::RigidBody2D& body);

void breakout::InitBall (BreakoutApp* app, phenyl::World& world) {
    app->addComponent<Ball>("Ball");

    world.addHandler<phenyl::signals::OnCollision, const Ball>([app] (const phenyl::signals::OnCollision& signal, const phenyl::Bundle<const Ball>& bundle) {
        if (signal.collisionLayers & FLOOR_LAYER) {
            bundle.entity().remove();
            app->subtractLife();
        }
    });
    app->runtime().addSystem<phenyl::FixedUpdate>("Ball::Update", BallUpdateSystem);
}

void BallUpdateSystem (const Ball& ball, phenyl::RigidBody2D& body) {
    auto vel = body.getMomentum() * body.getInvMass();

    if (glm::dot(vel, vel) > ball.maxSpeed * ball.maxSpeed) {
        body.applyImpulse(-body.getMomentum() * (glm::length(vel) - ball.maxSpeed) * ball.appliedDrag);
    }
}
#include "physics_2d.h"
#include "physics/components/2D/rigid_body.h"
#include "common/debug.h"

#include "common/components/2d/global_transform.h"
#include "physics/components/2D/rigid_body.h"
#include "physics/components/2D/colliders/box_collider.h"
#include "physics/signals/collision.h"

#include "physics/components/2D/rigid_body.h"
#include "component/component_serializer.h"
#include "runtime/delta_time.h"

#define SOLVER_ITERATIONS 10

using namespace phenyl::physics;

struct Constraints2D : public phenyl::runtime::IResource {
    std::vector<Constraint2D> constraints;

    std::string_view getName() const noexcept override {
        return "Constraints2D";
    }
};

static void RigidBody2DMotionSystem (const phenyl::runtime::Resources<const phenyl::runtime::FixedDelta>& resources, phenyl::common::GlobalTransform2D& transform, RigidBody2D& body) {
    auto& [delta] = resources;

    body.doMotion(transform, static_cast<float>(delta()));
}

static void Collider2DSyncSystem (const phenyl::common::GlobalTransform2D& transform, const RigidBody2D& body, BoxCollider2D& collider) {
    collider.syncUpdates(body, transform.transform2D.position());
}

static void BoxCollider2DFrameTransformSystem (const phenyl::common::GlobalTransform2D& transform, BoxCollider2D& collider) {
    collider.applyFrameTransform(transform.transform2D.rotMatrix());
}

static void CollisionCheck2DSystem (const phenyl::runtime::Resources<Constraints2D, const phenyl::runtime::FixedDelta>& resources,
    const phenyl::component::Bundle<BoxCollider2D>& bundle1, const phenyl::component::Bundle<BoxCollider2D>& bundle2) {
    auto& [constraints, deltaTime] = resources;

    auto entity1 = bundle1.entity();
    auto entity2 = bundle2.entity();
    auto& box1 = bundle1.get<BoxCollider2D>();
    auto& box2 = bundle2.get<BoxCollider2D>();
    if (!box1.shouldCollide(box2)) {
        return;
    }

    box1.collide(box2)
        .ifPresent([&] (SATResult2D result) {
            auto face1 = box1.getSignificantFace(result.normal);
            auto face2 = box2.getSignificantFace(-result.normal);

            auto manifold = buildManifold(face1, face2, result.normal, result.depth);
            constraints.constraints.emplace_back(manifold.buildConstraint(&box1, &box2, deltaTime()));

            auto contactPoint = manifold.getContactPoint();
            if (box1.layers & box2.mask) {
                //manager._signal<OnCollision>(entity2.id(), entity1.id(), (std::uint32_t)(box1.layers & box2.mask));
                entity2.raise(OnCollision{entity1.id(), (std::uint32_t)(box1.layers & box2.mask), contactPoint, -result.normal});
                //events.emplace_back(info2.id(), info1.id(), box1.layers & box2.mask);
            }

            if (box2.layers & box1.mask) {
                //manager.signal<OnCollision>(entity1.id(), entity2.id(), (std::uint32_t)(box2.layers & box1.mask));
                entity1.raise(OnCollision{entity2.id(), (std::uint32_t)(box2.layers & box1.mask), contactPoint, result.normal});
                //events.emplace_back(info1.id(), info2.id(), box2.layers & box1.mask);
            }
        });
}

static void Constraints2DSolveSystem (const phenyl::runtime::Resources<Constraints2D>& resources) {
    auto& [constraints] = resources;

    for (auto i = 0; i < SOLVER_ITERATIONS; i++) {
        bool shouldContinue = false;

        for (auto& c : constraints.constraints) {
            auto res = c.solve();
            shouldContinue = shouldContinue || res;
        }

        if (!shouldContinue) {
            break;
        }
    }

    constraints.constraints.clear();
}

static void Collider2DUpdateSystem (RigidBody2D& body, const BoxCollider2D& collider) {
    collider.updateBody(body);
}

void Physics2D::addComponents (runtime::PhenylRuntime& runtime) {
    runtime.addComponent<RigidBody2D>("RigidBody2D");
    //runtime.addUnserializedComponent<Collider2D>("Collider2D");
    runtime.addComponent<BoxCollider2D>("BoxCollider2D");

    //runtime.manager().inherits<BoxCollider2D, Collider2D>();

    //runtime.manager().addRequirement<RigidBody2D, common::GlobalTransform2D>();
    //runtime.manager().addRequirement<BoxCollider2D, common::GlobalTransform2D>();
    //runtime.manager().addRequirement<BoxCollider2D, RigidBody2D>();

    runtime.addResource<Constraints2D>();
    auto& motionSystem = runtime.addSystem<runtime::PhysicsUpdate>("RigidBody2D::Update", RigidBody2DMotionSystem);
    auto& syncSystem = runtime.addSystem<runtime::PhysicsUpdate>("Collider2D::Sync", Collider2DSyncSystem);
    auto& boxTransformSystem = runtime.addSystem<runtime::PhysicsUpdate>("BoxCollider2D::FrameTransform", BoxCollider2DFrameTransformSystem);
    auto& collCheckSystem = runtime.addSystem<runtime::PhysicsUpdate>("Physics2D::CollisionCheck", CollisionCheck2DSystem);
    auto& constraintSolveSystem = runtime.addSystem<runtime::PhysicsUpdate>("Physics2D::ConstraintsSolve", Constraints2DSolveSystem);
    auto& collUpdateSystem = runtime.addSystem<runtime::PhysicsUpdate>("Collider2D::PostCollision", Collider2DUpdateSystem);

    motionSystem.runBefore(syncSystem);
    syncSystem.runBefore(boxTransformSystem);
    boxTransformSystem.runBefore(collCheckSystem);
    collCheckSystem.runBefore(constraintSolveSystem);
    constraintSolveSystem.runBefore(collUpdateSystem);
}

void Physics2D::debugRender (component::World& world) {
    // Debug render
    world.query<common::GlobalTransform2D, BoxCollider2D>().each([] (const common::GlobalTransform2D& transform, const BoxCollider2D& box) {
        auto pos1 = box.frameTransform * glm::vec2{-1, -1} + transform.transform2D.position();
        auto pos2 = box.frameTransform * glm::vec2{1, -1} + transform.transform2D.position();
        auto pos3 = box.frameTransform * glm::vec2{1, 1} + transform.transform2D.position();
        auto pos4 = box.frameTransform * glm::vec2{-1, 1} + transform.transform2D.position();

        common::debugWorldRectOutline(pos1, pos2, pos3, pos4, {0, 0, 1, 1});
    });
}
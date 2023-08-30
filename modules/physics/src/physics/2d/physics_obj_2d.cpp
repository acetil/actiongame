#include <random>

#include "physics_obj_2d.h"
#include "component/component_serialiser.h"
#include "physics/components/2D/collision_component.h"
#include "physics/components/2D/rigid_body.h"
#include "common/events/entity_collision.h"
#include "common/events/entity_creation.h"
#include "common/debug.h"

#include "physics/shape/2d/box_shape_2d_interface.h"
#include "common/components/2d/global_transform.h"
#include "common/events/debug/debug_render.h"

#define SOLVER_ITERATIONS 10

using namespace physics;

ShapeId PhysicsObject2D::makeShapeFromRequest (ShapeRegistry2D& registry, ColliderId collider, std::size_t typeIndex, void* request) {
    switch (typeIndex) {
        case ShapeRequest<BoxShape2D>::type: {
            return makeId<ShapeId>(registry.addComponent<BoxShape2D>(static_cast<ShapeRequest<BoxShape2D>*>(request)->make(collider)));
        }

        default:
            logging::log(LEVEL_ERROR, "Unknown 2D shape type: {} requested for collider {}", typeIndex, collider.getValue());
            return {};
    }
}

ShapeId PhysicsObject2D::deserialiseShape (const util::DataValue& val, ColliderId collider, std::size_t layers, std::size_t mask) {
    if (!val.is<util::DataObject>()) {
        return {};
    }

    const auto& obj = val.get<util::DataObject>();
    if (!obj.contains("type") || !obj.at("type").is<std::string>()) {
        return {};
    }

    const auto& type = obj.at("type").get<std::string>();

    if (type == "BoxShape2D") {
        auto boxOpt = BoxShape2D::deserialise(obj, collider, layers, mask);
        if (!boxOpt) {
            return {};
        }

        return makeId<ShapeId>(shapeRegistry.addComponent<BoxShape2D>(boxOpt.getUnsafe()));
    } else {
        logging::log(LEVEL_ERROR, "Unknown shape type: {}", type);
        return {};
    }
}

void PhysicsObject2D::addComponentSerialisers (component::EntitySerialiser& serialiser) {
    //serialiser.addComponentSerialiser<SimpleFrictionMotion2D>("SimpleFrictionMotion2D");
    //serialiser.addComponentSerialiser<CollisionComponent2D>("CollisionComponent2D");
    serialiser.addComponentSerialiser<CollisionComponent2D>("CollisionComponent2D", [this] (const CollisionComponent2D& comp) -> util::DataValue {
        util::DataObject obj;
        obj["coll_scale"] = comp.transform;

        const auto& coll = getCollider(comp.collider);
        obj["mass"] = coll.mass;
        obj["layers"] = static_cast<unsigned int>(coll.hitboxLayers); // TODO: remove static cast
        obj["masks"] = static_cast<unsigned int>(coll.eventboxMask);
        obj["resolve_layers"] = static_cast<unsigned int>(coll.eventboxMask);
        obj["event_layers"] = static_cast<unsigned int>(coll.eventboxMask);
        obj["shape"] = ((Shape2D*)shapeRegistry.getComponentErased(makePublicId(coll.hitbox)))->serialise();

        return obj;
    }, [this] (const util::DataValue& val) -> util::Optional<CollisionComponent2D> {
        if (!val.is<util::DataObject>()) {
            return util::NullOpt;
        }
        const auto& obj = val.get<util::DataObject>();


        CollisionComponent2D comp;
        if (!obj.contains("coll_scale") || !phenyl_from_data(obj.at("coll_scale"), comp.transform)) {
            return util::NullOpt;
        }

        Collider2D coll;
        if (!obj.contains("mass") || !obj.at("mass").is<float>()) {
            return util::NullOpt;
        }
        coll.mass = obj.at("mass").get<float>();

        if (!obj.contains("layers") || !obj.at("layers").is<int>()) { // TODO
            return util::NullOpt;
        }
        coll.hitboxLayers = (std::size_t)obj.at("layers").get<unsigned int>();

        if (!obj.contains("masks") || !obj.at("masks").is<int>()) { // TODO
            return util::NullOpt;
        }
        coll.eventboxMask = (std::size_t)obj.at("masks").get<unsigned int>();

        if (!obj.contains("shape")) {
            return util::NullOpt;
        }
        auto collId = makeId<ColliderId>(colliders.push(coll));
        auto shapeId = deserialiseShape(obj.at("shape"), collId, coll.hitboxLayers, coll.eventboxMask);
        if (!shapeId) {
            destroyCollider(collId);
            return util::NullOpt;
        }
        getCollider(collId).hitbox = shapeId;
        comp.collider = collId;
        return {comp};
    });
    serialiser.addComponentSerialiser<RigidBody2D>("RigidBody2D");
    serialiser.addComponentSerialiser<SimpleFriction>("SimpleFriction");
}

void PhysicsObject2D::updatePhysics (component::EntityComponentManager& componentManager, float deltaTime) {
    for (auto [kinMotion, friction] : componentManager.iterate<RigidBody2D, SimpleFriction>()) {
        friction.updateFriction2D(kinMotion, deltaTime);
    }

    for (auto [kinMotion, transform] : componentManager.iterate<RigidBody2D, common::GlobalTransform2D>()) {
        //updatePhysicsInternal(i.get<SimpleFrictionMotion2D>(), i.get<component::Position2D>());
        kinMotion.doMotion(transform, deltaTime);
    }
}

void PhysicsObject2D::resolveCollision (physics::ColliderId id1, physics::ColliderId id2, glm::vec2 disp, component::EntityComponentManager& compManager) {
    const auto& coll1 = getCollider(id1);
    const auto& coll2 = getCollider(id2);

    auto totalMass = coll1.mass + coll2.mass;
    if (totalMass > 0) {
        auto res1 = (-disp) * coll1.mass / totalMass;
        logging::log(LEVEL_DEBUG, "Collision resolution for {}: <{}, {}>", coll1.entityId.value(), res1.x, res1.y);
        //compManager.get<component::Position2D>(coll1.entityId).getUnsafe() += res1;
        compManager.get<common::GlobalTransform2D>(coll1.entityId).getUnsafe().transform2D.translate(res1);
        auto res2 = disp * coll2.mass / totalMass;
        logging::log(LEVEL_DEBUG, "Collision resolution for {}: <{}, {}>", coll2.entityId.value(), res2.x, res2.y);
        compManager.get<common::GlobalTransform2D>(coll2.entityId).getUnsafe().transform2D.translate(res2);

        // TODO: evaluate if necessary
        /*compManager->getObjectData<RigidBody2D>(coll1.entityId).ifPresent([&disp, &coll1, &totalMass] (RigidBody2D& comp) {
            comp.momentum -= projectVec(disp * (coll1.mass / totalMass), comp.momentum);
        });

        compManager->get<RigidBody2D>(coll2.entityId).ifPresent([&disp, &coll2, &totalMass] (RigidBody2D& comp) {
            comp.momentum -= projectVec(-disp * (coll2.mass / totalMass), comp.momentum);
        });*/
    }
}

void PhysicsObject2D::checkCollisions (component::EntityComponentManager& compManager, const event::EventBus::SharedPtr& eventBus, view::GameView& gameView, float deltaTime) {
    //logging::log(LEVEL_DEBUG, "===New collision check===");
    for (const auto& [collComp, body, transform] : compManager.iterate<CollisionComponent2D, RigidBody2D, common::GlobalTransform2D>()) {
        //logging::log(LEVEL_DEBUG, "{}: collId={}", i.id().value(), i.get<CollisionComponent2D>().collider.getValue());
        auto& collider = getCollider(collComp.collider);
        //logging::log(LEVEL_DEBUG, "{}: shapeId={}", i.id().value(), collider.hitbox.getValue());
        collider.updated = true;

        auto hitbox = ((Shape2D*)(shapeRegistry.getComponentErased(makePublicId(collider.hitbox))));
        hitbox->applyTransform(transform.transform2D.rotMatrix() * collComp.transform); // TODO: dirty?
        hitbox->setPosition(transform.transform2D.position());

        collider.currentPos = transform.transform2D.position();
        collider.mass = body.mass;
        collider.inertialMoment = body.inertialMoment;
        collider.momentum = body.getMomentum();
        collider.angularMomentum = body.getAngularMomentum();
        collider.elasticity = body.elasticity;

        collider.appliedImpulse = {0.0f, 0.0f};
        collider.appliedAngularImpulse = 0.0f;
    }

    // TODO: better way
    for (auto [id, coll] : colliders.iterate()) {
        if (coll.updated) {
            coll.updated = false;
        } else {
            logging::log(LEVEL_DEBUG, "Removing coll={}, shape={}", id + 1, coll.hitbox.getValue());
            shapeRegistry.remove(makePublicId(coll.hitbox));
            colliders.remove(id); // Valid because FLVector, so removal doesnt change other elements
        }
    }

    /*for (const auto& [box1, id1] : shapeRegistry.iterate<BoxShape2D>()) {
        for (const auto& [box2, id2] : shapeRegistry.iterate<BoxShape2D>()) {
            if (id1 == id2 || !(box1.shouldCollide(box2))) {
                continue;
            }

            box1.collide(box2)
                .ifPresent([&collisionResults, &box1, &box2] (glm::vec2 vec) {
                    collisionResults.emplace_back(box1.getColliderId(), box2.getColliderId(), vec);
                });
        }
    }*/

    std::vector<std::tuple<ColliderId, ColliderId, Manifold2D>> collisionResults;
    auto iterable = shapeRegistry.iterate<BoxShape2D>();
    for (auto it1 = iterable.begin(); it1 != iterable.end(); ++it1) {
        auto [box1, id1] = *it1;
        auto it2 = it1;
        ++it2;
        for ( ; it2 != iterable.end(); ++it2) {
            auto [box2, id2] = *it2;
            if (!box1.shouldCollide(box2)) {
                continue;
            }
            box1.collide(box2)
                .ifPresent([&collisionResults, &box1, &box2] (SATResult2D result) {
                    auto face1 = box1.getSignificantFace(result.normal);
                    //logging::log(LEVEL_DEBUG, "Face 1: <{}, {}>-<{}, {}>, norm: <{}, {}>", face1.vertices[0].x, face1.vertices[0].y, face1.vertices[1].x, face1.vertices[1].y, face1.normal.x, face1.normal.y);
                    auto face2 = box2.getSignificantFace(-result.normal);
                    //logging::log(LEVEL_DEBUG, "Face 2: <{}, {}>-<{}, {}>, norm: <{}, {}>", face2.vertices[0].x, face2.vertices[0].y, face2.vertices[1].x, face2.vertices[1].y, face2.normal.x, face2.normal.y);

                    auto manifold = buildManifold(face1, face2, result.normal, result.depth);
                    //logging::log(LEVEL_DEBUG, "Manifold: <{}, {}>-<{}, {}>, norm: <{}, {}>, type: {}", manifold.points[0].x, manifold.points[0].y, manifold.points[1].x, manifold.points[1].y, manifold.normal.x, manifold.normal.y, (int)manifold.type);
                    collisionResults.emplace_back(box1.getColliderId(), box2.getColliderId(), manifold);
                });
        }
    }

    std::vector<std::tuple<component::EntityId, component::EntityId, std::uint32_t>> events;
    std::vector<Constraint2D> constraints;
    for (const auto& [id1, id2, manifold] : collisionResults) {
        //auto disp = manifold.normal * manifold.depth;
        /*logging::log(LEVEL_DEBUG, "Detected collision between entities {} and {} with min translation vec <{}, {}>!",
                     getCollider(id1).entityId.value(), getCollider(id2).entityId.value(), disp.x, disp.y);*/

        //resolveCollision(id1, id2, disp, compManager);
        manifold.buildConstraints(constraints, &getCollider(id1), &getCollider(id2), deltaTime);

        if (getCollider(id1).hitboxLayers & getCollider(id2).eventboxMask) {
            events.emplace_back(getCollider(id2).entityId, getCollider(id1).entityId, getCollider(id1).hitboxLayers & getCollider(id2).eventboxMask);
        }

        if (getCollider(id2).hitboxLayers & getCollider(id1).eventboxMask) {
            events.emplace_back(getCollider(id1).entityId, getCollider(id2).entityId, getCollider(id2).hitboxLayers & getCollider(id1).eventboxMask);
        }
    }

    solveConstraints(constraints, compManager, deltaTime);

    for (const auto& [id1, id2, layers] : events) {
        eventBus->raise(event::EntityCollisionEvent{compManager.view(id1), compManager.view(id2), layers, compManager, eventBus, gameView});
    }
}

void PhysicsObject2D::solveConstraints (std::vector<Constraint2D>& constraints, component::EntityComponentManager& compManager, float deltaTime) {
    //std::shuffle(constraints.begin(), constraints.end(), std::random_device{});
    for (auto i = 0; i < SOLVER_ITERATIONS; i++) {
         //logging::log(LEVEL_DEBUG, "Solver iteration: {}", i);
        bool shouldContinue = false;

        for (auto& c : constraints) {
            auto res = c.solve(deltaTime);
            shouldContinue = shouldContinue || res;
        }

        if (!shouldContinue) {
            break;
        }
    }

    for (auto [id, collider] : colliders.iterate()) {
        auto& [transform, body] = compManager.get<common::GlobalTransform2D, RigidBody2D>(collider.entityId).getUnsafe();

        auto appliedImpulse = collider.appliedImpulse;
        auto appliedAngularImpulse = collider.appliedAngularImpulse;


        if (body.mass != 0) {
            //logging::log(LEVEL_DEBUG, "Applying impulse <{}, {}> to entity {}!", appliedImpulse.x, appliedImpulse.y, collider.entityId.value());
            body.applyImpulse(appliedImpulse);
            //transform.transform2D.translate(appliedImpulse / body.mass * deltaTime);
        }

        if (body.inertialMoment != 0) {
            //logging::log(LEVEL_DEBUG, "Applying angular impulse {} to entity {}!", appliedAngularImpulse, collider.entityId.value());
            body.applyAngularImpulse(appliedAngularImpulse);
            //transform.transform2D.rotateBy(appliedAngularImpulse / body.inertialMoment * deltaTime);
        }
    }
}


ColliderId PhysicsObject2D::addCollider (component::EntityId entityId) {
    auto index = colliders.emplace(entityId);
    return makeId<ColliderId>(index);
}

bool PhysicsObject2D::colliderExists (ColliderId id) const {
    return colliders.present(getIdIndex(id));
}

Collider2D& PhysicsObject2D::getCollider (ColliderId id) {
    return colliders.at(getIdIndex(id));
}

const Collider2D& PhysicsObject2D::getCollider (ColliderId id) const {
    return colliders.at(getIdIndex(id));
}


void PhysicsObject2D::destroyCollider (physics::ColliderId id) {
    auto index = getIdIndex(id);

    if (!colliderExists(id)) {
        logging::log(LEVEL_WARNING, "Attempted to destroy collider {} that does not exist!", index);
    } else {
        colliders.remove(index);
    }
}

ShapeId PhysicsObject2D::getColliderHitbox (physics::ColliderId id) {
    if (!colliderExists(id)) {
        logging::log(LEVEL_WARNING, "Attempted to get hitbox of collider {} that does not exist!", getIdIndex(id));
        return {};
    }

    return getCollider(id).hitbox;
}

/*ShapeId PhysicsObject2D::getColliderEventbox (physics::ColliderId entityId) {
    if (!colliderExists(entityId)) {
        logging::log(LEVEL_WARNING, "Attempted to get eventbox of collider {} that does not exist!", getIdIndex(entityId));
        return {};
    }
    return getCollider(entityId).hitbox;
}*/

/*bool PhysicsObject2D::colliderShapesMerged (physics::ColliderId entityId) {
    if (!colliderExists(entityId)) {
        logging::log(LEVEL_WARNING, "Attempted to get shapes merged of collider {} that does not exist!", getIdIndex(entityId));
        return false;
    }
    return getCollider(entityId).shapesMerged;
}*/

/*void PhysicsObject2D::setColliderShapesMerged (physics::ColliderId entityId, bool merged) {
    if (!colliderExists(entityId)) {
        logging::log(LEVEL_WARNING, "Attempted to set shapes merged of collider {} that does not exist!", getIdIndex(entityId));
    }

    getCollider(entityId).shapesMerged = merged;
}*/


void PhysicsObject2D::setShapeType (physics::ShapeId id, physics::PrimitiveShape shape) {
    if (shape == PrimitiveShape::CUSTOM) {
        // TODO reconsider?
        logging::log(LEVEL_WARNING, "Cannot set shape to custom through setShapeType()!");
        return;
    }
}

ShapeDataNew PhysicsObject2D::getShapeData (physics::ShapeId id) const {
    auto pubId = makePublicId(id);
    auto shapeType = shapeRegistry.getTypeIndex(pubId);
    auto shapeData = shapeRegistry.getComponentErased(pubId);
    return {shapeType, (void*)shapeData}; // TODO: const qualify
}

ShapeId PhysicsObject2D::makeNewHitbox (physics::ColliderId colliderId, std::size_t typeIndex, void* request) {
    if (!colliderExists(colliderId)) {
        logging::log(LEVEL_WARNING, "Attempted to make new hitbox for colliderId {} that does not exist!", getIdIndex(colliderId));
        return {};
    }

    auto& collider = getCollider(colliderId);
    if (collider.hitbox) {
        shapeRegistry.remove(makePublicId(collider.hitbox));
    }

    auto id = makeShapeFromRequest(shapeRegistry, colliderId, typeIndex, request);

    collider.hitbox = id;

    return id;
}

/*ShapeId PhysicsObject2D::makeNewEventbox (physics::ColliderId colliderId, std::size_t typeIndex, void* request) {
    if (!colliderExists(colliderId)) {
        logging::log(LEVEL_WARNING, "Attempted to make new hitbox for colliderId {} that does not exist!", getIdIndex(colliderId));
        return {};
    }

    auto& collider = getCollider(colliderId);
    if (collider.eventbox) {
        shapeRegistry.remove(makePublicId(collider.hitbox));
    }

    auto entityId = makeShapeFromRequest(shapeRegistry, colliderId, typeIndex, request);

    collider.eventbox = entityId;

    return entityId;
    return {};
}*/

void PhysicsObject2D::addEventHandlers (const event::EventBus::SharedPtr& eventBus) {
    // TODO: pass through deserialiser instead
    scope = eventBus->getScope();
    eventBus->subscribe<event::EntityCreationEvent>([this] (event::EntityCreationEvent& event) {
        event.entityView.get<CollisionComponent2D>().ifPresent([this, &event] (CollisionComponent2D& comp) {
             if (!comp.collider) {
                 return;
             }

             auto& coll = getCollider(comp.collider);
             coll.entityId = event.entityView.id();
         });
    }, scope);

    eventBus->subscribe<event::DebugRenderEvent>([this] (const event::DebugRenderEvent& event) {
        debugColliderRender = event.doRender;
    }, scope);
}

void PhysicsObject2D::debugRender (const component::EntityComponentManager& componentManager) {
    if (debugColliderRender) {
        // Debug render
        for (const auto& [collComp, transform]: componentManager.iterate<CollisionComponent2D, common::GlobalTransform2D>()) {
            auto collider = getCollider(collComp.collider);
            shapeRegistry.getComponent<BoxShape2D>(makePublicId(collider.hitbox)).ifPresent(
                    [&transform] (const BoxShape2D& shape) {
                        auto pos1 = shape.getTransform() * glm::vec2{-1, -1} + transform.transform2D.position();
                        auto pos2 = shape.getTransform() * glm::vec2{1, -1} + transform.transform2D.position();
                        auto pos3 = shape.getTransform() * glm::vec2{1, 1} + transform.transform2D.position();
                        auto pos4 = shape.getTransform() * glm::vec2{-1, 1} + transform.transform2D.position();
                        //common::debugWorldRect(pos1, pos2, pos3, pos4, {1, 0, 0, 0.5}, {0, 0, 1, 1});
                        common::debugWorldRectOutline(pos1, pos2, pos3, pos4, {0, 0, 1, 1});
                    });
        }
    }
}

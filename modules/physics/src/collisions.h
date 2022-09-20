#pragma once

#include "physics/components/2D/collision_component.h"
#include <vector>

namespace physics {
    //void checkCollisionsEntity (CollisionComponent2D* comp, component::EntityId* ids, int numEntities, [[maybe_unused]] int direction, std::vector<std::tuple<component::EntityId, component::EntityId, glm::vec2>>* collisionResults);

    void checkCollisionsEntity (const component::EntityComponentManager::SharedPtr& compManager, std::vector<std::tuple<component::EntityId, component::EntityId, glm::vec2>>& collisionResults);

    // TODO: move to specialised maths help header
    inline float squaredDistance (glm::vec2 vec) {
        return glm::dot(vec, vec);
    }
    inline glm::vec2 projectVec (glm::vec2 onto, glm::vec2 vec) {
        return glm::dot(onto, vec) / squaredDistance(onto) * onto;
    }
}

#include <vector>
#include <algorithm>

#include "physics.h"
#include "physics_body.h"
#include "physics_equations.h"

using namespace physics;
void physics::PhysicsBody::updatePosition (float time) {
    if (forceUpdate) {
        ForceEquation** equations = resolveForces(forces, mass, *x, *y, velX, velY);
        xEquation = equations[0];
        yEquation = equations[1];
    }
    // TODO: account for intercepts with v = 0
    *x = xEquation->getPosition(time);
    *y = yEquation->getPosition(time);
    velX = xEquation->getVelocity(time);
    velY = yEquation->getVelocity(time);

    xEquation->updateTime(time);
    yEquation->updateTime(time);
}

void physics::PhysicsBody::addForce (Force f) {
    forces.push_back(f);
    forceUpdate = true;
}
void physics::PhysicsBody::removeMovementForces () {
    forces.erase(std::remove_if(forces.begin(), forces.end(), [](const Force& f){
        return f.type == MOVEMENT_FORCE_TYPE;
    }), forces.end());
    forceUpdate = true;
}
void physics::PhysicsBody::setVelocity (float velX, float velY) {
    this->velX = velX;
    this->velY = velY;
    forceUpdate = true;
}
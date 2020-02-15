#include "component/component.h"
#include "component/main_component.h"
#include "event/events/entity_creation.h"
#ifndef PHYSICS_NEW_H
#define PHYSICS_HEW_H
namespace physics {
    void updatePhysics (component::EntityMainComponent* comp, int numEntities, int direction);
    void onEntityCreation (event::EntityCreationEvent* event);
}
#endif
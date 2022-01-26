#include "component/rotation_component.h"
#include "component/component.h"
#include "event/event_bus.h"
#ifndef ROTATION_UPDATE_H
#define ROTATION_UPDATE_H
namespace component {
    void rotateEntity (int entityId, float newRotation, EntityComponentManager::SharedPtr manager, const event::EventBus::SharedPtr& eventBus);
    void rotateEntityBy (int entityId, float deltaTheta, EntityComponentManager::SharedPtr manager, const event::EventBus::SharedPtr& eventBus);
}
#endif
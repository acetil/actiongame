#include <string>
#include <utility>
#include <math.h>

#include "game_object.h"
#include "logging/logging.h"
#include "physics/physics.h"
#include "event/events/entity_creation.h"
#include "entity/controller/entity_controller.h"
#include "component/rotation_update.h"
#include "game/entity/entity_type_functions.h"

using namespace game;

game::GameObject::~GameObject () {
    for (auto const& x : entityRegistry) {
        delete x.second;
    }
    /*for (auto const& x : entities) {
        delete x.second;
    }*/
    for (auto& i : tileRegistry) {
        delete i;
    }
    delete eventBus;
}

/*void game::GameObject::registerEntity (const std::string& name, AbstractEntity* entity) {
    if (entityRegistry.count(name) > 0) {
        logging::logf(LEVEL_WARNING, "Duplicate registry of entity with name '%s'!", name.c_str());
        return;
    }
    entityRegistry[name] = entity;
    logging::logf(LEVEL_INFO, "Registered entity with name %s", name.c_str());
}

[[maybe_unused]] AbstractEntity* game::GameObject::getEntity (const std::string& name) {
    if (entityRegistry.count(name) > 0) {
        return entityRegistry[name];
    } else {
        return nullptr;
    }
}*/
int game::GameObject::createNewEntityInstance (const std::string& name, float x, float y) {
    // TODO: requires refactor
    if (entityTypes.count(name) == 0) {
        logging::log(LEVEL_WARNING, "Attempted creation of entity with name '{}' which doesn't exist!", name);
        return -1;
    } else {
        int entityId = entityComponentManager->addObject();
        setInitialEntityValues(entityComponentManager, entityTypes[name], entityId, x, y);
        auto viewCore = view::ViewCore(entityComponentManager);

        eventBus->raiseEvent(event::EntityCreationEvent(x, y, 0.1f, entityComponentManager,
                                                        entityComponentManager->getObjectData<AbstractEntity*>(entityId), entityId,
                             view::EntityView(viewCore, entityId, eventBus), view::GameView(this)));
        return entityId;
    }
}
/*AbstractEntity* game::GameObject::getEntityInstance (int entityId) {
    if (entities.count(entityId) > 0) {
        return entities[entityId];
    } else {
        return NULL;
    }
}
void game::GameObject::deleteEntityInstance (AbstractEntity* entity) {
    int entityId = entity->getEntityId();
    delete entity;
    entities[entityId] = NULL;
    entities.erase(entityId);
}
void game::GameObject::deleteEntityInstance (int entityId) {
    deleteEntityInstance(entities[entityId]);
}*/

void game::GameObject::registerTile (Tile* tile) {
    if (tileMap.count(tile->getName()) > 0) {
        logging::log(LEVEL_WARNING, "Duplicate registry of tile with name '{}'!", tile->getName());
        return;
    }
    int pos = tileRegistry.size();
    tileRegistry.push_back(tile);
    tileMap[tile->getName()] = pos;
}
int game::GameObject::getTileId (const std::string& name) {
    return tileMap[name];
}
Tile* game::GameObject::getTile (int tileId) {
    if (tileRegistry.size() < tileId || tileId < 0) {
        return nullptr;
    }
    return tileRegistry[tileId];
}
Tile* game::GameObject::getTile (const std::string& name) {
    // convenience function
    return getTile(getTileId(name));
}

/*void game::GameObject::updateEntities (float deltaTime) {
    for (auto const& it : entities) {
        it.second->update(deltaTime);
    }
}*/
void game::GameObject::setTextureIds (graphics::TextureAtlas atlas) {
    for (auto const& it : controllers) {
        it.second->setTextureIds(atlas);
    }
}
event::EventBus* game::GameObject::getEventBus () {
    return eventBus;
}
void game::GameObject::setEntityComponentManager (component::EntityComponentManager* manager) {
    this->entityComponentManager = manager;
}
void game::GameObject::updateEntityPosition () {
    physics::updatePhysics(entityComponentManager);
    physics::checkCollisions(entityComponentManager, eventBus, view::GameView(this));
}
void entityPrePhysicsFunc (AbstractEntity** entities, int numEntities, int direction, component::EntityComponentManager* manager, event::EventBus* bus, view::GameView gameView) {
    controlEntitiesPrePhysics(manager, gameView, 0, numEntities, direction, bus);
}
void entityPostPhysicsFunc (AbstractEntity** entities, int numEntities, int direction, component::EntityComponentManager* manager, event::EventBus* bus, view::GameView gameView) {
    controlEntitiesPostPhysics(manager, gameView, 0, numEntities, direction, bus);
}
void game::GameObject::updateEntitiesPrePhysics () {
    // TODO: make better way
    auto gameView = view::GameView(this);
    entityComponentManager->applyFunc<AbstractEntity*>(entityPrePhysicsFunc, entityComponentManager, eventBus, gameView);
}

void GameObject::updateEntitiesPostPhysics () {
    // TODO: make better way
    auto gameView = view::GameView(this);
    entityComponentManager->applyFunc<AbstractEntity*>(entityPostPhysicsFunc, entityComponentManager, eventBus, gameView);
}

void GameObject::deleteEntityInstance (int entityId) {
    entityComponentManager->removeObject(entityId); // TODO: implement queue
}

void GameObject::registerEntityType (const std::string& name, EntityTypeBuilder entityTypeBuilder) {
    entityTypeBuilders[name] = std::move(entityTypeBuilder);
}

void GameObject::buildEntityTypes () {
    for (auto [name, builder] : entityTypeBuilders) {
        entityTypes[name] = builder.build(controllers);
    }
}

EntityController* GameObject::getController (const std::string& name) {
    return controllers[name];
}

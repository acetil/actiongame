#include <exception>

#include "component/component.h"

#include "engine/engine.h"
#include "engine/game_init.h"
#include "engine/entity/entities.h"
#include "engine/entity/controller/entity_controller.h"
#include "component/component_serializer.h"

#include "graphics/graphics_init.h"
#include "graphics/graphics_headers.h"
#include "graphics/ui/debug_ui.h"

#include "physics/physics.h"

#include "logging/logging.h"
#include "component/component_serialiser.h"
#include "common/components/2d/global_transform.h"
#include "common/components/2d/global_transform_serialize.h"
#include "physics/physics.h"

using namespace engine;

class engine::detail::Engine {
private:
    event::EventBus::SharedPtr eventBus;
    game::PhenylGameHolder gameObjHolder;
    graphics::PhenylGraphicsHolder graphicsHolder;
    component::EntityComponentManager componentManager;
    std::unique_ptr<component::EntitySerializer> entitySerializer;
    std::unique_ptr<physics::IPhysics> physicsObj;
    void addEventHandlers ();
    void addDefaultSerialisers ();
    void addComponents ();
public:
    Engine ();
    ~Engine();

    [[nodiscard]] game::detail::GameObject::SharedPtr getGameObjectTemp () const;
    [[nodiscard]] game::PhenylGame getGameObject () const;
    [[nodiscard]] graphics::PhenylGraphics getGraphics () const;
    component::EntityComponentManager& getComponentManager ();
    [[nodiscard]] const component::EntityComponentManager& getComponentManager () const;
    event::EventBus::SharedPtr getEventBus ();
    component::EntitySerializer& getEntitySerializer ();
    physics::PhenylPhysics getPhysics ();

    void updateEntityPosition (float deltaTime);
    void debugRender ();
};

engine::PhenylEngine::PhenylEngine () {
    internal = std::make_unique<engine::detail::Engine>();
}

engine::PhenylEngine::~PhenylEngine () = default;

game::detail::GameObject::SharedPtr engine::PhenylEngine::getGameTemp () {
    return internal->getGameObjectTemp();
}

graphics::PhenylGraphics PhenylEngine::getGraphics () {
    return internal->getGraphics();
}

game::PhenylGame PhenylEngine::getGame () {
    return internal->getGameObject();
}

event::EventBus::SharedPtr PhenylEngine::getEventBus () {
    return internal->getEventBus();
}

component::EntityComponentManager& PhenylEngine::getComponentManager () {
    return internal->getComponentManager();
}

component::EntitySerializer& PhenylEngine::getEntitySerializer () {
    return internal->getEntitySerializer();
}

physics::PhenylPhysics PhenylEngine::getPhysics () {
    return internal->getPhysics();
}

void PhenylEngine::updateEntityPosition (float deltaTime) {
    internal->updateEntityPosition(deltaTime);
}

void PhenylEngine::debugRender () {
    internal->debugRender();
}

engine::detail::Engine::Engine () : componentManager{256}{
    eventBus = event::EventBus::NewSharedPtr();
    entitySerializer = std::make_unique<component::EntitySerializer>();
    physicsObj = physics::makeDefaultPhysics();
    addEventHandlers();

    auto gameObj = gameObjHolder.getGameObject();
    auto graphics = graphicsHolder.getGraphics();

    gameObj.setEntityComponentManager(&componentManager);
    gameObj.setSerializer(entitySerializer.get());

    addComponents();

    addDefaultSerialisers();
    gameObj.addDefaultSerialisers();
    graphics.addComponentSerializers(getEntitySerializer());
    physicsObj->addComponentSerializers(getEntitySerializer());
    //physics::addComponentSerialisers(getEntitySerialiser());

    // TODO: move all to user
    //graphics.getTextureAtlas("sprite").ifPresent([&gameObj](auto& atlas){gameObj.setTextureIds(atlas);});
    graphics::addMapRenderLayer(graphics, eventBus);
    graphics.addEntityLayer(&componentManager); // TODO: unhackify
    graphics.getUIManager().addRenderLayer(graphics.tempGetGraphics(), graphics.getRenderer());

    gameObjHolder.initGame(graphics, eventBus);

    gameObjHolder.getGameObject().getGameInput().addInputSources(graphicsHolder.tempGetGraphics()->getInputSources());
}

engine::detail::Engine::~Engine () {
    logger::log(LEVEL_INFO, "MAIN", "Shutting down!");
    gameObjHolder.tempGetGameObject()->clearPrefabs();
    componentManager.clear();
}

game::detail::GameObject::SharedPtr detail::Engine::getGameObjectTemp () const {
    return gameObjHolder.tempGetGameObject();
}

graphics::PhenylGraphics detail::Engine::getGraphics () const {
    return graphicsHolder.getGraphics();
}

game::PhenylGame detail::Engine::getGameObject () const {
    return gameObjHolder.getGameObject();
}

event::EventBus::SharedPtr detail::Engine::getEventBus () {
    return eventBus;
}

void detail::Engine::addEventHandlers () {
    graphicsHolder.getGraphics().addEventHandlers(eventBus);
    gameObjHolder.getGameObject().addEventHandlers(eventBus);

    eventBus->subscribeUnscoped(game::addEntities);
    //eventBus->subscribeUnscoped(physics::onEntityCreation);

    graphics::addDebugEventHandlers(eventBus);

    game::addControlEventHandlers(eventBus);
    physicsObj->addEventHandlers(eventBus);
}

component::EntityComponentManager& detail::Engine::getComponentManager () {
    return componentManager;
}

const component::EntityComponentManager& detail::Engine::getComponentManager () const {
    return componentManager;
}

component::EntitySerializer& detail::Engine::getEntitySerializer () {
    return *entitySerializer;
}

void detail::Engine::addDefaultSerialisers () {
    //entitySerialiser->addComponentSerialiser<component::Position2D>("Position2D");
    entitySerializer->addSerializer<common::GlobalTransform2D>();
}

physics::PhenylPhysics detail::Engine::getPhysics () {
    return physics::PhenylPhysics(physicsObj.get());
}

void detail::Engine::updateEntityPosition (float deltaTime) {
    // TODO: remove function?
    physicsObj->updatePhysics(getComponentManager(), deltaTime);
    view::GameView gameView{gameObjHolder.tempGetGameObject().get()}; // TODO
    physicsObj->checkCollisions(getComponentManager(), getEventBus(), gameView, deltaTime);
}

void detail::Engine::debugRender () {
    physicsObj->debugRender(getComponentManager());
}

void detail::Engine::addComponents () {
    componentManager.addComponent<common::GlobalTransform2D>();
    physicsObj->addComponents(componentManager);
    graphicsHolder.tempGetGraphics()->addComponents(componentManager);
}

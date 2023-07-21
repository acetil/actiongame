#include "entity_layer.h"
#include "common/components/2d/global_transform.h"

#include <utility>


#define BUFFER_SIZE 100 * 2 * 6

#define NUM_TRIANGLE_VERTICES 3
//#define TRIANGLES_PER_SPRITE 2
//#define NUM_POS_PER_VERTEX 2

using namespace graphics;

static void bufferPosDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2>& buffer);
static void bufferUvDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2> buffer);
static void bufferActualPosDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2>& offsetBuffer, Buffer<glm::mat2>& transformBuffer);

namespace graphics {
    class EntityPipeline : public Pipeline<component::EntityComponentManager> {
    private:
        PipelineStage renderStage;
        ShaderProgramNew shader;
        Buffer<glm::vec2> posBuffer;
        Buffer<glm::vec2> uvBuffer;
        Buffer<glm::vec2> offsetBuffer;
        Buffer<glm::mat2> transformBuffer;

    public:
        EntityPipeline () = default;

        explicit EntityPipeline (const ShaderProgramNew& _shader) : shader{_shader} {};

        void init (Renderer* renderer) override {
            renderStage = std::move(renderer->buildPipelineStage(PipelineStageBuilder(shader)
                                                               .addVertexAttrib<glm::vec2>(0)
                                                               .addVertexAttrib<glm::vec2>(1)
                                                               .addVertexAttrib<glm::vec2>(2)
                                                               .addVertexAttrib<glm::mat2>(3)));

            posBuffer = renderer->makeBuffer<glm::vec2>(BUFFER_SIZE);
            uvBuffer = renderer->makeBuffer<glm::vec2>(BUFFER_SIZE);
            offsetBuffer = renderer->makeBuffer<glm::vec2>(BUFFER_SIZE);
            transformBuffer = renderer->makeBuffer<glm::mat2>(BUFFER_SIZE);

            renderStage.bindBuffer(0, posBuffer);
            renderStage.bindBuffer(1, uvBuffer);
            renderStage.bindBuffer(2, offsetBuffer);
            renderStage.bindBuffer(3, transformBuffer);
        }

        void applyCamera (graphics::Camera& camera) {
            renderStage.applyUniform(camera.getUniformName(), camera.getCamMatrix());
        }

        void bufferData (component::EntityComponentManager& manager) override {
            /*posBuffer.clearData();
            uvBuffer.clearData();
            offsetBuffer.clearData();
            transformBuffer.clearData();*/
            renderStage.clearBuffers();


            bufferPosDataNew(manager, posBuffer);
            bufferUvDataNew(manager, uvBuffer);
            bufferActualPosDataNew(manager, offsetBuffer, transformBuffer);

            /*posBuffer.bufferData();
            uvBuffer.bufferData();
            offsetBuffer.bufferData();
            transformBuffer.bufferData();*/
            renderStage.bufferAllData();
        }

        void render () override {
            renderStage.render();
        }
    };
}


//void bufferPosData (Model2D* comp, int numEntities, [[maybe_unused]] int direction, std::pair<Buffer*, GlobalTransform2D*> tup);
//static void bufferPosData (const component::EntityComponentManager::SharedPtr& manager, Buffer* buffers);
//void bufferUvData (Model2D* comp, int numEntities, [[maybe_unused]] int direction, Buffer* buf);
//static void bufferUvData (const component::EntityComponentManager::SharedPtr& manager, Buffer* buffer);
//void bufferActualPosData (GlobalTransform2D* comp, int numEntities, [[maybe_unused]] int direction, Buffer* bufs);
//static void bufferActualPosData (const component::EntityComponentManager::SharedPtr& manager, Buffer* buffer);
std::string EntityRenderLayer::getName () {
    return "entity_layer";
}

int EntityRenderLayer::getPriority () {
    return 1;
}

bool graphics::EntityRenderLayer::isActive () {
    return active;
}

void EntityRenderLayer::gatherData () {

}

void EntityRenderLayer::preRender (graphics::Renderer* renderer) {
    /*componentManager->applyFunc<Model2D>(bufferPosData, std::pair(&buffers[0],
                                                            componentManager->get<GlobalTransform2D>().orElse(nullptr)));*/
    //bufferPosData(componentManager, &buffers[0]);

    //componentManager->applyFunc<Model2D>(bufferUvData, &buffers[1]);

    //bufferUvData(componentManager, &buffers[1]);

    /*for (auto i : *componentManager) {
        i.applyFunc<GlobalTransform2D, component::Rotation2D>([](GlobalTransform2D& absPos, component::Rotation2D& rotComp) {
            absPos.transform *= rotComp.rotMatrix;
        });
    }*/
    /*for (auto [transform, rotation] : componentManager->iterate<GlobalTransform2D, component::Rotation2D>()) {
        transform.rotTransform = transform.transform * rotation.rotMatrix;
    }*/

    /*componentManager->applyFunc<GlobalTransform2D, component::Rotation2D>([](GlobalTransform2D* absPos, component::Rotation2D* rotComp, int numEntities, int) {
        for (int i = 0; i < numEntities; i++) {
            absPos[i].transform *= rotComp[i].rotMatrix;
        }
    });*/

    //componentManager->applyFunc<GlobalTransform2D>(bufferActualPosData, &buffers[2]);
    //bufferActualPosData(componentManager, &buffers[2]);

    entityPipeline->bufferData(*componentManager);

    /*componentManager->applyFunc<GlobalTransform2D, component::Rotation2D>([](GlobalTransform2D* absPos, component::Rotation2D* rotComp, int numEntities, int) {
        for (int i = 0; i < numEntities; i++) {
            absPos[i].transform *= glm::inverse(rotComp[i].rotMatrix);
        }
    });*/
    /*for (auto i : *componentManager) {
        i.applyFunc<GlobalTransform2D, component::Rotation2D>([](GlobalTransform2D& absPos, component::Rotation2D& rotComp) {
            absPos.transform *= glm::inverse(rotComp.rotMatrix);
        });
    }*/
    /*for (const auto& i :  componentManager->iterate<GlobalTransform2D, component::Rotation2D>()) {
        i.get<GlobalTransform2D>().transform *= glm::inverse(i.get<component::Rotation2D>().rotMatrix);
    }*/

    //numTriangles = buffers[0].currentSize() / sizeof(float) / 2 / 3;
    //renderer->bufferData(buffIds, buffers);
}

int EntityRenderLayer::getUniformId (std::string uniformName) {
    return 0;
}

void EntityRenderLayer::applyUniform (int uniformId, void* data) {

}

void EntityRenderLayer::applyCamera (graphics::Camera camera) {
    //shaderProgram.bind();
    //shaderProgram.applyUniform(camera.getUniformName(), camera.getCamMatrix());
    entityPipeline->applyCamera(camera);
}

void EntityRenderLayer::render (graphics::Renderer* renderer, graphics::FrameBuffer* frameBuf) {
    //shaderProgram.bind();
    //frameBuf->bind();
    //printf("Num triangles: %d\n", numTriangles);
    //renderer->render(buffIds, shaderProgram, numTriangles); // TODO: Make better
    entityPipeline->render();
}

EntityRenderLayer::EntityRenderLayer (graphics::Renderer* renderer,
                                                component::EntityComponentManager*componentManager) : shaderProgram{renderer->getProgramNew("default").orThrow()}, componentManager{componentManager} {
    //this->shaderProgram = renderer->getProgram("entity").value();
    //this->shaderProgram = renderer->getProgramNew("default").orThrow(); // TODO
    /*this->buffIds = renderer->getBufferIds(5, BUFFER_SIZE * 2 * sizeof(float), {2, 2, 2, 2, 2});
    this->buffers[0] = Buffer(BUFFER_SIZE * 2, sizeof(float), false);
    this->buffers[1] = Buffer(BUFFER_SIZE * 2, sizeof(float), false);
    this->buffers[2] = Buffer(BUFFER_SIZE * 2, sizeof(float), false);
    this->buffers[3] = Buffer(BUFFER_SIZE * 4, sizeof(float), false);
    this->buffers[4] = Buffer(BUFFER_SIZE * 4, sizeof(float), false); // TODO: better way to input matrices*/

    this->entityPipeline = std::make_unique<EntityPipeline>(shaderProgram);
    this->entityPipeline->init(renderer);
}

EntityRenderLayer::~EntityRenderLayer () = default;

/*void bufferPosData (Model2D* comp, int numEntities, [[maybe_unused]] int direction, std::pair<Buffer*, GlobalTransform2D*> tup) {
    auto [buf, modComp] = tup;
    for (int i = 0; i < numEntities; i++) {
        //logging::logf(LEVEL_DEBUG, "Num vertices: %d", comp[i].positionData.size());
        //logging::log(LEVEL_DEBUG, "Buffering entity pos data!");
        //auto ptr = buffer->getVertexBufferPos();
        /*glm::vec2 vertices[4];
        vertices[0] = comp->pos;
        vertices[1] = comp->pos + comp->vec1;
        vertices[2] = comp->pos + comp->vec2;
        vertices[3] = comp->pos + comp->vec1 + comp->vec2;
        for (int j = 0; j < NUM_TRIANGLE_VERTICES * TRIANGLES_PER_SPRITE; j++) {
            int correctedVertex = j % NUM_TRIANGLE_VERTICES + j / NUM_TRIANGLE_VERTICES;
            buffer->pushData(&vertices[correctedVertex].x, 2);
        } *//*

        buf->pushData(comp->positionData.begin(), comp->positionData.size());
        modComp->vertices = comp->positionData.size() / 2;
        comp++;
        modComp++;
    }

}*/
static void bufferPosDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2>& buffer) {
    for (auto [model, transform] : manager.iterate<Model2D, common::GlobalTransform2D>()) {
        //auto& model = i.get<Model2D>();
        buffer.pushData(model.positionData.begin(), model.positionData.end());
        //transform.vertices = model.positionData.size();
    }
}
/*void bufferUvData (Model2D* comp, int numEntities, [[maybe_unused]] int direction, Buffer* buf) {
    for (int i = 0; i < numEntities; i++) {
        buf->pushData(comp->uvData.begin(), comp->uvData.size());
        comp++;
    }
}*/

static void bufferUvDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2> buffer) {
    for (const auto& model : manager.iterate<Model2D>()) {
        buffer.pushData(model.uvData.begin(), model.uvData.end());
    }
}
/*void bufferActualPosData (GlobalTransform2D* comp, int numEntities, [[maybe_unused]] int direction, Buffer* bufs) {
    for (int i = 0; i < numEntities; i++) {
        //logging::logf(LEVEL_DEBUG, "Abs pos: <%f, %f>", comp->pos.x, comp->pos.y);
        for (int j = 0; j < comp->vertices; j++) {
            bufs->pushData(&comp->pos[0], 2);
            (bufs + 1)->pushData(&comp->transform[0][0], 2);
            (bufs + 2)->pushData(&comp->transform[1][0], 2);
            //buf.second->pushData(&comp->transform[2], 2); // TODO: update buffer implementation
        }
        comp++;
    }
}*/

static void bufferActualPosDataNew (const component::EntityComponentManager& manager, Buffer<glm::vec2>& offsetBuffer, Buffer<glm::mat2>& transformBuffer) {
    for (const auto& [transform, model] : manager.iterate<common::GlobalTransform2D, Model2D>()) {
        for (int j = 0; j < model.positionData.size(); j++) {
            offsetBuffer.pushData(transform.transform2D.position());
            //(buffer + 1)->pushData(&absPos.transform[0][0], 2);
            //(buffer + 2)->pushData(&absPos.transform[1][0], 2); // TODO: update buffer implementation
            transformBuffer.pushData(transform.transform2D.getMatrix());
        }
    }
}
#include "graphics_layer.h"
#include "map_layer.h"
#include "entity_layer.h"

using namespace graphics;

GraphicsRenderLayer::GraphicsRenderLayer (Renderer* renderer) {
    //program = renderer->getProgram("graphics_program").value();
    program = renderer->getProgram("default").value(); // TODO
    renderLayers.push_back(new MapRenderLayer(renderer));
    //renderLayers.push_back(new EntityRenderLayer());

}


std::string GraphicsRenderLayer::getName() {
    return "graphics_layer";
}

int GraphicsRenderLayer::getPriority() {
    return 1;
}

bool GraphicsRenderLayer::isActive() {
    return active;
}

void GraphicsRenderLayer::gatherData () {
    for (auto i : renderLayers) {
        if (!i->isActive()) {
            continue;
        }
        i->gatherData();
    }
}

void GraphicsRenderLayer::preRender(Renderer *renderer) {
    for (auto i : renderLayers) {
        if (!i->isActive()) {
            continue;
        }
        i->preRender(renderer);
    }
}

void GraphicsRenderLayer::render(Renderer *renderer, FrameBuffer *frameBuf) {
    for (auto i : renderLayers) {
        if (!i->isActive()) {
            continue;
        }
        i->render(renderer, frameBuf); // TODO
    }
}

void GraphicsRenderLayer::applyCamera(Camera camera) {
    for (auto i : renderLayers) {
        if (!i->isActive()) {
            continue;
        }
        i->applyCamera(camera);
    }
}

int GraphicsRenderLayer::getUniformId(std::string uniformName) {
    return 0; // TODO
}

void GraphicsRenderLayer::applyUniform(int uniformId, void *data) {
    // TODO
}

std::optional<RenderLayer*> GraphicsRenderLayer::getRenderLayer (const std::string& layer) {
    for (auto i : renderLayers) {
        if (i->getName() == layer) {
            // TODO: optimize
            return std::optional(i);
        }
    }
    return std::nullopt;
}

void GraphicsRenderLayer::addRenderLayer (RenderLayer* layer) {
    renderLayers.push_back(layer);
}

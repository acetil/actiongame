#include "graphics/ui/debug_ui.h"

#include <utility>
#include <queue>

#include "core/delta_time.h"
#include "util/profiler.h"
#include "util/smooth_queue.h"
#include "core/assets/assets.h"
#include "core/debug.h"
#include "core/runtime.h"
#include "graphics/plugins/ui_plugin.h"

using namespace phenyl;

std::string_view graphics::ProfileUiPlugin::getName () const noexcept {
    return "ProfileUiPlugin";
}

void graphics::ProfileUiPlugin::init (core::PhenylRuntime& runtime) {
    runtime.addPlugin<graphics::UIPlugin>();
    runtime.addResource<core::DebugRenderConfig>();

    runtime.addSystem<core::Update>("ProfileUiPlugin::update", this, &ProfileUiPlugin::update);
    runtime.addSystem<core::Render>("ProfileUiPlugin::render", this, &ProfileUiPlugin::render);
}

void graphics::ProfileUiPlugin::update (core::PhenylRuntime& runtime) {
    auto deltaTime = runtime.resource<core::DeltaTime>()();

    deltaTimeQueue.pushPop((float)deltaTime);
    frameQueue.pushPop(util::getProfileFrameTime());
    graphicsQueue.pushPop(util::getProfileTime("graphics"));
    physicsQueue.pushPop(util::getProfileTime("physics"));
}

void graphics::ProfileUiPlugin::render (core::PhenylRuntime& runtime) {
    if (!runtime.resource<core::DebugRenderConfig>().doProfileRender) {
        return;
    }

    auto& canvas = runtime.resource<Canvas>();

    canvas.renderText(glm::vec2{5, 15}, canvas.defaultFont(), 11, "physics: " + std::to_string(physicsQueue.getSmoothed() * 1000) + "ms");
    canvas.renderText(glm::vec2{5, 30}, canvas.defaultFont(), 11, "graphics: " + std::to_string(graphicsQueue.getSmoothed() * 1000) + "ms");
    canvas.renderText(glm::vec2{5, 45}, canvas.defaultFont(), 11, "frame time: " + std::to_string(frameQueue.getSmoothed() * 1000) + "ms");

    canvas.renderText(glm::vec2{700, 15}, canvas.defaultFont(), 11, std::to_string(1.0f / deltaTimeQueue.getSmoothed()) + " fps", {0.0f, 1.0f, 0.0f});
}

#include "graphics/plugins/ui_plugin.h"
#include "graphics/plugins/graphics_plugin.h"

using namespace phenyl::graphics;

std::string_view UIPlugin::getName () const noexcept {
    return "UIPlugin";
}

void UIPlugin::init (phenyl::runtime::PhenylRuntime& runtime) {
    runtime.addPlugin<GraphicsPlugin>();

    manager = std::make_unique<UIManager>(runtime.resource<Renderer>());

    runtime.addResource(manager.get());
}

void UIPlugin::frameBegin (runtime::PhenylRuntime& runtime) {
    manager->setMousePos(runtime.resource<Renderer>().getViewport().getCursorPos());
    manager->updateUI();
}

void UIPlugin::render (runtime::PhenylRuntime& runtime) {
    manager->renderUI();
}

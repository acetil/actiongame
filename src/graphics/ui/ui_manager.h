#include <unordered_map>
#include <string>

#include "graphics/font/font_manager.h"
#include "graphics/buffer.h"
#include "graphics/font/font.h"

#ifndef UI_MANAGER_H
#define UI_MANAGER_H
namespace graphics {
    #ifndef RENDERER_H
    class Renderer;
    #endif

    #ifndef UI_LAYER_H
    class UIRenderLayer;
    #endif

    #ifndef GRAPHICS_H
    class Graphics;
    #endif

    class UIManager {
    private:
        FontManager fontManager;
        std::unordered_map<std::string, Font> fonts;
        UIRenderLayer* uiLayer;
    public:
        UIManager(Renderer* renderer, FontManager& _fontManager);
        void renderText(const std::string& font, const std::string& text, int size, int x, int y);
        void addRenderLayer (Graphics* graphics, Renderer* renderer);
    };
}
#endif

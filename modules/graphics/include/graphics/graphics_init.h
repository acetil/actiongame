#include "graphics/graphics_headers.h"
#include "graphics/graphics_new_include.h"
#include "graphics/graphics.h"

#ifndef GRAPHICS_INIT_H
    #define GRAPHICS_INIT_H
    #define DEFAULT_WINDOW_X 800
    #define DEFAULT_WINDOW_Y 600
    #define DEFAULT_WINDOW_NAME "Action game"
    #define GRAPHICS_INIT_SUCCESS 0
    #define GRAPHICS_INIT_FAILURE -1
    namespace graphics {
        int initWindow (GLFWwindow** windowPtr);
        int initGraphics (GLFWwindow* window, Graphics::SharedPtr& graphicsNew);
        void destroyGraphics (const Graphics::SharedPtr& graphics);
    }
#endif
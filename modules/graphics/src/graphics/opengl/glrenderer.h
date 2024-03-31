#pragma once

#include <unordered_map>
#include <memory>

#include "graphics/renderers/renderer.h"
#include "graphics/shaders/shaders.h"
#include "graphics/graphics_headers.h"

#include "glshader.h"
#include "graphics/glfw/glfw_viewport.h"


namespace phenyl::graphics {
    class GLRenderer : public Renderer {
    private:
        std::unique_ptr<GLFWViewport> viewport;
        util::Map<std::string, Shader> shaderProgramsNew;

        GLShaderManager shaderManager;

        common::Asset<Shader> boxShader;
        common::Asset<Shader> debugShader;
        common::Asset<Shader> spriteShader;
        common::Asset<Shader> textShader;
        common::Asset<Shader> particleShader;
    protected:
        std::unique_ptr<IBuffer> makeRendererBuffer (std::size_t startCapacity, std::size_t elementSize) override;
        std::unique_ptr<IUniformBuffer> makeRendererUniformBuffer (bool readable) override;
        std::unique_ptr<IImageTexture> makeRendererImageTexture (const TextureProperties& properties) override;
        std::unique_ptr<IImageArrayTexture> makeRendererArrayTexture(const TextureProperties &properties, std::uint32_t width, std::uint32_t height) override;

    public:
        static std::unique_ptr<GLRenderer> Make (const GraphicsProperties& properties);
        explicit GLRenderer (std::unique_ptr<GLFWViewport> viewport);

        void setupErrorHandling ();

        std::string_view getName() const noexcept override;

        double getCurrentTime () override;

        void clearWindow () override;
        void render () override;
        void finishRender () override;

        PipelineBuilder buildPipeline () override;
        void loadDefaultShaders () override;

        Viewport& getViewport () override;
        const Viewport& getViewport () const override;

    };
}

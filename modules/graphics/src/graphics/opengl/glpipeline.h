#pragma once

#include "util/map.h"

#include "graphics/pipeline.h"
#include "graphics/graphics_headers.h"
#include "graphics/shader.h"
#include "glshader.h"

namespace phenyl::graphics {
    class GlPipeline : public IPipeline {
    private:
        struct PipelineIndex {
            GLenum typeEnum;
            std::size_t typeSize;
        };

        GLuint vaoId;
        GLenum renderMode = GL_TRIANGLES;
        core::Asset<Shader> shader;
        std::vector<std::size_t> bufferTypes;
        util::Map<UniformBinding, std::size_t> uniformTypes;
        std::optional<PipelineIndex> indexType = std::nullopt;

        GlShader& getShader ();
    public:
        explicit GlPipeline ();
        GlPipeline (const GlPipeline&) = delete;
        GlPipeline (GlPipeline&& other) noexcept;

        GlPipeline& operator= (const GlPipeline&) = delete;
        GlPipeline& operator= (GlPipeline&& other) noexcept;

        ~GlPipeline () override;

        void bindBuffer (std::size_t type, BufferBinding binding, IBuffer& buffer) override;
        void bindIndexBuffer (ShaderIndexType type, IBuffer& buffer) override;
        void bindUniform (std::size_t type, UniformBinding binding, IUniformBuffer& buffer) override;
        void bindSampler (SamplerBinding binding, const ISampler& sampler) override;
        void unbindIndexBuffer () override;

        void render (std::size_t vertices, std::size_t offset) override;

        void setRenderMode (GLenum renderMode);
        void setShader (core::Asset<Shader> shader);

        BufferBinding addBuffer (std::size_t type, GLuint divisor);
        void addAttrib (GLenum type, GLint size, GLuint location, BufferBinding binding, std::size_t offset);

        UniformBinding addUniform (std::size_t type, unsigned int location);
        SamplerBinding addSampler (unsigned int location);

        GLuint getCurrDivisor () const;
    };

    class GlPipelineBuilder : public IPipelineBuilder {
    private:
        std::unique_ptr<GlPipeline> pipeline;
    public:
        GlPipelineBuilder ();

        void withGeometryType (GeometryType type) override;
        void withShader (core::Asset<Shader> shader) override;

        BufferBinding withBuffer(std::size_t type, std::size_t size, BufferInputRate inputRate) override;
        void withAttrib(ShaderDataType type, unsigned int location, BufferBinding binding, std::size_t offset) override;

        UniformBinding withUniform (std::size_t type, unsigned int location) override;
        SamplerBinding withSampler (unsigned int location) override;

        std::unique_ptr<IPipeline> build() override;
    };
}
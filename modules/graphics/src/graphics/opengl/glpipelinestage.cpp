#include "glpipelinestage.h"
#include "glbuffer.h"

#include "logging/logging.h"

using namespace graphics;

static void enableVertexAttribPointer (GLuint vaoId, GlBuffer* buffer, int location, GLenum type, GLsizei size, GLsizei stride=0, std::size_t offset=0);
static void setupVertexAttribPointer (GLuint vaoId, GlBuffer* buffer, int location, ShaderDataType attribType);

GLPipelineStage::GLPipelineStage (PipelineStageSpec& spec) {
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
    //std::size_t i = 0;
    for (auto& [k, v] : spec.vertexAttribs) {
        vertexAttribs[k] = {v, nullptr};
    }
}

void GLPipelineStage::bindBuffer (int location, ShaderDataType attribType, std::shared_ptr<RendererBufferHandle> handle) {
    if (vertexAttribs[location].first != attribType) {
        logging::log(LEVEL_ERROR, "Attempted to bind buffer of type {} to attribute of type {}!", getUniformTypeName(attribType),
                     getUniformTypeName(vertexAttribs[location].first));
        return;
    }

    auto glBuffer = std::dynamic_pointer_cast<GlBuffer>(handle);
    glBindVertexArray(vaoId);
    //glBuffer->bindBuffer();

    setupVertexAttribPointer(vaoId, glBuffer.get(), location, vertexAttribs[location].first);

    vertexAttribs[location].second = glBuffer;

    //glBuffer->onStageBind(shared_from_this(), vertexAttribs[location].second);
}

void GLPipelineStage::render () {
    std::size_t numVertices = -1;
    for (auto& [k, buf] : vertexAttribs) {
        auto n = buf().second->getNumElements();

        if (n < numVertices) {
            numVertices = n;
        }
    }

    //logging::log(LEVEL_DEBUG, "Drawing {} vertices!", numVertices);
    glBindVertexArray(vaoId);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(numVertices));
}

GLPipelineStage::~GLPipelineStage () {
    glDeleteVertexArrays(1, &vaoId);
}

void GLPipelineStage::clearBuffers () {
    for (auto& [k, buf] : vertexAttribs) {
        buf().second->clearBuffer();
    }
}

void GLPipelineStage::bufferData () {
    for (auto& [k, buf] : vertexAttribs) {
        buf().second->bufferData();
    }
}

static void setupVertexAttribPointer (GLuint vaoId, GlBuffer* buffer, int location, ShaderDataType attribType) {
    //glBindVertexArray(vaoId);
    switch (attribType) {
        case ShaderDataType::FLOAT:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 1);
            break;
        case ShaderDataType::INT:
            enableVertexAttribPointer(vaoId, buffer, location, GL_INT, 1);
            break;
        case ShaderDataType::VEC2F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 2);
            break;
        case ShaderDataType::VEC3F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 3);
            break;
        case ShaderDataType::VEC4F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 4);
            break;
        case ShaderDataType::MAT2F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 2, sizeof(float) * 4);
            enableVertexAttribPointer(vaoId, buffer, location + 1, GL_FLOAT, 2, sizeof(float) * 4, sizeof(float) * 2);
            break;
        case ShaderDataType::MAT3F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 3, sizeof(float) * 9);
            enableVertexAttribPointer(vaoId, buffer, location + 1, GL_FLOAT, 3, sizeof(float) * 9, sizeof(float) * 3);
            enableVertexAttribPointer(vaoId, buffer, location + 2, GL_FLOAT, 3, sizeof(float) * 9, sizeof(float) * 6);
            break;
        case ShaderDataType::MAT4F:
            enableVertexAttribPointer(vaoId, buffer, location, GL_FLOAT, 4, sizeof(float) * 16);
            enableVertexAttribPointer(vaoId, buffer, location + 1, GL_FLOAT, 4, sizeof(float) * 16, sizeof(float) * 4);
            enableVertexAttribPointer(vaoId, buffer, location + 2, GL_FLOAT, 4, sizeof(float) * 16, sizeof(float) * 8);
            enableVertexAttribPointer(vaoId, buffer, location + 3, GL_FLOAT, 4, sizeof(float) * 16, sizeof(float) * 12);
            break;
        default:
            logging::log(LEVEL_WARNING, "Unable to setup attrib pointer for shader data type {}", getUniformTypeName(attribType));
    }
}

static void enableVertexAttribPointer (GLuint vaoId, GlBuffer* buffer, int location, GLenum type, GLsizei size, GLsizei stride, std::size_t offset) {
    glBindVertexArray(vaoId);
    buffer->bindBuffer();
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, size, type, GL_FALSE, stride, (void*)offset);
}
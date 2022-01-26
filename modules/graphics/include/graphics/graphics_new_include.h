#include <stdio.h>
#include <string>
#include <utility>
#include <vector>
#include <utility>
#include <optional>

#include "graphics/shaders/shaders.h"
#include "component/serialisable_component.h"
#include "graphics/textures/image.h"
#include "logging/logging.h"
#include "camera.h"
#include "util/span.h"

//#include "buffer.h"
//#include "renderer.h"
#ifndef GRAPHICS_NEW_INCLUDE_H
#define GRAPHICS_NEW_INCLUDE_H
namespace graphics {
    struct TextureOffset {
        glm::vec2 offset;
        glm::mat2 recMat;
        TextureOffset (glm::vec2 off, glm::mat2 mat) : offset{off}, recMat{mat} {};
        TextureOffset () : TextureOffset ({0.0f, 0.0f}, {{1.0f, 0.0f}, {0.0f, 1.0f}}) {};
    };


    struct Model {
        std::string modelName;
        std::string texPath;
        std::vector<std::pair<TextureOffset, Image::SharedPtr>> textures;
        Model (std::string modelName, Image::SharedPtr image) {
            this->modelName = std::move(modelName);
            textures.emplace_back(TextureOffset(), image);
            texPath = "";
        }
    };

    struct FixedModel : component::SerialisableComponent<FixedModel> {
    private:
        static constexpr std::string_view _name = "fixed_model";
        static std::string_view const& getName () {
            return _name;
        }

        util::DataValue serialise ();
        void deserialise (util::DataValue const& val);

    public:
        util::span<float> positionData;
        util::span<float> uvData;
        std::string modelName;
        FixedModel () : positionData(), uvData() {}
        FixedModel (float* posPtr, float* uvPtr, int size, std::string name) {
            positionData = util::span(posPtr, size);
            uvData = util::span(uvPtr, size);
            modelName = std::move(name);
        }
        FixedModel (float* posStart, float* posEnd, float* uvStart, float* uvEnd, std::string name) {
            /*logging::logf(LEVEL_DEBUG, "Pos size: %ld", posEnd - posStart);
            logging::logf(LEVEL_DEBUG, "UV size: %ld", uvEnd - uvStart);*/
#ifndef NDEBUG
            assert(posEnd - posStart == uvEnd - uvStart);
#endif
            positionData = util::span(posStart, posEnd);
            uvData = util::span(uvStart, uvEnd);
            modelName = std::move(name);
        }
        friend component::SerialisableComponent<FixedModel>;
    };

    struct AbsolutePosition : component::SerialisableComponent<AbsolutePosition> {
    private:
        static constexpr std::string_view _name = "absolute_position";
        static std::string_view const& getName () {
            return _name;
        }
        util::DataValue serialise ();
        void deserialise (util::DataValue const& val);
    public:
        AbsolutePosition() = default;
        AbsolutePosition (int _vertices, glm::vec2 _pos, glm::mat2 _transform) : component::SerialisableComponent<AbsolutePosition>(),
                                                                                 vertices{_vertices}, pos{_pos}, transform{_transform} {}

        int vertices;
        glm::vec2 pos;
        glm::mat2 transform;
        friend component::SerialisableComponent<AbsolutePosition>;
    };


}
#endif //GRAPHICS_NEW_INCLUDE_H
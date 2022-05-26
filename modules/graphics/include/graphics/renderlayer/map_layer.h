#pragma once

#include "engine/map/map.h"
#include "render_layer.h"
#include "common/events/map_load.h"
#include "event/event_scope.h"
#include "graphics/pipeline/pipeline.h"

namespace graphics {

    class MapPipeline : public Pipeline<game::Map::SharedPtr> {
    public:
        void onMapLoad (game::Map::SharedPtr newMap) {
            bufferData(newMap);
        }

        virtual void applyCamera (const Camera& camera) = 0;
    };

    class MapRenderLayer : public RenderLayer {
    private:
        bool active = false;
        game::Map::SharedPtr map;
        ShaderProgramNew program;
        TextureAtlas& atlas;
        event::EventScope eventScope;

        std::unique_ptr<MapPipeline> mapPipeline;
    public:
        explicit MapRenderLayer(Renderer* renderer, TextureAtlas& atlas);
        std::string getName () override;

        int getPriority () override;

        bool isActive () override;

        void gatherData () override;

        void preRender (Renderer* renderer) override;

        int getUniformId (std::string uniformName) override;

        void applyUniform (int uniformId, void* data) override;

        void applyCamera (Camera camera) override;

        void render (Renderer* renderer, FrameBuffer* frameBuf) override;

        void attachMap (game::Map::SharedPtr map);

        void onMapLoad (event::MapLoadEvent& event);

        event::EventScope& getEventScope ();
    };

};

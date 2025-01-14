#pragma once

#include <fstream>
#include <vector>

#include "core/serialization/component_serializer.h"

namespace phenyl::game {
    namespace detail {
        class LevelEntity;
    }

    class LevelManager;

    class Level : public core::IAssetType<Level> {
    private:
        //std::vector<detail::LevelEntity> entities;
        std::ifstream file;
        std::ifstream::pos_type startPos;
        LevelManager& manager;

        Level (std::ifstream file, LevelManager& manager);

        void loadImmediate (core::World& world, core::EntityComponentSerializer& serializer);
        friend LevelManager;
    public:
        void load (bool additive=false);
    };
}
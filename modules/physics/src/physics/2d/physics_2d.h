#pragma once

#include "physics/physics.h"
#include "core/world.h"


namespace phenyl::physics {
    class Physics2D {
    private:
    public:
        void addComponents(core::PhenylRuntime& runtime);

        void debugRender (core::World& world);
    };
}
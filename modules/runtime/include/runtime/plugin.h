#pragma once

#include "runtime.h"

namespace phenyl::runtime {
    class IPlugin {
    public:
        virtual ~IPlugin() = default;

        [[nodiscard]] virtual std::string_view getName () const noexcept = 0;

        virtual void init (PhenylRuntime& runtime) = 0;
        // virtual void postInit (PhenylRuntime& runtime) {};
        //
        // virtual void frameBegin (PhenylRuntime& runtime) {}
        // virtual void update (PhenylRuntime& runtime, double deltaTime) {}
        // virtual void render (PhenylRuntime& runtime) {}
        //
        // virtual void fixedUpdate (PhenylRuntime& runtime, double deltaTime) {}
        // virtual void physicsUpdate (PhenylRuntime& runtime, double deltaTime) {}

        virtual void shutdown (PhenylRuntime& runtime) {}
    };
}
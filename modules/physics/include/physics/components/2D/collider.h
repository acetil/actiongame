#pragma once

#include "graphics/maths_headers.h"
#include "util/data.h"

namespace physics {
    class RigidBody2D;

    class ColliderComp2D {
    private:
        glm::vec2 currentPos = {0, 0};

        float invMass{1.0f};
        float invInertiaMoment{1.0f};

        glm::vec2 momentum{0.0f};
        float angularMomentum{0.0f};

        glm::vec2 appliedImpulse{0.0f, 0.0f};
        float appliedAngularImpulse{0.0f};

        float outerRadius{0.0f};

        [[nodiscard]] glm::vec2 getCurrVelocity () const {
            return (momentum + appliedImpulse) * invMass;
        }

        [[nodiscard]] float getCurrAngularVelocity () const {
            return (angularMomentum + appliedAngularImpulse) * invInertiaMoment;
        }

        void applyImpulse (glm::vec2 impulse) {
            appliedImpulse += impulse;
        }

        void applyAngularImpulse (float angularImpulse) {
            appliedAngularImpulse += angularImpulse;
        }

        void syncUpdates (const RigidBody2D& body, glm::vec2 pos);
        void updateBody (RigidBody2D& body) const;

        friend class PhysicsObject2D;
        friend class Constraint2D;
        friend class Manifold2D;
    protected:
        void setOuterRadius (float newOuterRadius) {
            outerRadius = newOuterRadius;
        };
        [[nodiscard]] glm::vec2 getPosition () const {
            return currentPos;
        }

        [[nodiscard]] glm::vec2 getDisplacement (const ColliderComp2D& other) const {
            return other.currentPos - currentPos;
        }

        [[nodiscard]] bool shouldCollide (const ColliderComp2D& other) const;
    public:
        std::uint64_t layers = 0;
        std::uint64_t mask = 0;
        float elasticity{0.0f};

        ColliderComp2D () = default;

        [[nodiscard]] virtual util::DataValue serialise () const;
        virtual bool deserialise (const util::DataValue& val);
    };
}
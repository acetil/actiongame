#pragma once

#include <vector>

#include "ui_component.h"
#include "graphics/ui/ui_defs.h"

namespace graphics::ui {

    namespace detail {
        struct FlexBoxItem;
    }

    enum class FlexJustify {
        NONE,
        CENTRE,
        SPACE_BETWEEN,
        SPACE_AROUND,
        SPACE_EVENLY
    };

    enum class FlexAlign {
        START,
        END,
        CENTRE
    };

    class UIFlexBoxNode : public UIComponentNode {
    private:
        glm::vec2 primaryAxisVec = getAxisVec(Axis::DOWN);
        glm::vec2 secondaryAxisVec = getAxisVec(Axis::RIGHT);

        std::vector<detail::FlexBoxItem> items;
        std::vector<std::shared_ptr<UIComponentNode>> children;

        FlexJustify justification = FlexJustify::NONE;
        FlexAlign alignment = FlexAlign::START;

        glm::vec2 minSize = {0, 0};
        glm::vec2 maxSize = {-1, -1};

        void placeChildren (const std::vector<UIAnchor>& anchors);
        float expandChildren (const std::vector<UIAnchor>& anchors, float remainingSize);
        void justifyChildren (float remainingSpace);
        void alignChildren (const std::vector<UIAnchor>& anchors);

    protected:
        void updateLayout ();

    public:
        explicit UIFlexBoxNode (std::weak_ptr<UIComponentNode> parent);
        ~UIFlexBoxNode() override;
        void setAxes (Axis primaryAxis, Axis secondaryAxis);
        void addComponent (const std::shared_ptr<UIComponentNode>& uiNode);
        void addComponent (const std::shared_ptr<UIComponentNode>& uiNode, int index);
        void setMaxSize (glm::vec2 maxSize);
        void setSize(glm::vec2 _size) override;

        void setJustify (FlexJustify justify);
        void setAlign (FlexAlign align);

        void render(UIManager &uiManager) override;
        UIAnchor getAnchor() override;

        void onMousePosChange(glm::vec2 oldMousePos) override;
        bool onMousePress() override;
        void onMouseRelease() override;
    };
}
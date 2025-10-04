#pragma once
#include "IPanel.hpp"
#include <string>

class SceneHierarchyPanel : public IPanel {
public:
    const char* Name() const override { return "SceneHierarchy"; }
    void Draw(PanelContext& ctx) override;
};


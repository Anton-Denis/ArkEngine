#pragma once
#include "IPanel.hpp"

class InspectorPanel : public IPanel {
public:
    const char* Name() const override { return "Inspector"; }
    void Draw(PanelContext& ctx) override;
};


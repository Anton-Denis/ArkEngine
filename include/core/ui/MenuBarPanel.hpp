#pragma once
#include "IPanel.hpp"

class MenuBarPanel : public IPanel {
public:
    const char* Name() const override { return "MenuBar"; }
    void Draw(PanelContext& ctx) override;
};


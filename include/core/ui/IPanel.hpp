#pragma once
#include <string>
struct PanelContext; // forward
struct IPanel {
    virtual ~IPanel() = default;
    virtual const char* Name() const = 0;
    virtual void Draw(PanelContext& ctx) = 0;
};

